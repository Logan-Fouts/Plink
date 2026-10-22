#include "hardware/mux_hw.h"
#include "fakes/mux_hw_fake.h"
#include "config.h"

static int      pin_levels[MUX_SELECT_PIN_COUNT];
static uint16_t adc_value;

void mux_fake_reset(void) {
  for (int i = 0; i < MUX_SELECT_PIN_COUNT; i++) pin_levels[i] = -1;
  adc_value = 0;
}

int  mux_fake_pin_level(int pin) {
  return pin_levels[pin];
}

void mux_fake_set_adc_value(uint16_t v) {
  adc_value = v;
}

// Function Squatting
void mux_hw_init(void) {}

void mux_hw_set_select_pin(int i, int level) { 
  pin_levels[i] = level;
}

uint16_t mux_hw_read_adc(void) {
  return adc_value;
}
