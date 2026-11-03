# Plink

**A low-cost, velocity-sensitive USB MIDI keyboard built on the Raspberry Pi Pico, using Hall-effect key switches.**

Plink reads 16 magnetic (Hall-effect) key switches through an analog multiplexer, works out how fast each key was pressed, and sends MIDI notes with velocity over USB. Each key has its own NeoPixel LED, and two front-panel buttons transpose the keyboard up or down.

> **Status:** early prototype (v1.0 hardware in progress). The firmware works on a breadboard build; the PCB and enclosure are still in progress.

---

## Features

- **Velocity-sensitive keys.** Velocity comes from how long a key takes to travel between two points in its stroke, read by an analog Hall sensor.
- **Plug-and-play USB MIDI.** The keyboard shows up as a USB composite device (MIDI + serial). It needs no drivers on Windows, macOS or Linux.
- **Automatic calibration.** Each key's resting value is measured at power-up, and its full-press value is learned as you play.
- **Per-key RGB feedback.** One WS2812 (NeoPixel) LED per key, driven by the RP2040's PIO.
- **Transpose.** ±24 semitones from two push buttons. Press both at once to reset.
- **Unit-tested input logic.** The input logic sits behind a small hardware abstraction layer, so it builds and runs its tests on a desktop machine.

## Hardware

### Bill of materials (v1.0)

