#include "input/keys.h"

#include "input/button.h"
#include "midi/midi_out.h"
#include "debug/debug.h"
#include "config.h"

static button_t buttons[KEY_COUNT];
static const uint8_t key_pins[KEY_COUNT]  = KEY_PINS;
static const uint8_t key_notes[KEY_COUNT] = KEY_NOTES;

void keys_init(void) {
    for (int i = 0; i < KEY_COUNT; i++) {
        button_init(&buttons[i], key_pins[i]);
    }
}

void keys_update(uint32_t now_ms) {
    for (int i = 0; i < KEY_COUNT; i++) {
        button_event_t e = button_update(&buttons[i], now_ms);

        if (e == BUTTON_EVENT_PRESSED) {
            midi_send_note_on(key_notes[i], DEFAULT_VELOCITY);
            debug_printf("key %d (GPIO %d, note %d) pressed\r\n", i, key_pins[i], key_notes[i]);
        } else if (e == BUTTON_EVENT_RELEASED) {
            midi_send_note_off(key_notes[i]);
            debug_printf("key %d (GPIO %d, note %d) released\r\n", i, key_pins[i], key_notes[i]);
        }
    }
}