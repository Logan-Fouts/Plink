#include "leds/leds.h"

#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"      // generated from ws2812.pio at build time
#include "config.h"

// Each LED takes ~30 us to send (24 bits at 800 kHz); after a frame the line
// must stay low for ~300 us so the LEDs latch the new colours.
#define FRAME_TIME_US   (LED_COUNT * 30u + 300u)

static PIO      led_pio;
static uint     led_sm;
static uint32_t pixels[LED_COUNT];     // packed 0xGGRRBB, brightness already applied
static bool     dirty = false;
static uint32_t last_send_us = 0;

static inline uint32_t pack_grb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

void leds_init(void) {
    led_pio = pio0;
    uint offset = pio_add_program(led_pio, &ws2812_program);
    led_sm = pio_claim_unused_sm(led_pio, true);
    ws2812_program_init(led_pio, led_sm, offset, LED_PIN, 800000, false);

    last_send_us = time_us_32() - FRAME_TIME_US;
    leds_clear();
    leds_update(); 
}

void leds_set_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= LED_COUNT || index < 0) return;

    uint32_t value = pack_grb((uint8_t)((r * LED_BRIGHTNESS) / 255),
                              (uint8_t)((g * LED_BRIGHTNESS) / 255),
                              (uint8_t)((b * LED_BRIGHTNESS) / 255));
    if (pixels[index] != value) {
        pixels[index] = value;
        dirty = true;
    }
}

void leds_off(uint8_t index) {
    leds_set_color(index, 0, 0, 0);
}

void leds_clear(void) {
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        leds_off(i);
    }
    dirty = true;
}

void leds_update(void) {
    if (!dirty) return;

    uint32_t now = time_us_32();
    if ((now - last_send_us) < FRAME_TIME_US) return;   // previous frame still going

    last_send_us = now;
    dirty = false;

    // The state machine expects the 24 colour bits at the top of the 32-bit word.
    // This blocks briefly while the FIFO drains (~30 us per LED once it is full).
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        pio_sm_put_blocking(led_pio, led_sm, pixels[i] << 8u);
    }
}