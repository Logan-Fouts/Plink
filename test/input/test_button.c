#include "unity.h"
#include "input/button.h"
#include "config.h"

#define REST 1000   // pretend resting ADC reading

static button_t b;

void setUp(void)    { button_init(&b, REST); }
void tearDown(void) {}

// Raw ADC value for a position (0-1000 per mille of travel), using the
// default span from config.h. Works whichever direction the span points.
static uint16_t raw_at(int position) {
  return (uint16_t)(REST + (HALL_DEFAULT_SPAN * position) / 1000);
}

// Move the key to `start_pos` then `end_pos`, `dt_us` apart. Returns the second event.
static button_event_t press(int start_pos, int end_pos, uint32_t t0, uint32_t dt_us) {
  button_update(&b, raw_at(start_pos), t0);
  return button_update(&b, raw_at(end_pos), t0 + dt_us);
}

void test_rest_gives_no_event(void) {
  button_event_t e = button_update(&b, raw_at(0), 0);
  TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, e.type);
  TEST_ASSERT_EQUAL(BUTTON_STATE_IDLE, b.state);
}

void test_fast_press_gives_max_velocity(void) {
  button_event_t e = press(250, 750, 1000, VELOCITY_FAST_US);
  TEST_ASSERT_EQUAL(BUTTON_EVENT_PRESSED, e.type);
  TEST_ASSERT_EQUAL_UINT8(127, e.velocity);
  TEST_ASSERT_EQUAL_UINT32(VELOCITY_FAST_US, e.travel_us);
  TEST_ASSERT_TRUE(button_is_pressed(&b));
}

void test_slow_press_gives_min_velocity(void) {
  button_event_t e = press(250, 750, 1000, VELOCITY_SLOW_US);
  TEST_ASSERT_EQUAL(BUTTON_EVENT_PRESSED, e.type);
  TEST_ASSERT_EQUAL_UINT8(1, e.velocity);
}

void test_medium_press_is_between(void) {
  uint32_t dt = (VELOCITY_FAST_US + VELOCITY_SLOW_US) / 2;
  button_event_t e = press(250, 750, 1000, dt);
  TEST_ASSERT_EQUAL(BUTTON_EVENT_PRESSED, e.type);
  TEST_ASSERT_GREATER_THAN_UINT8(1, e.velocity);
  TEST_ASSERT_LESS_THAN_UINT8(127, e.velocity);
}

void test_press_that_never_reaches_top_sends_nothing(void) {
  TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, button_update(&b, raw_at(250), 0).type);
  TEST_ASSERT_EQUAL(BUTTON_STATE_TRAVELLING, b.state);
  TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, button_update(&b, raw_at(100), 5000).type);
  TEST_ASSERT_EQUAL(BUTTON_STATE_IDLE, b.state);
}

void test_release_after_press(void) {
  press(250, 750, 0, VELOCITY_FAST_US);
  button_event_t e = button_update(&b, raw_at(100), 100000);
  TEST_ASSERT_EQUAL(BUTTON_EVENT_RELEASED, e.type);
  TEST_ASSERT_FALSE(button_is_pressed(&b));
  TEST_ASSERT_EQUAL(BUTTON_STATE_IDLE, b.state);
}

void test_hysteresis_holds_key_down_between_thresholds(void) {
  press(250, 750, 0, VELOCITY_FAST_US);
  // 180 is below LOW (200) but above RELEASE (150): still counts as down
  TEST_ASSERT_EQUAL(BUTTON_EVENT_NONE, button_update(&b, raw_at(180), 100000).type);
  TEST_ASSERT_TRUE(button_is_pressed(&b));
}

void test_single_scan_jump_still_sends_note(void) {
  button_event_t e = button_update(&b, raw_at(900), 5000);
  TEST_ASSERT_EQUAL(BUTTON_EVENT_PRESSED, e.type);
  TEST_ASSERT_EQUAL_UINT8(127, e.velocity);
  TEST_ASSERT_EQUAL_UINT32(0, e.travel_us);
}

void test_learns_longer_travel(void) {
  button_update(&b, raw_at(1500), 0);
  TEST_ASSERT_EQUAL_INT32(raw_at(1500), b.pressed_raw);
  TEST_ASSERT_EQUAL_INT32(1000, button_position(&b));
}

void test_timer_wraparound(void) {
  // Timestamps straddle the 32-bit wrap: 0xFFFFFF00 -> 0x00000100 is 512 us
  button_update(&b, raw_at(250), 0xFFFFFF00u);
  button_event_t e = button_update(&b, raw_at(750), 0x00000100u);
  TEST_ASSERT_EQUAL(BUTTON_EVENT_PRESSED, e.type);
  TEST_ASSERT_EQUAL_UINT32(512, e.travel_us);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_rest_gives_no_event);
  RUN_TEST(test_fast_press_gives_max_velocity);
  RUN_TEST(test_slow_press_gives_min_velocity);
  RUN_TEST(test_medium_press_is_between);
  RUN_TEST(test_press_that_never_reaches_top_sends_nothing);
  RUN_TEST(test_release_after_press);
  RUN_TEST(test_hysteresis_holds_key_down_between_thresholds);
  RUN_TEST(test_single_scan_jump_still_sends_note);
  RUN_TEST(test_learns_longer_travel);
  RUN_TEST(test_timer_wraparound);
  return UNITY_END();
}
