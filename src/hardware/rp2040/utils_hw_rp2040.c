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

bool utils_hw_tud_conencted(void) {
  return(tud_cdc_connected());
}

uint32_t time_us(void) {
  return(time_us_32());
}

