#ifndef __ADC_H__
#define __ADC_H__

#include <em_device.h>
#include "cmu.h"

#define ADC_5V0_DIV             3.12765957f // Voltage divider ratio
#define ADC_4V2_DIV             2.f         // Voltage divider ratio
#define ADC_VBAT_DIV            2.f         // Voltage divider ratio
#define ADC_VIN_DIV             7.38297872f // Voltage divider ratio

#define ADC_5V0_CHAN             ADC_SINGLECTRL_POSSEL_APORT0XCH4
#define ADC_4V2_CHAN             ADC_SINGLECTRL_POSSEL_APORT0XCH5
#define ADC_VBAT_CHAN            ADC_SINGLECTRL_POSSEL_APORT0XCH6
#define ADC_VIN_CHAN             ADC_SINGLECTRL_POSSEL_APORT0XCH7

#define LOW_BAT_VOLTAGE         3550.f
#define LOW_BAT_VOLTAGE_HYST    100.f

void adc_init();

float adc_get_avdd();
float adc_get_dvdd();
float adc_get_iovdd();
float adc_get_corevdd();
float adc_get_5v0();
float adc_get_4v2();
float adc_get_vbat();
float adc_get_vin();

float adc_get_temperature();

#endif  // __ADC_H__
