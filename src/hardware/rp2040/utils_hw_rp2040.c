#include "pico/stdlib.h"

void utils_hw_sleep(int us) {
  busy_wait_us_32(us);
}
