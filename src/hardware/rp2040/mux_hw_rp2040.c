#include "hardware/mux_hw.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "config.h"

#define ADC_FIRST_GPIO 26

static const uint8_t select_pins[4] = MUX_SELECT_PINS;

void mux_hw_init(void) {
  adc_init();
  adc_gpio_init(MUX_ADC_GPIO);
  adc_select_input(MUX_ADC_GPIO - ADC_FIRST_GPIO);  // Sets the ADC input source pin.
  for (int i = 0; i < MUX_SELECT_PIN_COUNT; i++) {
      gpio_init(select_pins[i]);
      gpio_set_dir(select_pins[i], GPIO_OUT);
      gpio_put(select_pins[i], 0);
  }
}

void mux_hw_set_select_pin(int pin_ix, int level) {
    gpio_put(select_pins[pin_ix], level);
}

uint16_t mux_hw_read_adc(void) {
  return adc_read();
}
