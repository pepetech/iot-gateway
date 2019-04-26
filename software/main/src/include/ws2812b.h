#ifndef __WS2812B_H__
#define __WS2812B_H__

#include <em_device.h>
#include <stdlib.h>
#include <string.h>
#include "cmu.h"
#include "ldma.h"
#include "utils.h"

#define WS2812B_NUM_LEDS		2
#define WS2812B_FREQ            800000UL
#define WS2812B_T0H             0.000000400f // 400 ns
#define WS2812B_T1H             0.000000800f // 800 ns
#define WS2812B_INV             1
#define WS2812B_DMA_CHANNEL		12

void ws2812b_init();

void ws2812b_set_color(uint16_t usLED, uint8_t ubRed, uint8_t ubGreen, uint8_t ubBlue);

#endif // __WS2812B_H__
