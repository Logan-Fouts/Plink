#include "input/push_button.h"

#include "config.h"

void pushbutton_init(pushbutton_t *b) {
  b->pressed        = false;
  b->last_change_us = 0;
}

pushbutton_event_t pushbutton_update(pushbutton_t *b, bool raw_pressed, uint32_t now_us) {
  // Unsigned subtraction stays correct when the 32-bit timer wraps around.
  if (raw_pressed != b->pressed && (now_us - b->last_change_us) >= PUSHBUTTON_DEBOUNCE_US) {
    b->pressed        = raw_pressed;
    b->last_change_us = now_us;
    return raw_pressed ? PUSHBUTTON_EVENT_PRESSED : PUSHBUTTON_EVENT_RELEASED;
  }
  return PUSHBUTTON_EVENT_NONE;
}
