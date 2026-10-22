#include "hardware/utils_hw.h"
#include "fakes/utils_hw_fake.h"

static uint32_t last_sleep_us;

void utils_fake_reset(void)            { last_sleep_us = 0; }
uint32_t utils_fake_last_sleep_us(void) { return last_sleep_us; }

void utils_hw_sleep(uint32_t us)       { last_sleep_us = us; }
