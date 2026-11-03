#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"  // generated from ws2812.pio at build time
#include "hardware/leds_hw.h"
#include "config.h"

static PIO led_pio;
static int led_sm;

void leds_hw_init() {
  led_pio = pio0;

  uint offset = pio_add_program(led_pio, &ws2812_program);
  led_sm      = pio_claim_unused_sm(led_pio, true);
  ws2812_program_init(led_pio, led_sm, offset, LED_PIN, LED_BITRATE_HZ, false);
}

void leds_hw_pio_put_blocking(uint32_t pixel_value) {
  pio_sm_put_blocking(led_pio, led_sm, pixel_value << 8u);
}
