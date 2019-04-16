#ifndef __WS2812_H__
#define __WS2812_H__

#include <c_types.h>
#include "user_interface.h"
#include "registers.h"

void ws2812_write_data(const uint8_t pin, uint8_t *pixels, const uint32_t length);

#endif
