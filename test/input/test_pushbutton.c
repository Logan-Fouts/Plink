#include "unity.h"
#include "input/push_button.h"
#include "config.h"

static pushbutton_t b;

void setUp(void)    { pushbutton_init(&b); }
void tearDown(void) {}

#define T0 1000000u   // arbitrary start time, well past boot

void test_idle_gives_no_event(void) {
    TEST_ASSERT_EQUAL(PUSHBUTTON_EVENT_NONE, pushbutton_update(&b, false, T0));
    TEST_ASSERT_FALSE(pushbutton_is_pressed(&b));
}

void test_press_is_reported_immediately(void) {
    TEST_ASSERT_EQUAL(PUSHBUTTON_EVENT_PRESSED, pushbutton_update(&b, true, T0));
    TEST_ASSERT_TRUE(pushbutton_is_pressed(&b));
}

void test_held_button_reports_only_once(void) {
    pushbutton_update(&b, true, T0);
    TEST_ASSERT_EQUAL(PUSHBUTTON_EVENT_NONE, pushbutton_update(&b, true, T0 + 50000));
}

void test_bounce_inside_debounce_window_is_ignored(void) {
    pushbutton_update(&b, true, T0);
    // contact bounces open 1 ms later: ignored, still counts as pressed
    TEST_ASSERT_EQUAL(PUSHBUTTON_EVENT_NONE, pushbutton_update(&b, false, T0 + 1000));
    TEST_ASSERT_TRUE(pushbutton_is_pressed(&b));
}

void test_release_after_debounce_window(void) {
    pushbutton_update(&b, true, T0);
    TEST_ASSERT_EQUAL(PUSHBUTTON_EVENT_RELEASED,
                      pushbutton_update(&b, false, T0 + PUSHBUTTON_DEBOUNCE_US));
    TEST_ASSERT_FALSE(pushbutton_is_pressed(&b));
}

void test_timer_wraparound(void) {
    pushbutton_update(&b, true, 0xFFFFF000u);
    // 0xFFFFF000 -> 0x00003000 is 0x4000 = 16384 us, longer than the 10 ms debounce
    TEST_ASSERT_EQUAL(PUSHBUTTON_EVENT_RELEASED, pushbutton_update(&b, false, 0x00003000u));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_idle_gives_no_event);
    RUN_TEST(test_press_is_reported_immediately);
    RUN_TEST(test_held_button_reports_only_once);
    RUN_TEST(test_bounce_inside_debounce_window_is_ignored);
    RUN_TEST(test_release_after_debounce_window);
    RUN_TEST(test_timer_wraparound);
    return UNITY_END();
}
