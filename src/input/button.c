#include "input/button.h"

#include "pico/stdlib.h"
#include "config.h"

void button_init(button_t *b, uint8_t pin) {
    b->pin = pin;
    b->pressed = false;
    b->last_change_ms = 0;

    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);             // idle = high, pressed = low
}

button_event_t button_update(button_t *b, uint32_t now_ms) {
    bool raw_pressed = !gpio_get(b->pin);   // low means pressed

    // Accept the first edge immediately (no added latency), then ignore
    // further changes for BUTTON_DEBOUNCE_MS while the contacts bounce.
    if (raw_pressed != b->pressed &&
        (now_ms - b->last_change_ms) >= BUTTON_DEBOUNCE_MS) {
        b->pressed = raw_pressed;
        b->last_change_ms = now_ms;
        return raw_pressed ? BUTTON_EVENT_PRESSED : BUTTON_EVENT_RELEASED;
    }

    return BUTTON_EVENT_NONE;
}