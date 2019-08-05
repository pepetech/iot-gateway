#ifndef __SI7210_H__
#define __SI7210_H__

#include <em_device.h>
#include <stdlib.h>
#include "systick.h"
#include "atomic.h"
#include "gpio.h"
#include "i2c.h"

#include "debug_macros.h"

#define SI7210_I2C_ADDR 0x30
//#define SI7210_I2C_ADDR 0x31
//#define SI7210_I2C_ADDR 0x32
//#define SI7210_I2C_ADDR 0x33

// Registers
#define SI7210_REG_CHIP_ID      0xC0
#define SI7210_REG_DSPSIGH      0xC1
#define SI7210_REG_DSPSIGL      0xC2
#define SI7210_REG_DSPSIGSEL    0xC3
#define SI7210_REG_MEAS         0xC4
#define SI7210_REG_ARAUTOINC    0xC5
#define SI7210_REG_SWOP         0xC6
#define SI7210_REG_SWHYST       0xC7
#define SI7210_REG_SLTIME       0xC8
#define SI7210_REG_SWTAMPER     0xC9
#define SI7210_REG_A0           0xCA
#define SI7210_REG_A1           0xCB
#define SI7210_REG_A2           0xCC
#define SI7210_REG_DF           0xCD // DF - Digital Filter
#define SI7210_REG_A3           0xCE
#define SI7210_REG_A4           0xCF
#define SI7210_REG_A5           0xD0
#define SI7210_REG_OTPADDR      0xE1
#define SI7210_REG_OTPDAT       0xE2
#define SI7210_REG_OTPREAD      0xE3
#define SI7210_REG_TMFG         0xE4

// OTP Registers
#define SI7210_OTP_SWOP         0x04
#define SI7210_OTP_SWHYST       0x05
#define SI7210_OTP_SLTIME       0x06
#define SI7210_OTP_SWTAMPER     0x08
#define SI7210_OTP_PU_A0        0x09    // Power up A0
#define SI7210_OTP_PU_A1        0x0A
#define SI7210_OTP_PU_A2        0x0B
#define SI7210_OTP_PU_A3        0x0D
#define SI7210_OTP_PU_A4        0x0E
#define SI7210_OTP_PU_A5        0x0F
#define SI7210_OTP_BURSTSIZE    0x0C
#define SI7210_OTP_PARTNUM      0x14
#define SI7210_OTP_VARIANT      0x15
#define SI7210_OTP_SERIANUM0    0x18
#define SI7210_OTP_SERIANUM1    0x19
#define SI7210_OTP_SERIANUM2    0x1A
#define SI7210_OTP_SERIANUM3    0x1B
#define SI7210_OTP_TEMPOFFADJ   0x1D
#define SI7210_OTP_TEMPGAINADJ  0x1E
#define SI7210_OTP_SSNC_A0      0x21    // A0 – A5 for 20 mT scale and no magnet temperature compensation
#define SI7210_OTP_SSNC_A1      0x22    // SSNC - Small Scale No Compensate
#define SI7210_OTP_SSNC_A2      0x23
#define SI7210_OTP_SSNC_A3      0x24
#define SI7210_OTP_SSNC_A4      0x25
#define SI7210_OTP_SSNC_A5      0x26
#define SI7210_OTP_LSNC_A0      0x27    // A0 - A5 for 200 mT scale and no magnet temperature compensation
#define SI7210_OTP_LSNC_A1      0x28    // LSNC - Large Scale No Compensate
#define SI7210_OTP_LSNC_A2      0x29
#define SI7210_OTP_LSNC_A3      0x2A
#define SI7210_OTP_LSNC_A4      0x2B
#define SI7210_OTP_LSNC_A5      0x2C
#define SI7210_OTP_SSCN_A0      0x2D    // A0 – A5 for 20 mT scale at 25°C -0.12%/°C magnet temperature compensation (Neodymium)
#define SI7210_OTP_SSCN_A1      0x2E    // SSCN - Small Scale Compensate Neodymium
#define SI7210_OTP_SSCN_A2      0x2F
#define SI7210_OTP_SSCN_A3      0x30
#define SI7210_OTP_SSCN_A4      0x31
#define SI7210_OTP_SSCN_A5      0x32
#define SI7210_OTP_LSCN_A0      0x33    // A0 – A5 for 200 mT scale at 25°C -0.12%/°C magnet temperature compensation (Neodymium)
#define SI7210_OTP_LSCN_A1      0x34    // LSCN - Large Scale Compensate Neodymium
#define SI7210_OTP_LSCN_A2      0x35
#define SI7210_OTP_LSCN_A3      0x36
#define SI7210_OTP_LSCN_A4      0x37
#define SI7210_OTP_LSCN_A5      0x38
#define SI7210_OTP_SSCC_A0      0x39    // A0 – A5 for 20 mT scale at 25°C -0.2%/°C magnet temperature compensation (Ceramic)
#define SI7210_OTP_SSCC_A1      0x3A    // SSCC - Small Scale Compensate Ceramic
#define SI7210_OTP_SSCC_A2      0x3B
#define SI7210_OTP_SSCC_A3      0x3C
#define SI7210_OTP_SSCC_A4      0x3D
#define SI7210_OTP_SSCC_A5      0x3E
#define SI7210_OTP_LSCC_A0      0x3F    // A0 – A5 for 200 mT scale at 25°C -0.2%/°C magnet temperature compensation (Ceramic)
#define SI7210_OTP_LSCC_A1      0x40    // LSCC - Large Scale Compensate Ceramic
#define SI7210_OTP_LSCC_A2      0x41
#define SI7210_OTP_LSCC_A3      0x42
#define SI7210_OTP_LSCC_A4      0x43
#define SI7210_OTP_LSCC_A5      0x44

