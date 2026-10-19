#pragma once

#include <stdbool.h>
#include <stdint.h>

// Where a key is in its press.
typedef enum {
    BUTTON_STATE_IDLE,
    BUTTON_STATE_TRAVELLING,
    BUTTON_STATE_DOWN
} button_state_t;

typedef enum {
    BUTTON_EVENT_NONE,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED
} button_event_type_t;

typedef struct {
    button_event_type_t type;
    uint8_t  velocity;     // 1-127, only valid for PRESSED
    uint32_t travel_us;    // time between the two thresholds (for tuning)
} button_event_t;

// One Hall-effect key. It owns its calibration and state, so one update
// function works for every key. It knows nothing about pins or the ADC:
// you hand it a raw reading.
typedef struct {
    button_state_t state;
    int32_t  rest_raw;      // reading with the key up
    int32_t  pressed_raw;   // reading with the key fully down (auto-extends)
    uint32_t start_us;      // when the LOW threshold was crossed
    int32_t  position;      // last position, 0..1000
} button_t;

// Call once with the resting reading (key not pressed).
void button_init(button_t *b, uint16_t rest_raw);

// Feed in a new raw ADC reading and the current time in microseconds.
// Returns an event if the key was pressed or released.
button_event_t button_update(button_t *b, uint16_t raw, uint32_t now_us);

static inline bool button_is_pressed(const button_t *b) {
    return b->state == BUTTON_STATE_DOWN;
}

static inline int32_t button_position(const button_t *b) {
    return b->position;
}