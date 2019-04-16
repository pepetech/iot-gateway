#ifndef __GPIO_H__
#define __GPIO_H__

#include <c_types.h>
#include "registers.h"

static const uint8_t IOMUX_REG_OFFS[] = {13, 6, 14, 5, 15, 16, 7, 8, 9, 10, 11, 12, 1, 2, 3, 4};
static const uint32_t IOMUX_GPIO_FUNC[] = {
    IOMUX_GPIO0_FUNC_GPIO,
    IOMUX_GPIO1_FUNC_GPIO,
    IOMUX_GPIO2_FUNC_GPIO,
    IOMUX_GPIO3_FUNC_GPIO,
    IOMUX_GPIO4_FUNC_GPIO,
    IOMUX_GPIO5_FUNC_GPIO,
    IOMUX_GPIO6_FUNC_GPIO,
    IOMUX_GPIO7_FUNC_GPIO,
    IOMUX_GPIO8_FUNC_GPIO,
    IOMUX_GPIO9_FUNC_GPIO,
    IOMUX_GPIO10_FUNC_GPIO,
    IOMUX_GPIO11_FUNC_GPIO,
    IOMUX_GPIO12_FUNC_GPIO,
    IOMUX_GPIO13_FUNC_GPIO,
    IOMUX_GPIO14_FUNC_GPIO,
    IOMUX_GPIO15_FUNC_GPIO
};

extern uint32_t get_gpio(uint32_t gpio_num);

#endif
