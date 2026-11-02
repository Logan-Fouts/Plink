#include "input/mux.h"
#include "hardware/mux_hw.h"
#include "hardware/utils_hw.h"
#include "config.h"

/**
 * @brief Initialize the MUX hardware (ADC input and select pins).
 *
 * Must be called once before mux_read(). Leaves the select pins at
 * channel 0.
 */
void mux_init(void) {
  mux_hw_init();
}

/**
 * @brief Read one MUX channel, averaged over MUX_SAMPLES ADC readings.
 *
 * Selects the channel, waits MUX_SETTLE_US for the signal to settle,
 * discards one conversion, then averages the samples. Blocks for the duration.
 *
 * @param channel Channel index, associated with a key.
 * @return Averaged raw ADC value (12-bit, 0-4095).
 */
uint16_t mux_read(int channel) {
  
  if (channel < 0) return 0;

  // Switch to correct ADC channel with 4 select pins
  for (int i = 0; i < MUX_SELECT_PIN_COUNT; i++) {
    mux_hw_set_select_pin(i, (channel >> i) & 1);
  }

  // Let readings settle on new channel
  utils_hw_sleep(MUX_SETTLE_US);

  // Throw away the first conversion: the ADC's sample-and-hold capacitor
  // still carries charge from the previous channel, which makes a pressed
  // key bleed into the next channel read (neighbouring keys "press" too).
  (void)mux_hw_read_adc();

  // Take readings and average them
  uint32_t sum = 0;
  for (int n = 0; n < MUX_SAMPLES; n++) {
    sum += mux_hw_read_adc();
  }

  return (uint16_t)(sum / MUX_SAMPLES);
}
