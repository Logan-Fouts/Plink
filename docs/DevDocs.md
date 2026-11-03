# Plink Developer Guide

This guide covers how the Plink firmware is structured, how data moves from a key press to a MIDI message, and how to extend and test the code. For build and flashing instructions, see the [README](../README.md).

## Contents

1. [Architecture](#architecture)
2. [Main loop](#main-loop)
3. [Modules](#modules)
4. [Key sensing in depth](#key-sensing-in-depth)
5. [Hardware abstraction layer](#hardware-abstraction-layer)
6. [Testing](#testing)
7. [Code style and linting](#code-style-and-linting)
8. [Common tasks](#common-tasks)
9. [Known issues and TODOs](#known-issues-and-todos)

---

## Architecture

The firmware is written in C11 on top of the Pico SDK and TinyUSB. It is split into layers so that the logic can be compiled and tested without the RP2040:

```
┌──────────────────────────────────────────────────────────┐
│ main.c         startup, dual-core handshake, main loop   │
├──────────────────────────────────────────────────────────┤
│ input/         keys · controls                           │  application
│ leds/  midi/  debug/                                     │
├──────────────────────────────────────────────────────────┤
│ input/         button · push_button · mux                │  pure logic
├──────────────────────────────────────────────────────────┤
│ hardware/      *_hw.h interfaces                         │  HAL
│ hardware/rp2040/   Pico SDK implementations              │
│ test/fakes/        host-side fakes for unit tests        │
└──────────────────────────────────────────────────────────┘
```

- **Pure logic** modules (`button`, `push_button`) take raw readings and timestamps as arguments and touch no hardware. `mux` reaches hardware only through the HAL.
- **HAL** headers in `src/hardware/` declare the few hardware operations the firmware needs. `src/hardware/rp2040/` implements them with the Pico SDK, and `test/fakes/` implements them for tests.
- **Configuration** is centralised in `src/config.h`. No other file should hard-code pins, thresholds or timings.

## Main loop

Everything currently runs on **core 0**:

```
stdio_init_all → tusb_init → controls_init → keys_init (mux, LEDs, calibration)
        │
        ▼
launch core 1 ── FIFO handshake ──▶ core 1 idles (reserved for future work)
        │
        ▼
┌─ loop ─────────────────────────────────────────────┐
│ tud_task()            USB housekeeping              │
│ drain MIDI/CDC input  keep host→device buffers free │
│ controls_update(now)  transpose buttons             │
│ keys_update(now)      read keys → MIDI + LED buffer │
│ leds_update()         push LED buffer if changed    │
└─────────────────────────────────────────────────────┘
```

The loop never blocks for long. Each `mux_read` waits about 10 µs to settle, and `leds_update` sends a frame only when something has changed and the previous frame has finished.

## Modules

| Module | Files | Responsibility |
| --- | --- | --- |
| **keys** | `input/keys.{c,h}` | Owns the array of `keyswitch_t`. Calibrates keys at boot, scans them every loop, applies transpose and sends MIDI note on/off and LED updates. |
| **button** | `input/button.{c,h}` | Hall-effect key state machine: converts raw ADC readings to travel position and emits press/release events with velocity. |
| **mux** | `input/mux.{c,h}` | Selects a CD74HC4067 channel and returns an averaged 12-bit ADC reading. |
| **controls** | `input/controls.{c,h}` | Front-panel push buttons (transpose). |
| **push_button** | `input/push_button.{c,h}` | Debounce logic for ordinary digital buttons. |
| **leds** | `leds/leds.{c,h}`, `leds/ws2812.pio` | Buffered NeoPixel driver. `leds_set_*` writes only to the buffer, and `leds_update()` sends it. |
| **midi_out** | `midi/midi_out.{c,h}` | Builds Note On/Off and CC messages and writes them to the USB MIDI stream. Messages are dropped while USB is not mounted. |
| **midi_in** | `midi/midi_in.{c,h}` | Placeholder. Incoming MIDI is currently discarded in `main.c`. |
| **debug** | `debug/debug.{c,h}` | `debug_printf()` over USB CDC. Does nothing when `DEBUG` is off or no terminal is open. |
| **usb** | `usb/tusb_config.h`, `usb/usb_descriptors.c` | TinyUSB configuration and descriptors for the composite CDC + MIDI device. |

### Key data types

```c
// input/keys.c (private)
typedef struct {
    uint8_t  mux_channel;    // CD74HC4067 channel 0–15
    uint8_t  note;           // base MIDI note from KEY_MAP
    uint8_t  sounding_note;  // note actually sent (NOTE_NONE if silent)
    bool     connected;      // false if no sensor detected at boot
    button_t button;         // Hall-sensor state
} keyswitch_t;

// input/button.h
typedef struct {
    button_state_t state;    // IDLE / TRAVELLING / DOWN
    int32_t  rest_raw;       // ADC reading with key up
    int32_t  pressed_raw;    // ADC reading fully down (learned)
    uint32_t start_us;       // time LOW threshold was crossed
    int32_t  position;       // 0..1000
} button_t;

typedef struct {
    button_event_type_t type;  // NONE / PRESSED / RELEASED
    uint8_t  velocity;         // 1–127 (PRESSED only)
    uint32_t travel_us;        // LOW→HIGH time, useful for tuning
} button_event_t;
```

`sounding_note` stores the note that was actually sent on press. The matching Note Off then goes out for the same note even if the transpose changes while the key is held.

## Key sensing in depth

### 1. Reading the sensor (`mux_read`)

1. Drive S0–S3 with the channel number.
2. Wait `MUX_SETTLE_US` (10 µs).
3. **Discard one ADC conversion.** The ADC's sample-and-hold capacitor still holds charge from the previous channel. Without this, a pressed key bleeds into the next channel read.
4. Average `MUX_SAMPLES` (4) conversions.

### 2. Calibration (`calibrate_key`)

At boot, each key is read `HALL_CALIBRATION_READS` (32) times and averaged to give `rest_raw`. If the average is below `HALL_DISCONNECTED_RAW`, the channel is treated as having no sensor and is skipped until the next reset. SIG is pulled down, so an empty channel reads near zero.

`pressed_raw` starts at `rest_raw + HALL_DEFAULT_SPAN`. Whenever a reading goes beyond it, `pressed_raw` is extended. The full range is therefore learned while you play, and a conservative default is safe. The sign of `HALL_DEFAULT_SPAN` sets the direction: positive if the reading rises when the key is pressed.

### 3. Position

```
position = clamp( (raw − rest_raw) × 1000 / (pressed_raw − rest_raw), 0, 1000 )
```

Position is in per mille of full travel. The formula works whichever way the sensor output moves.

### 4. State machine (`button_update`)

```
              pos ≥ LOW (start timer)          pos ≥ HIGH
   ┌──────┐ ───────────────────────▶ ┌────────────┐ ──────────▶ ┌──────┐
   │ IDLE │                          │ TRAVELLING │  PRESSED    │ DOWN │
   └──────┘ ◀─────────────────────── └────────────┘  (velocity) └──────┘
      ▲      pos < RELEASE (cancelled, no note)                    │
      └────────────────────────────────────────────────────────────┘
                         pos < RELEASE → RELEASED
```

| Threshold | Default | Meaning |
| --- | --- | --- |
| `HALL_THRESHOLD_LOW` | 200 | Timing starts |
| `HALL_THRESHOLD_HIGH` | 700 | Note fires |
| `HALL_THRESHOLD_RELEASE` | 150 | Key released. It sits below LOW, which gives hysteresis. |

### 5. Velocity

The time between crossing LOW and HIGH maps linearly to velocity:

| Travel time | Velocity |
| --- | --- |
| ≤ `VELOCITY_FAST_US` (2 ms) | 127 |
| ≥ `VELOCITY_SLOW_US` (60 ms) | 1 |
| in between | linear interpolation |

Timestamps are `uint32_t` microseconds, and the subtraction is safe across timer wrap-around. A curved mapping (for example square-root) may feel more natural and is a candidate improvement in `travel_to_velocity()`.

### LEDs

The pixel buffer stores pre-scaled GRB values. `LED_BRIGHTNESS` is applied in `leds_set_color`. `leds_update()` sends a frame only if the buffer is dirty **and** the previous frame plus its latch time (about 30 µs per LED + 300 µs) has passed. It is safe to call every loop.

### USB

The device is a composite **CDC (serial) + MIDI** device. The VID/PID (`0xCAFE` / `0x4011`) are TinyUSB test IDs. They are fine for development but must be replaced before any public release.

## Hardware abstraction layer

| Header | Functions |
| --- | --- |
| `hardware/mux_hw.h` | `mux_hw_init`, `mux_hw_set_select_pin`, `mux_hw_read_adc` |
| `hardware/leds_hw.h` | `leds_hw_init`, `leds_hw_pio_put_blocking` |
| `hardware/utils_hw.h` | `utils_hw_sleep`, `time_us`, `utils_hw_debug_write`, `utils_hw_tud_connected`, `utils_hw_usb_mounted`, `utils_hw_midi_stream_write` |

Rules of thumb:

- Modules under `input/`, `leds/`, `midi/` and `debug/` should include `hardware/*_hw.h`, not Pico SDK headers. (`controls.c` currently calls the GPIO SDK directly. See [Known issues](#known-issues-and-todos).)
- A new hardware dependency gets a function in the right `*_hw.h`, an implementation in `hardware/rp2040/`, and a fake in `test/fakes/` if it is used by tested code.

## Testing

Tests use [Unity](https://github.com/ThrowTheSwitch/Unity), included as a git submodule in `lib/Unity` (run `git submodule update --init` if it's empty). They are built with the host compiler, not the ARM toolchain.

```bash
cmake -S test -B build/test
cmake --build build/test
ctest --test-dir build/test --output-on-failure
```

| Test | Covers | Fakes |
| --- | --- | --- |
| `test_mux` | Channel selection, settling, averaging | `mux_hw_fake`, `utils_hw_fake` |
| `test_button` | Position, state machine, velocity | none (pure logic) |
| `test_pushbutton` | Debouncing | none (pure logic) |

The fakes record what the code under test did (for example `mux_fake_pin_level()` and `utils_fake_last_sleep_us()`) and let a test inject inputs (`mux_fake_set_adc_value()`). Call the `*_fake_reset()` functions from `setUp()`.

### Adding a test

1. Create `test/<area>/test_<module>.c` with `setUp`, `tearDown`, your `test_*` functions and a `main` that calls `UNITY_BEGIN()`, `RUN_TEST(...)` and `UNITY_END()`.
2. Register it in `test/CMakeLists.txt`:

   ```cmake
   add_executable(test_<module>
       <area>/test_<module>.c
       ../src/<area>/<module>.c
       # fakes/... if it touches the HAL
   )
   target_link_libraries(test_<module> PRIVATE unity)
   target_include_directories(test_<module> PRIVATE
       ${CMAKE_CURRENT_SOURCE_DIR}/../src
       ${CMAKE_CURRENT_SOURCE_DIR})
   add_test(NAME test_<module> COMMAND test_<module>)
   ```

## Code style and linting

| Tool | Config | Run | What it does |
| --- | --- | --- | --- |
| clang-format | `.clang-format` | `scripts/format.sh` (`--check` to verify only) | Formats all C in `src/` and `test/` |
| clang-tidy | `.clang-tidy` | `scripts/lint.sh` (`--fix` to auto-apply fixes) | Bug-finding static analysis (bugprone, cert, clang-analyzer, misc, performance, readability) |
| EditorConfig | `.editorconfig` | your editor | Line endings, indentation, trailing whitespace |

**Setup.** Run `python3 -m venv .venv && .venv/bin/pip install -r requirements-dev.txt`. The scripts and the hook use `.venv/bin/` automatically when it exists, and fall back to `clang-format`/`clang-tidy` on your `PATH` otherwise. Different clang-format versions format slightly differently, so use the pinned one.

**Style summary.** 2-space indent, 100-column limit, `type *ptr`, attached braces, and aligned macros, struct fields and trailing comments. Include order is never changed automatically, because the Pico SDK and TinyUSB headers can be order-sensitive. To protect a hand-aligned block (like `KEY_MAP` in `config.h`), wrap it in `// clang-format off` / `// clang-format on`.

**Auto-format on commit.** `scripts/install-hooks.sh` sets `core.hooksPath` to `scripts/hooks`. The pre-commit hook then formats staged C files and re-stages them. If a file is only partially staged (`git add -p`), the hook leaves it alone and blocks the commit until you format it yourself.

**Format on save (VS Code).** Install the C/C++ or clangd extension and set `"editor.formatOnSave": true`. Both read `.clang-format`.

**Linting.** clang-tidy reads `build/compile_commands.json`, which is generated when you run `cmake -S . -B build`. The script points clang-tidy at the ARM target and the GCC toolchain's system headers, and reports only on files in this repo's `src/`, not the Pico SDK. Every finding counts as an error, so the script exits non-zero if anything is reported. To silence a single false positive, put `// NOLINT(check-name)` on the line.

**CI/CD.** `.github/workflows/ci.yml` runs on every push to `master` and on every pull request. It has three jobs: **format check**, **unit tests**, and **firmware build + clang-tidy lint** (against Pico SDK 2.3.1, which is cached between runs). Each run's `plink.uf2`/`plink.elf` is uploaded as a build artifact. Pushing a tag like `v1.0.0` also publishes a GitHub Release with the firmware attached, once all jobs pass.

## Common tasks

**Change which notes the keys play.** Edit `KEY_MAP` in `config.h`. Each entry is `{ mux_channel, midi_note }`.

**Tune key feel.** Set `DEBUG_STREAM_KEY` to the key you're tuning and watch `raw` and `pos` on the serial port. Press events also print `travel` in µs. Use those numbers to adjust the `HALL_THRESHOLD_*` and `VELOCITY_*` values.

**Add a source file.** Add it to `add_executable(plink ...)` in the root `CMakeLists.txt`. If it is tested, also add it to the matching target in `test/CMakeLists.txt`.

**Add a push button.** Pick a free GPIO in `config.h`, then add a `pushbutton_t` to `controls.c`, initialise the pin with a pull-up in `controls_init`, and handle its events in `controls_update`.

**Send a CC from a knob.** `midi_send_cc(controller, value)` already exists. Read the pot through a spare mux channel or ADC pin, scale it to 0–127, and send only when the value changes.

## Known issues and TODOs

- **`debug_printf` is not thread-safe.** Core 1 calls it too. Route core 1 output through the multicore FIFO or a locked queue.
- **`apply_avg_reading()`** is declared in `keys.h` but never defined. Calibration happens inside `keys_init()`. Remove the declaration or implement it.
- **`controls.c` bypasses the HAL** (`gpio_*` calls), so it can't be unit tested yet.
- **`midi_in` is empty.** Incoming MIDI is drained and discarded.
- **Core 1 is idle.** It is reserved for future work such as generative patterns.
- **USB VID/PID** are TinyUSB test values.
- **Open clang-tidy findings** (`scripts/lint.sh`): a possible divide-by-zero in `compute_position()` when `pressed_raw == rest_raw`, non-`static` globals (`core1_entry`, `desc_device`, `desc_configuration`), and redundant parentheses in `utils_hw_rp2040.c`.
