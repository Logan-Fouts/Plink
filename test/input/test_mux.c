#include <stdlib.h>
#include <time.h>
#include "unity.h"
#include "input/mux.h"
#include "config.h"
#include "fakes/mux_hw_fake.h"
#include "fakes/utils_hw_fake.h"


void setUp(void) {
  mux_fake_reset();
  utils_fake_reset();
  srand(time(NULL));              // seed rand
}

void tearDown(void) {}

void test_read_returns_adc_value_when_all_samples_equal(void) {
  mux_fake_set_adc_value(1234);
  TEST_ASSERT_EQUAL_UINT16(1234, mux_read(0));
}

void test_random_adc_readings(void) {
  int expected_total = 0;
  int result = 0;
  for (int i = 0; i < 4; i++) {
    int reading = rand() % 380 + 20;
    mux_fake_set_adc_value(reading); // 20 -> 400 for semi relistic readings
    result += mux_read(0);
    expected_total += reading;
  }

  TEST_ASSERT_EQUAL_UINT32(expected_total, result);
}

void test_read_channel_5_sets_pins_1_0_1_0(void) {
  mux_read(5);                              // 0b0101, pin 0 is the LSB
  TEST_ASSERT_EQUAL(1, mux_fake_pin_level(0));
  TEST_ASSERT_EQUAL(0, mux_fake_pin_level(1));
  TEST_ASSERT_EQUAL(1, mux_fake_pin_level(2));
  TEST_ASSERT_EQUAL(0, mux_fake_pin_level(3));
}

void test_read_channel_15_sets_pins(void) {
  mux_read(15);                              // 0b0101, pin 0 is the LSB
  TEST_ASSERT_EQUAL(1, mux_fake_pin_level(0));
  TEST_ASSERT_EQUAL(1, mux_fake_pin_level(1));
  TEST_ASSERT_EQUAL(1, mux_fake_pin_level(2));
  TEST_ASSERT_EQUAL(1, mux_fake_pin_level(3));
}

void test_read_negative_channel(void) {
  mux_read(-2);
  TEST_ASSERT_EQUAL(-1, mux_fake_pin_level(0));
  TEST_ASSERT_EQUAL(-1, mux_fake_pin_level(1));
  TEST_ASSERT_EQUAL(-1, mux_fake_pin_level(2));
  TEST_ASSERT_EQUAL(-1, mux_fake_pin_level(3));
}

void test_read_waits_settle_time(void) {
  mux_read(0);
  TEST_ASSERT_EQUAL_UINT32(MUX_SETTLE_US, utils_fake_last_sleep_us());
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_read_returns_adc_value_when_all_samples_equal);
  RUN_TEST(test_read_channel_5_sets_pins_1_0_1_0);
  RUN_TEST(test_read_waits_settle_time);
  RUN_TEST(test_read_negative_channel);
  RUN_TEST(test_random_adc_readings);
  RUN_TEST(test_read_channel_15_sets_pins);
  return UNITY_END();
}
