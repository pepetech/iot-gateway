#include "ets_func.h"
#include "gpio.h"

uint32_t get_gpio(uint32_t gpio_num)
{
    if(gpio_num >= 0 && gpio_num < 16)
    {
        uint32_t IOMUX_REG = REG_IOMUX_BASE(IOMUX_REG_OFFS[gpio_num]);

        // Store old values
        uint32_t old_out = REG(GPIO_ENABLE_OUT);
        uint32_t old_mux = REG(IOMUX_REG);

    	REG(GPIO_ENABLE_OUT_CLEAR) = BIT(gpio_num); // Set as input

        REG(IOMUX_REG) = (REG_IOMUX_BASE(IOMUX_REG) & ~IOMUX_PIN_FUNC_MASK) | IOMUX_GPIO_FUNC[gpio_num] | IOMUX_PIN_PULLUP; // Configure as GPIO with pullup

    	ets_delay_us(10); // Allow soft pullup to take effect if line was floating

    	int result = REG(GPIO_IN) & BIT(gpio_num); // Read input

    	// Set IOMUX & GPIO output buffer back to initial values
        REG(GPIO_ENABLE_OUT) = old_out;
        REG(IOMUX_REG) = old_mux;

    	return !!result;
    }
    else if(gpio_num == 16)
    {
        REG(RTC_GPIO_CFG(3)) &= ~(RTC_GPIO_CFG3_PIN_FUNC_M << RTC_GPIO_CFG3_PIN_FUNC_S);
        REG(RTC_GPIO_CFG(3)) |= RTC_GPIO_CFG3_PIN_FUNC_RTC_GPIO0 | RTC_GPIO_CFG3_PIN_PULLUP; // Configure iomux as RTCGPIO0 (GPIO16) with pullup
        REG(RTC_GPIO_CONF) &= ~RTC_GPIO_CONF_OUT_ENABLE; // Disable control by RTC, controlled by user now
        REG(RTC_GPIO_ENABLE) &= ~BIT(0); // Set RTCGPIO0 as input

        return REG(RTC_GPIO_IN) & BIT(0);
    }
}
