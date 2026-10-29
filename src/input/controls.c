#include "input/controls.h"

#include "pico/stdlib.h"
#include "input/push_button.h"
#include "input/keys.h"
#include "config.h"

static pushbutton_t down_button;
static pushbutton_t up_button;

static void pin_setup(uint8_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);               // idle = high, pressed = low
}

void controls_init(void) {
    pin_setup(TRANSPOSE_DOWN_PIN);
    pin_setup(TRANSPOSE_UP_PIN);
    pushbutton_init(&down_button);
    pushbutton_init(&up_button);
}

// One button was pressed. If the other is already held, reset to 0;
// otherwise shift by one semitone.
static void on_press(int8_t delta, const pushbutton_t *other) {
    if (pushbutton_is_pressed(other)) {
        keys_set_transpose(0);
    } else {
        keys_shift_transpose(delta);
    }
}

void controls_update(uint32_t now_us) {
    pushbutton_event_t down = pushbutton_update(&down_button, !gpio_get(TRANSPOSE_DOWN_PIN), now_us);
    pushbutton_event_t up   = pushbutton_update(&up_button,   !gpio_get(TRANSPOSE_UP_PIN),   now_us);

    if (down == PUSHBUTTON_EVENT_PRESSED) on_press(-1, &up_button);
    if (up   == PUSHBUTTON_EVENT_PRESSED) on_press(+1, &down_button);
}
