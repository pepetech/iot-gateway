#include "ws2812.h"

static inline __attribute__((always_inline)) uint32_t _get_tick_count()
{
  uint32_t cx;
  __asm__ __volatile__("rsr %0,ccount":"=a" (cx));
  return cx;
}

void ws2812_write_data(const uint8_t pin, uint8_t *pixels, const uint32_t length)
{
    uint32_t pin_mask = ((uint32_t)1 << pin);
    uint8_t* end = pixels + length;
    uint8_t current_pixel = 0;
    uint8_t mask = 0x80;
    uint32_t start_tick = 0;
    uint32_t current_tick = 0;
    uint32_t hold_time;

    if(pixels)
        current_pixel = *pixels++;
    else
        end--;

    uint32_t f800_t0h = (ets_get_cpu_frequency() * 1000000 / 2500000);
    uint32_t f800_t1h = (ets_get_cpu_frequency() * 1000000 / 1250000);
    uint32_t f800_cycle = (ets_get_cpu_frequency() * 1000000 / 800000);

    while(1)
    {
        if(current_pixel & mask)
            hold_time = f800_t1h;
        else
            hold_time = f800_t0h;

        while(((current_tick = _get_tick_count()) - start_tick) < f800_cycle);
        REG(GPIO_OUT_SET) = pin_mask;

        start_tick = current_tick;

        while(((current_tick = _get_tick_count()) - start_tick) < hold_time);
        REG(GPIO_OUT_CLEAR) = pin_mask;

        if(!(mask >>= 1))
        {
            if(pixels >= end)
                break;

            if(pixels)
                current_pixel = *pixels++;
            else
                end--;

            mask = 0x80;
        }
    }

    while((_get_tick_count() - start_tick) < f800_cycle);
}
