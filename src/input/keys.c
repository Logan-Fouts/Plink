#include "input/keys.h"
#include "input/button.h"
#include "input/mux.h"
#include "midi/midi_out.h"
#include "debug/debug.h"
#include "leds/leds.h"
#include "config.h"

// A key = which mux channel it is on + which note it plays + its Hall state.
typedef struct {
    uint8_t  mux_channel;
    uint8_t  note;
    button_t button;
} keyswitch_t;

static keyswitch_t keys[KEY_COUNT] = KEY_MAP;

void keys_init(void) {
    mux_init();
    leds_init();

    // Measure each key's resting value (average of several readings).
    for (int i = 0; i < KEY_COUNT; i++) {
        uint32_t sum = 0;
        for (int n = 0; n < HALL_CALIBRATION_READS; n++) {
            sum += mux_read(keys[i].mux_channel);
        }
        button_init(&keys[i].button, (uint16_t)(sum / HALL_CALIBRATION_READS));
    }
}

void keys_update(uint32_t now_us) {
#if DEBUG_STREAM_KEY >= 0
    static uint32_t last_stream_us = 0;
#endif

    for (int i = 0; i < KEY_COUNT; i++) {
        keyswitch_t *k = &keys[i];

        uint16_t raw = mux_read(k->mux_channel);
        button_event_t e = button_update(&k->button, raw, now_us);

        if (e.type == BUTTON_EVENT_PRESSED) {
            midi_send_note_on(k->note, e.velocity);
            leds_set_color(i, LED_PRESSED_R, LED_PRESSED_G, LED_PRESSED_B);
            debug_printf("key %d note %d pressed, velocity %d (travel %u us)\r\n",
                         i, k->note, e.velocity, (unsigned)e.travel_us);
        } else if (e.type == BUTTON_EVENT_RELEASED) {
            midi_send_note_off(k->note);
            leds_off(i);
            debug_printf("key %d note %d released\r\n", i, k->note);
        }

#if DEBUG_STREAM_KEY >= 0
        if (i == DEBUG_STREAM_KEY && (now_us - last_stream_us) >= 20000) {
            last_stream_us = now_us;
            debug_printf("raw %u pos %d\r\n", raw, (int)button_position(&k->button));
        }
#endif
    }
}
