#ifndef __CCS811_H__
#define __CCS811_H__

#include <em_device.h>
#include <stdlib.h>
#include "systick.h"
#include "atomic.h"
#include "gpio.h"
#include "i2c.h"

#define CCS811_I2C_ADDR 0x5A

// Registers
#define CCS811_REG_STATUS           0x00
#define CCS811_REG_MEAS_MODE        0x01
#define CCS811_REG_ALG_RESULT_DATA  0x02
#define CCS811_REG_RAW_DATA         0x03
#define CCS811_REG_ENV_DATA         0x05
#define CCS811_REG_NTC              0x06
#define CCS811_REG_THRESHOLDS       0x10
#define CCS811_REG_BASELINE         0x11
#define CCS811_REG_HW_ID            0x20
#define CCS811_REG_HW_VERSION       0x21
#define CCS811_REG_BL_FW_VERSION    0x23
#define CCS811_REG_APP_FW_VERSION   0x24
#define CCS811_REG_INTERNAL_STATUS  0xA0
#define CCS811_REG_ERROR_ID         0xE0
#define CCS811_REG_APP_ERASE        0xF1
#define CCS811_REG_APP_DATA         0xF2
#define CCS811_REG_APP_VERIFY       0xF3
#define CCS811_REG_APP_START        0xF4
#define CCS811_REG_SW_RESET         0xFF

// Times
#define CCS811_T_START          70
#define CCS811_T_AWAKE          1
#define CCS811_T_SLEEP          1
#define CCS811_T_RESET          10
#define CCS811_T_APP_ERASE      500
#define CCS811_T_APP_VERIFY     500
#define CCS811_T_APP_DATA       50
#define CCS811_T_APP_START      5

// CCS811_REG_STATUS
#define CCS811_REG_STATUS_FW_MODE_BOOT     0x00
#define CCS811_REG_STATUS_FW_MODE_APP      0x80
#define CCS811_REG_STATUS_APP_ERASED       0x40
#define CCS811_REG_STATUS_APP_VERIFIED     0x20
#define CCS811_REG_STATUS_APP_VALID        0x10
#define CCS811_REG_STATUS_DATA_AVAILABLE   0x08
#define CCS811_REG_STATUS_ERROR            0x01

// CCS811_REG_MEAS_MODE
#define CCS811_REG_MEAS_MODE_DRIVE_MODE_IDLE  0x00
#define CCS811_REG_MEAS_MODE_DRIVE_MODE_1S    0x10
#define CCS811_REG_MEAS_MODE_DRIVE_MODE_10S   0x20
#define CCS811_REG_MEAS_MODE_DRIVE_MODE_60S   0x30
#define CCS811_REG_MEAS_MODE_DRIVE_MODE_250MS 0x40
#define CCS811_REG_MEAS_MODE_INT_DATARDY      0x08
#define CCS811_REG_MEAS_MODE_INT_THRESH       0x04

// CCS811_REG_APP_ERROR_ID
#define CCS811_REG_APP_ERROR_ID_ERROR_HEATER_SUPPLY      0x20
#define CCS811_REG_APP_ERROR_ID_ERROR_HEATER_FAULT       0x10
#define CCS811_REG_APP_ERROR_ID_ERROR_MAX_RESISTANCE     0x08
#define CCS811_REG_APP_ERROR_ID_ERROR_MEASMODE_INVALID   0x04
#define CCS811_REG_APP_ERROR_ID_ERROR_READ_REG_INVALID   0x02
#define CCS811_REG_APP_ERROR_ID_ERROR_WRITE_REG_INVALID  0x01

// Utility
#define CCS811_SW_VERSION_MAJOR(x)      (((x) & 0xF000) >> 12)
#define CCS811_SW_VERSION_MINOR(x)      (((x) & 0x0F00) >> 8)
#define CCS811_SW_VERSION_TRIVIAL(x)    (((x) & 0x00FF) >> 0)


uint8_t ccs811_init();

void ccs811_app_erase();
void ccs811_app_send_data(uint8_t *pubSrc);
uint8_t ccs811_app_verify();
void ccs811_app_start();

void ccs811_software_reset();
void ccs811_hardware_reset();

uint16_t ccs811_read_etvoc();
uint16_t ccs811_read_eco2();

uint8_t ccs811_read_status();
uint8_t ccs811_read_internal_status();
uint8_t ccs811_read_error();
uint8_t ccs811_read_hw_id();
uint8_t ccs811_read_hw_version();
uint16_t ccs811_read_boot_version();
uint16_t ccs811_read_app_version();

void ccs811_write_meas_mode(uint8_t ubMode);
uint8_t ccs811_read_meas_mode();

void ccs811_write_env_data(float fTemp, float fHumid);

void ccs811_write_tresh(uint16_t usLowToMed, uint16_t usMedToHigh);

void ccs811_write_baseline(uint16_t usBaseline);
uint16_t ccs811_read_baseline();

#endif // __CCS811_H__
