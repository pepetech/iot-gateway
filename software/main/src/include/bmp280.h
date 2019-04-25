#ifndef __BMP280_H__
#define __BMP280_H__

#include <em_device.h>
#include <stdlib.h>
#include "systick.h"
#include "atomic.h"
#include "i2c.h"

#define BMP280_I2C_ADDR 0x76

// Registers
#define BMP280_REG_DIG_T1_L		0x88
#define BMP280_REG_DIG_T1_H		0x89
#define BMP280_REG_DIG_T2_L		0x8A
#define BMP280_REG_DIG_T2_H		0x8B
#define BMP280_REG_DIG_T3_L		0x8C
#define BMP280_REG_DIG_T3_H		0x8D

#define BMP280_REG_DIG_P1_L		0x8E
#define BMP280_REG_DIG_P1_H		0x8F
#define BMP280_REG_DIG_P2_L		0x90
#define BMP280_REG_DIG_P2_H		0x91
#define BMP280_REG_DIG_P3_L		0x92
#define BMP280_REG_DIG_P3_H		0x93
#define BMP280_REG_DIG_P4_L		0x94
#define BMP280_REG_DIG_P4_H		0x95
#define BMP280_REG_DIG_P5_L		0x96
#define BMP280_REG_DIG_P5_H		0x97
#define BMP280_REG_DIG_P6_L		0x98
#define BMP280_REG_DIG_P6_H		0x99
#define BMP280_REG_DIG_P7_L		0x9A
#define BMP280_REG_DIG_P7_H		0x9B
#define BMP280_REG_DIG_P8_L		0x9C
#define BMP280_REG_DIG_P8_H		0x9D
#define BMP280_REG_DIG_P9_L		0x9E
#define BMP280_REG_DIG_P9_H		0x9F

#define BMP280_REG_HW_ID		0xD0
#define BMP280_REG_VERSION		0xD1 // Not in datasheet, from Aafruit Arduino lib
#define BMP280_REG_SW_RESET		0xE0

#define BMP280_REG_STATUS		0XF3
#define BMP280_REG_CONTROL		0xF4
#define BMP280_REG_CONFIG		0xF5
#define BMP280_REG_PRESSURE_H	0xF7
#define BMP280_REG_PRESSURE_L	0xF8
#define BMP280_REG_PRESSURE_XL	0xF9
#define BMP280_REG_TEMP_H		0xFA
#define BMP280_REG_TEMP_L		0xFB
#define BMP280_REG_TEMP_XL		0xFC

// Times
#define BMP280_T_START_PON		2
#define BMP280_T_START_RESET	10 // Not in datasheet, guessing
#define BMP280_T_CALIB_READ		50 // Not in datasheet, guessing

// BMP280_REG_STATUS
#define BMP280_MEASURING			0x80
#define BMP280_READING_CALIB		0x01

// BMP280_REG_CONTROL
#define BMP280_TEMP_DISABLED		0x00
#define BMP280_TEMP_OS1				0x20
#define BMP280_TEMP_OS2				0x40
#define BMP280_TEMP_OS4				0x60
#define BMP280_TEMP_OS8				0x80
#define BMP280_TEMP_OS16			0xA0
#define BMP280_PRESSURE_DISABLED	0x00
#define BMP280_PRESSURE_OS1			0x04
#define BMP280_PRESSURE_OS2			0x08
#define BMP280_PRESSURE_OS4			0x0C
#define BMP280_PRESSURE_OS8			0x10
#define BMP280_PRESSURE_OS16		0x14
#define BMP280_MODE_SLEEP			0x00
#define BMP280_MODE_FORCED			0x01
#define BMP280_MODE_NORMAL			0x03

// BMP280_REG_CONFIG
#define BMP280_STANDBY_500US		0x00
#define BMP280_STANDBY_62500US		0x20
#define BMP280_STANDBY_125MS		0x40
#define BMP280_STANDBY_250MS		0x60
#define BMP280_STANDBY_500MS		0x80
#define BMP280_STANDBY_1000MS		0xA0
#define BMP280_STANDBY_10MS			0xC0
#define BMP280_STANDBY_20MS			0xE0
#define BMP280_FILTER_OFF			0x00
#define BMP280_FILTER_2				0x04
#define BMP280_FILTER_4				0x08
#define BMP280_FILTER_8				0x0C
#define BMP280_FILTER_16			0x10
#define BMP280_SPI_3W				0x01


uint8_t bmp280_init();

void bmp280_software_reset();

float bmp280_read_temperature();
float bmp280_read_pressure();

void bmp280_take_forced_meas();

uint8_t bmp280_read_status();
uint8_t bmp280_read_id();
uint8_t bmp280_read_version();

void bmp280_write_control(uint8_t ubControl);
uint8_t bmp280_read_control();

void bmp280_write_config(uint8_t ubConfig);
uint8_t bmp280_read_config();

#endif // __BMP280_H__
