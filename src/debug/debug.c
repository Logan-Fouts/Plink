#include "debug/debug.h"
#include "hardware/utils_hw.h"
#include "config.h"

#include <stdarg.h>
#include <stdio.h>

void debug_printf(const char *fmt, ...) {
  if (!utils_hw_tud_connected() || !DEBUG) return;  // No terminal open or debugging off, skip

  char    buf[DEBUG_BUFFER_SIZE];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  utils_hw_debug_write(buf);
}
