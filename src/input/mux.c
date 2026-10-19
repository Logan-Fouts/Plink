#include "input/mux.h"

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "config.h"

static const uint8_t select_pins[4] = MUX_SELECT_PINS;

/**
 * @brief Initializes the ADC and the Multiplexer control pins.
 *
 * This function sets up the hardware interface: it initializes the ADC,
 * configures the necessary GPIOs for reading, and ensures all MUX select
 * pins are set to a known initial state (usually '0').
 */
void mux_init(void) {
    adc_init();
    adc_gpio_init(MUX_ADC_GPIO);
    // Sets the ADC input source pin. This specific address assumes GP26 corresponds to ADC Channel 0, etc.
    adc_select_input(MUX_ADC_GPIO - 26);
    for (int i = 0; i < 4; i++) {
        gpio_init(select_pins[i]);
        gpio_set_dir(select_pins[i], GPIO_OUT);
        // Set all select pins to LOW (0) as a safe default state.
        gpio_put(select_pins[i], 0);
    }
}

/**
 * @brief Reads an averaged analog voltage from the specified MUX channel.
 *
 * @param channel The desired channel number (0-15, based on MUX mapping).
 * @return uint16_t The averaged ADC reading for that channel.
 */
uint16_t mux_read(uint8_t channel) {
    // --- STAGE 1: CHANNEL SELECTION ---
    // Write a binary pattern to the select pins (select_pins[0] through [3])
    // to physically route the desired 'channel' input to the ADC input pin.
    for (int i = 0; i < 4; i++) {
        gpio_put(select_pins[i], (channel >> i) & 1);
    }

    // Wait for the electrical signal on the selected channel to stabilize after switching.
    busy_wait_us_32(MUX_SETTLE_US);

    // --- STAGE 2: ACQUISITION (AVERAGING) ---
    uint32_t sum = 0;
    for (int n = 0; n < MUX_SAMPLES; n++) {
        // Read the ADC value multiple times and accumulate the sum for better accuracy.
        sum += adc_read();
    }

    // --- STAGE 3: CALCULATION & RETURN ---
    // Return the average reading across all samples.
    return (uint16_t)(sum / MUX_SAMPLES);
}