#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PUSHBUTTON_EVENT_NONE,
    PUSHBUTTON_EVENT_PRESSED,
    PUSHBUTTON_EVENT_RELEASED
} pushbutton_event_t;

typedef struct {
    bool     pressed;          // current debounced state
    uint32_t last_change_us;   // time of the last accepted change
} pushbutton_t;

void pushbutton_init(pushbutton_t *b);

// raw_pressed: true while the button is physically held down.
// Accepts the first edge immediately, then ignores changes for the debounce time.
pushbutton_event_t pushbutton_update(pushbutton_t *b, bool raw_pressed, uint32_t now_us);

static inline bool pushbutton_is_pressed(const pushbutton_t *b) {
    return b->pressed;
}
