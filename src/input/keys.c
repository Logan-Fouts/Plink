#include "input/keys.h"

#include "input/button.h"
#include "input/mux.h"
#include "midi/midi_out.h"
#include "debug/debug.h"
#include "leds/leds.h"
#include "config.h"

#include <stdbool.h>

#define NOTE_NONE 0xFF

// A key = which mux channel it is on + which note it plays + its Hall state.
typedef struct {
  uint8_t  mux_channel;
  uint8_t  note;           // base note from KEY_MAP
  uint8_t  sounding_note;  // note actually sent on press (NOTE_NONE if none)
  bool     connected;      // false if no sensor was found on this channel
  button_t button;
} keyswitch_t;

static keyswitch_t keys[KEY_COUNT] = KEY_MAP;
static int8_t      transpose       = 0;

// Measure a key's resting value (average of several readings), work out
// whether a sensor is connected, and reset its learned full-press value.
// The key must be at rest while this runs.
static void calibrate_key(keyswitch_t *k) {
  uint32_t sum = 0;
  for (int n = 0; n < HALL_CALIBRATION_READS; n++) {
    sum += mux_read(k->mux_channel);
  }
  uint16_t rest = (uint16_t)(sum / HALL_CALIBRATION_READS);
  k->connected  = rest >= HALL_DISCONNECTED_RAW;
  button_init(&k->button, rest);
}

void keys_init(void) {
  mux_init();
  leds_init();

  for (int i = 0; i < KEY_COUNT; i++) {
    keys[i].sounding_note = NOTE_NONE;
    calibrate_key(&keys[i]);
  }
}

void keys_set_transpose(int8_t semitones) {
  if (semitones < TRANSPOSE_MIN) semitones = TRANSPOSE_MIN;
  if (semitones > TRANSPOSE_MAX) semitones = TRANSPOSE_MAX;
  if (semitones != transpose) {
    transpose = semitones;
    debug_printf("transpose %+d\r\n", (int)transpose);
  }
}

void keys_shift_transpose(int8_t semitones) {
  keys_set_transpose((int8_t)(transpose + semitones));
}

int8_t keys_get_transpose(void) {
  return transpose;
}

void keys_update(uint32_t now_us) {
#if DEBUG_STREAM_KEY >= 0
  static uint32_t last_stream_us = 0;
#endif

  for (int i = 0; i < KEY_COUNT; i++) {
    keyswitch_t *k = &keys[i];
    if (!k->connected) continue;  // nothing wired to this channel

    uint16_t       raw = mux_read(k->mux_channel);
    button_event_t e   = button_update(&k->button, raw, now_us);

    if (e.type == BUTTON_EVENT_PRESSED) {
      // Remember the note we send, so the release matches even if the
      // transpose changes while the key is held.
      int n = k->note + transpose;
      if (n >= 0 && n <= 127) {
        k->sounding_note = (uint8_t)n;
        midi_send_note_on(k->sounding_note, e.velocity);
      } else {
        k->sounding_note = NOTE_NONE;  // out of MIDI range: stay silent
      }
      leds_set_color(i, LED_PRESSED_R, LED_PRESSED_G, LED_PRESSED_B);
      debug_printf("key %d note %d pressed, velocity %d (travel %u us)\r\n", i,
                   (int)k->sounding_note, e.velocity, (unsigned)e.travel_us);
    } else if (e.type == BUTTON_EVENT_RELEASED) {
      if (k->sounding_note != NOTE_NONE) {
        midi_send_note_off(k->sounding_note);
        k->sounding_note = NOTE_NONE;
      }
      leds_off(i);
      debug_printf("key %d released\r\n", i);
    }

#if DEBUG_STREAM_KEY >= 0
    if (i == DEBUG_STREAM_KEY && (now_us - last_stream_us) >= 20000) {
      last_stream_us = now_us;
      debug_printf("raw %u pos %d\r\n", raw, (int)button_position(&k->button));
    }
#endif
  }
}
