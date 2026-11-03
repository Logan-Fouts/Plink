#include "input/button.h"

#include "config.h"

void button_init(button_t *b, uint16_t rest_raw) {
  b->state       = BUTTON_STATE_IDLE;
  b->rest_raw    = rest_raw;
  b->pressed_raw = (int32_t)rest_raw + HALL_DEFAULT_SPAN;
  b->start_us    = 0;
  b->position    = 0;
}

// Map the travel time between the two thresholds to a MIDI velocity.
// Linear for now; a curve (e.g. square-root shaped) often feels better.
static uint8_t travel_to_velocity(uint32_t dt_us) {
  if (dt_us <= VELOCITY_FAST_US) return 127;
  if (dt_us >= VELOCITY_SLOW_US) return 1;
  return (uint8_t)(1 + (126u * (VELOCITY_SLOW_US - dt_us)) / (VELOCITY_SLOW_US - VELOCITY_FAST_US));
}

// Convert a raw reading to a position 0..1000 using the calibration.
// Works whichever direction the sensor reading moves.
static int32_t compute_position(button_t *b, int32_t raw) {
  int32_t span = b->pressed_raw - b->rest_raw;

  // If the key went further than we expected, learn the new full-press value.
  if ((span > 0 && raw > b->pressed_raw) || (span < 0 && raw < b->pressed_raw)) {
    b->pressed_raw = raw;
    span           = b->pressed_raw - b->rest_raw;
  }

  int32_t pos;
  if (span == 0) {
    pos = ((raw - b->rest_raw) * 1000) / DEFAULT_SPAN;  // Avoid division by zero
  } else {
    pos = ((raw - b->rest_raw) * 1000) / span;
  }

  if (pos < 0) pos = 0;
  if (pos > 1000) pos = 1000;
  return pos;
}

button_event_t button_update(button_t *b, uint16_t raw, uint32_t now_us) {
  button_event_t ev = {BUTTON_EVENT_NONE, 0, 0};

  int32_t pos = compute_position(b, raw);
  b->position = pos;

  // IDLE -> TRAVELLING: the key has started moving, start the clock.
  if (b->state == BUTTON_STATE_IDLE && pos >= HALL_THRESHOLD_LOW) {
    b->state    = BUTTON_STATE_TRAVELLING;
    b->start_us = now_us;
  }

  if (b->state == BUTTON_STATE_TRAVELLING) {
    if (pos >= HALL_THRESHOLD_HIGH) {
      // TRAVELLING -> DOWN: second threshold reached, work out velocity.
      uint32_t dt  = now_us - b->start_us;  // safe across timer wrap-around
      ev.type      = BUTTON_EVENT_PRESSED;
      ev.velocity  = travel_to_velocity(dt);
      ev.travel_us = dt;
      b->state     = BUTTON_STATE_DOWN;
    } else if (pos < HALL_THRESHOLD_RELEASE) {
      // Key went back up before reaching threshold: cancel, no note.
      b->state = BUTTON_STATE_IDLE;
    }
  } else if (b->state == BUTTON_STATE_DOWN) {
    if (pos < HALL_THRESHOLD_RELEASE) {
      // DOWN -> IDLE: key released.
      ev.type  = BUTTON_EVENT_RELEASED;
      b->state = BUTTON_STATE_IDLE;
    }
  }

  return ev;
}
