#ifndef MUX_HW_FAKE_H
#define MUX_HW_FAKE_H
#include <stdint.h>

void mux_fake_reset(void);
int  mux_fake_pin_level(int pin);  // -1 if never set
void mux_fake_set_adc_value(uint16_t v);

#endif
