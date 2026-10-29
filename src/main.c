#include "pico/stdlib.h"
#include "tusb.h"
#include "pico/multicore.h"
#include "pico/time.h"

#include "input/keys.h"
#include "leds/leds.h"
#include "debug/debug.h"
#include "input/controls.h"

#define FLAG_VALUE 123 // Arbitrary value to pass between cores 1 and 2 for handshake


// TODO: Add a linter
// TODO: Pitch Shifter?
void core1_entry() {

  multicore_fifo_push_blocking(FLAG_VALUE);
  multicore_fifo_pop_blocking();
  debug_printf("Core 1 Started\r\n"); // TODO: NOT TREAD SAFE use a safe queue with locks or for small info the FIFO


  while (true) {
    if (tud_cdc_connected()) {
      debug_printf("Core 1 running\r\n");
      sleep_ms(1000);
    }
  }
}


int main(void) {
  // Initialize the hardware and peripherals and start the USB stack on core 0
  stdio_init_all();
  tusb_init();
  controls_init();
  keys_init();
  tud_task();

  // Start core 1
  multicore_launch_core1(core1_entry);

  // Wait for core 1 to start
  multicore_fifo_pop_blocking();
  multicore_fifo_push_blocking(FLAG_VALUE);
  debug_printf("Handshake complete\r\n");

  // Run the main loop on core 0
  while (true) {
    tud_task();                          // USB housekeeping

    // Discard incoming data so the buffers never fill up
    uint8_t packet[4];
    while (tud_midi_available()) tud_midi_packet_read(packet);
    while (tud_cdc_available())  tud_cdc_read_char();

    uint32_t now_us = time_us_32();
    controls_update(now_us);
    keys_update(now_us);
    leds_update();                       // send LED changes (if any)
  }
}
