#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BUTTON_EVENT_NONE,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED
} button_event_t;

// Each button owns its own state, so one update function works for all of them.
typedef struct {
    uint8_t  pin;
    bool     pressed;          // current debounced state
    uint32_t last_change_ms;   // time of the last accepted change
} button_t;

// Set up the GPIO (input, pull-up) and clear the state.
void button_init(button_t *b, uint8_t pin);

// Read the pin and return an event if the state changed.
// Call this often, passing the same timestamp for a whole scan.
button_event_t button_update(button_t *b, uint32_t now_ms);

static inline bool button_is_pressed(const button_t *b) {
    return b->pressed;
}