// SI7210_REG_CHIP_ID
#define SI7210_REG_CHIP_ID_CHIPID   0xF0
#define SI7210_REG_CHIP_ID_REVID    0x0F

// SI7210_REG_DSPSIGSEL
#define SI7210_REG_DSPSIGSEL_DSPSIGSEL  0x07

// SI7210_REG_MEAS
#define SI7210_REG_MEAS_MEAS        0x80
#define SI7210_REG_MEAS_USESTORE    0x08
#define SI7210_REG_MEAS_ONEBURST    0x04
#define SI7210_REG_MEAS_STOP        0x02
#define SI7210_REG_MEAS_SLEEP       0x01

// SI7210_REG_ARAUTOINC
#define SI7210_REG_ARAUTOINC_ARAUTOINC  0x01

// SI7210_REG_SWOP
#define SI7210_REG_SWOP_SW_LOW4FIELD    0x80
#define SI7210_REG_SWOP_SW_OP           0x7F

// SI7210_REG_SWHYST
#define SI7210_REG_SWHYST_SW_FIELDPOLSEL    0xC0
#define SI7210_REG_SWHYST_SW_HYST           0x3F

// SI7210_REG_SWTAMPER
#define SI7210_REG_SWTAMPER_SW_TAMPER   0xFC
#define SI7210_REG_SWTAMPER_SLFAST      0x02
#define SI7210_REG_SWTAMPER_SLTIMEENA   0x01

// SI7210_REG_DF
#define SI7210_REG_DF_DF_BURSTSIZE      0xE0
#define SI7210_REG_DF_DF_BW             0x1E
#define SI7210_REG_DF_DF_IIR            0x01

// SI7210_REG_OTPREAD
#define SI7210_REG_OTPREAD_OTP_READ_EN  0x02
#define SI7210_REG_OTPREAD_OTP_BUSY     0x01

// SI7210_REG_TMFG
#define SI7210_REG_TMFG_TM_FG           0x01

typedef void (* si7210_callback_fn_t)();

uint8_t si7210_init();
void si7210_isr();

void si7210_set_trigger_callback(si7210_callback_fn_t pfFunc);

uint8_t si7210_get_chip_id();
uint32_t si7210_get_serial_num();

void si7210_load_otp_analog_cfg(uint8_t *pubBuf, uint8_t addr);
void si7210_set_analog_cfg(uint8_t *pubBuf);

void si7210_cfg_digital_filtering(uint8_t ubBurstSize, uint8_t ubSampleAvgSize, uint8_t ubIIRmode);
void si7210_cfg_out_pin(uint8_t ubPinPol, uint8_t ubFieldPol, uint8_t ubDecisionPointCenter, uint8_t ubHysteresis);

void si7210_set_scale(float fScale);

float si7210_read_mag_field();

#endif // __SI7210_H__