#include "input/mux.h"

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "config.h"

static const uint8_t select_pins[4] = MUX_SELECT_PINS;

void mux_init(void) {
    adc_init();
    adc_gpio_init(MUX_ADC_GPIO);
    adc_select_input(MUX_ADC_GPIO - 26);     // GP26 = ADC0, GP27 = ADC1, ...

    for (int i = 0; i < 4; i++) {
        gpio_init(select_pins[i]);
        gpio_set_dir(select_pins[i], GPIO_OUT);
        gpio_put(select_pins[i], 0);
    }
}

uint16_t mux_read(uint8_t channel) {
    // The 4 select pins form a binary number that picks the channel.
    for (int i = 0; i < 4; i++) {
        gpio_put(select_pins[i], (channel >> i) & 1);
    }
    busy_wait_us_32(MUX_SETTLE_US);          // let the signal settle

    uint32_t sum = 0;
    for (int n = 0; n < MUX_SAMPLES; n++) {
        sum += adc_read();
    }
    return (uint16_t)(sum / MUX_SAMPLES);
}