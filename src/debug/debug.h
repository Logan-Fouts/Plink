#pragma once

// Print over USB serial (CDC). Does nothing if no terminal is open.
// Works like printf: debug_printf("key %d pressed\r\n", i);
void debug_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
