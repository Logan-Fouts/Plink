#include "debug/debug.h"

#include <stdarg.h>
#include <stdio.h>
#include "tusb.h"

void debug_printf(const char *fmt, ...) {
    if (!tud_cdc_connected()) return;   // no terminal open, skip

    char buf[96];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    tud_cdc_write_str(buf);
    tud_cdc_write_flush();
}