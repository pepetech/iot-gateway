#ifndef __SI7021_H__
#define __SI7021_H__

#include <em_device.h>
#include <stdlib.h>
#include "systick.h"
#include "atomic.h"
#include "i2c.h"

#define SI7021_I2C_ADDR 0x40

// Commands
#define SI7021_CMD_MEAS_RH_HOLD     0xE5
#define SI7021_CMD_MEAS_RH_NOHOLD   0xF5
#define SI7021_CMD_MEAS_TEMP_HOLD   0xE3
#define SI7021_CMD_MEAS_TEMP_NOHOLD 0xF3
#define SI7021_CMD_READ_PREV_TEMP   0xE0
#define SI7021_CMD_RESET            0xFE
#define SI7021_CMD_WRITE_USER       0xE6
#define SI7021_CMD_READ_USER        0xE7
#define SI7021_CMD_WRITE_HEATER     0x51
#define SI7021_CMD_READ_HEATER      0x11
#define SI7021_CMD_READ_UID_A1      0xFA
#define SI7021_CMD_READ_UID_A2      0x0F
#define SI7021_CMD_READ_UID_B1      0xFC
#define SI7021_CMD_READ_UID_B2      0xC9
#define SI7021_CMD_READ_FW_VERSION1 0x84
#define SI7021_CMD_READ_FW_VERSION2 0xB8

// Times
#define SI7021_T_START  80

// User register
#define SI7021_USER_RES_RH12_T14    0x00
#define SI7021_USER_RES_RH8_T12     0x01
#define SI7021_USER_RES_RH10_T13    0x80
#define SI7021_USER_RES_RH11_T11    0x81
#define SI7021_USER_VDD_OK          0x00
#define SI7021_USER_VDD_LOW         0x40
#define SI7021_USER_HEATER_ON       0x04
#define SI7021_USER_HEATER_OFF      0x00

// Heater control register
#define SI7021_HEATER_3p09mA    0x00
#define SI7021_HEATER_9p18mA    0x01
#define SI7021_HEATER_15p24mA   0x02
#define SI7021_HEATER_21p3mA    0x03
#define SI7021_HEATER_27p39mA   0x04
#define SI7021_HEATER_33p45mA   0x05
#define SI7021_HEATER_39p51mA   0x06
#define SI7021_HEATER_45p57mA   0x07
#define SI7021_HEATER_51p69mA   0x08
#define SI7021_HEATER_57p75mA   0x09
#define SI7021_HEATER_63p81mA   0x0A
#define SI7021_HEATER_69p87mA   0x0B
#define SI7021_HEATER_75p93mA   0x0C
#define SI7021_HEATER_81p99mA   0x0D
#define SI7021_HEATER_88p05mA   0x0E
#define SI7021_HEATER_94p20mA   0x0F

// Firmware revision
#define SI7021_FW_REV_1p0   0xFF
#define SI7021_FW_REV_2p0   0x20

// Device
#define SI7021_SNB3_DEV_ENG_SAMPLE0   0x00
#define SI7021_SNB3_DEV_ENG_SAMPLE1   0xFF
#define SI7021_SNB3_DEV_7013          0x0D
#define SI7021_SNB3_DEV_7020          0x14
#define SI7021_SNB3_DEV_7021          0x15


uint8_t si7021_init();

void si7021_software_reset();

float si7021_read_temperature();
float si7021_read_humidity();

void si7021_set_heater_current(uint8_t ubCurrent);
uint8_t si7021_get_heater_current();

void si7021_write_user(uint8_t ubUser);
uint8_t si7021_read_user();

uint8_t si7021_read_firmware_version();
uint64_t si7021_read_unique_id();

#endif // __SI7021_H__
