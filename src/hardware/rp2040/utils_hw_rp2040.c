#include "hardware/utils_hw.h"
#include <stdbool.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "config.h"

void utils_hw_sleep(uint32_t us) {
  busy_wait_us_32(us);
}

void utils_hw_debug_write(char buf[DEBUG_BUFFER_SIZE]) {
  tud_cdc_write_str(buf);
  tud_cdc_write_flush();
}

bool utils_hw_tud_connected(void) {
  return(tud_cdc_connected());
}

uint32_t time_us(void) {
  return(time_us_32());
}

void utils_hw_midi_stream_write(uint8_t cable_num, uint8_t msg[3], uint32_t bufsize) {
  tud_midi_stream_write(cable_num, msg, bufsize);
}

