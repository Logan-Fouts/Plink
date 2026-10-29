#pragma once

#include <stdint.h>

// Front-panel push buttons (currently: transpose down / up).
void controls_init(void);
void controls_update(uint32_t now_us);