| Part | Qty | Notes |
| --- | --- | --- |
| [Raspberry Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/) (RP2040) | 1 | Microcontroller + USB |
| [CD74HC4067](https://www.digikey.com/en/products/detail/texas-instruments/CD74HC4067SM96/390310) 16-channel analog mux | 1 | Feeds all key sensors into a single ADC pin |
| [Hall-effect linear switches](https://www.maxgaming.se/sv/switchar/moyu-studio-yurui-he-linear-switch) | 16 | Magnetic key switches |
| [SS49E linear Hall sensors](https://www.electrokit.com/en/ss49eua-to-92-hall-effektsensor-2mv/g) | 16 | One under each switch |
| [WS2812 NeoPixels](https://www.digikey.com/en/products/detail/adafruit-industries-llc/1655/5154679) | 16 | One per key |
| Momentary push buttons | 2 | Transpose down / up |

### Pinout

All pin assignments live in [`src/config.h`](src/config.h).

### Design files

- [`hardware/plink_kicad/`](hardware/plink_kicad): KiCad schematic, PCB and custom symbol/footprint libraries
- [`hardware/plink_freecad/`](hardware/plink_freecad): FreeCAD models of the case and components (switch, sensor, LEDs)

## Getting started

### Prerequisites

- [Pico SDK](https://github.com/raspberrypi/pico-sdk) (2.x) with the `PICO_SDK_PATH` environment variable set
- `arm-none-eabi-gcc` toolchain
- CMake ≥ 3.13 (≥ 3.20 for tests)
- A host C compiler (gcc/clang) for running the unit tests

### Clone

```bash
git clone --recurse-submodules https://github.com/Logan-Fouts/Plink.git
cd Plink
```

If you already cloned without `--recurse-submodules`, run `git submodule update --init` to fetch the Unity test framework.

### Build the firmware

```bash
cmake -S . -B build
cmake --build build
```

This produces `build/plink.uf2`.

> **`unrecognized command-line option '-mthumb'`?** CMake is using your desktop `gcc` instead of `arm-none-eabi-gcc`, usually because of a stale cache or an editor kit (for example VS Code CMake Tools set to "GCC x86_64"). Delete the cache and configure again: `rm -rf build/CMakeCache.txt build/CMakeFiles && cmake -S . -B build`.

### Flash

1. Hold **BOOTSEL** on the Pico and plug it into USB. It mounts as a drive named `RPI-RP2`.
2. Copy `build/plink.uf2` onto that drive. The Pico reboots into the firmware.

Or, with [picotool](https://github.com/raspberrypi/picotool):

```bash
picotool load -x build/plink.uf2 -f
```

### Play

Keep your hands off the keys while the keyboard powers up, because each key's resting value is calibrated at boot. After that, open any DAW or MIDI monitor and select the Plink MIDI device.

## Configuration

All tunable settings are compile-time constants in [`src/config.h`](src/config.h):

| Setting | Default | What it does |
| --- | --- | --- |
| `KEY_MAP` | ch 0–15 → notes 60–75 | Mux channel and base MIDI note for each key |
| `HALL_THRESHOLD_LOW` / `HIGH` | 200 / 700 | Travel points (‰) where velocity timing starts and the note fires |
| `HALL_THRESHOLD_RELEASE` | 150 | Travel below which a key counts as released |
| `VELOCITY_FAST_US` / `SLOW_US` | 2 ms / 60 ms | Travel times that map to velocity 127 / 1 |
| `MIDI_CHANNEL` | 0 | MIDI channel (0 = channel 1) |
| `LED_BRIGHTNESS` | 64 | Global LED brightness (0–255) |
| `TRANSPOSE_MIN` / `MAX` | −24 / +24 | Transpose range in semitones |
| `DEBUG` / `DEBUG_STREAM_KEY` | on / key 0 | Serial debug output and live sensor stream |

## Debugging

The keyboard also exposes a USB serial port. Open it with any terminal (`screen`, `minicom`, PuTTY, the VS Code Serial Monitor) to see key events, velocities and transpose changes. With `DEBUG_STREAM_KEY` set, one key's raw ADC value and position are printed every 20 ms, which helps when tuning thresholds.

## Running the tests

The input logic (mux reading, key state machine, push-button debouncing) is tested on the host with [Unity](https://github.com/ThrowTheSwitch/Unity):

```bash
cmake -S test -B build/test
cmake --build build/test
ctest --test-dir build/test --output-on-failure
```

## Code style

Code is formatted with **clang-format** and checked with **clang-tidy**. The tool versions are pinned in `requirements-dev.txt`.

```bash
python3 -m venv .venv && .venv/bin/pip install -r requirements-dev.txt   # one-time
scripts/install-hooks.sh     # one-time: auto-format staged files on every commit

scripts/format.sh            # format everything
scripts/lint.sh              # static analysis (needs a configured firmware build in build/)
```

CI checks formatting, runs the unit tests, builds the firmware and runs clang-tidy on every push and pull request. Tagging a version (`git tag v1.0.0 && git push --tags`) publishes a release with the `.uf2` attached.

## Project layout

```
Plink/
├── src/
│   ├── main.c            # Startup and main loop
│   ├── config.h          # Pins, thresholds and other tunables
│   ├── input/            # Keys, Hall-sensor state machine, mux, push buttons
│   ├── leds/             # NeoPixel buffer + WS2812 PIO program
│   ├── midi/             # USB MIDI output
│   ├── usb/              # TinyUSB config and descriptors
│   ├── debug/            # Serial debug printing
│   └── hardware/         # Hardware abstraction layer (RP2040 implementation)
├── test/                 # Host-side unit tests and hardware fakes
├── scripts/              # format.sh, lint.sh, git hooks
├── hardware/             # KiCad PCB + FreeCAD mechanical models
├── docs/                 # Developer documentation
└── lib/Unity/            # Unit-test framework (git submodule)
```

## Roadmap

- [x] Velocity-sensitive Hall-effect keys over USB MIDI
- [x] Per-key NeoPixel feedback
- [x] Transpose buttons
- [ ] Custom PCB (in progress)
- [ ] Enclosure
- [ ] Potentiometers / knobs sending MIDI CC
- [ ] MIDI input handling
- [ ] Generative, parameterised note patterns (second core)

## Documentation

For architecture, module internals and how to extend the firmware, see the **[Developer Guide](docs/DevDocs.md)**.

## License

No license has been chosen yet. Until one is added, all rights are reserved.
