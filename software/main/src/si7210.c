#include "si7210.h"

float l_fScale = 0.00125f;

static si7210_callback_fn_t pfTriggerCallback = NULL;

static uint8_t si7210_read_register(uint8_t ubRegister)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c0_write_byte(SI7210_I2C_ADDR, ubRegister, I2C_RESTART);
		return i2c0_read_byte(SI7210_I2C_ADDR, I2C_STOP);
	}
}
static void si7210_write_register(uint8_t ubRegister, uint8_t ubValue)
{
	uint8_t pubBuffer[2];

    pubBuffer[0] = ubRegister;
    pubBuffer[1] = ubValue;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c0_write(SI7210_I2C_ADDR, pubBuffer, 2, I2C_STOP);
	}
}
static void si7210_rmw_register(uint8_t ubRegister, uint8_t ubMask, uint8_t ubValue)
{
	si7210_write_register(ubRegister, (si7210_read_register(ubRegister) & ubMask) | ubValue);
}
static uint8_t si7210_read_otp_register(uint8_t ubRegister)
{
	si7210_write_register(SI7210_OTPADDR, ubRegister); // load otp address to read

    si7210_write_register(SI7210_OTPREAD, SI7210_BIT_OTPREADEN); // set read en to start otp read

    while(si7210_read_register(SI7210_OTPREAD) & SI7210_BIT_OTPBUSY); // wait for the read to complete

    return si7210_read_register(SI7210_OTPDAT); // return the data loaded from the otp memory
}

uint8_t si7210_init()
{
	if(((si7210_get_chip_id() >> 4) & 0xFF) != 0x01) // Check chip id (0x1 for all Si7210 parts.)
		return 0;

    si7210_write_register(SI7210_SLTIME, 25);
    si7210_write_register(SI7210_SWTAMPER, (63 << SI7210_BIT_OFF_SWTAMPER));
    si7210_write_register(SI7210_MEAS, SI7210_BIT_USESTORE);

    uint8_t ubAnCfg[6] = {0};
    si7210_load_otp_analog_cfg(ubAnCfg, SI7210_OTP_SSCN_A0);
    si7210_set_analog_cfg(ubAnCfg);
    si7210_set_scale(0.0125f);
    si7210_cfg_digital_filtering(2, 2, 0);
    si7210_cfg_out_pin(0, 0b00, 25, 15);

    return 1;
}
void si7210_isr()
{
    if(pfTriggerCallback)
        pfTriggerCallback();
}

void si7210_set_trigger_callback(si7210_callback_fn_t pfFunc)
{
    pfTriggerCallback = pfFunc;
}

uint8_t si7210_get_chip_id()
{
    return si7210_read_register(SI7210_CHIP_ID);
}
uint32_t si7210_get_serial_num()
{
    uint32_t ullSNum = 0;

    ullSNum |= ((uint32_t)si7210_read_otp_register(SI7210_OTP_SERIANUM0) << 24) & 0xFF000000;
    ullSNum |= ((uint32_t)si7210_read_otp_register(SI7210_OTP_SERIANUM1) << 16) & 0x00FF0000;
    ullSNum |= ((uint32_t)si7210_read_otp_register(SI7210_OTP_SERIANUM2) << 8) & 0x0000FF00;
    ullSNum |= ((uint32_t)si7210_read_otp_register(SI7210_OTP_SERIANUM3)) & 0x000000FF;

    return ullSNum;
}

void si7210_load_otp_analog_cfg(uint8_t *pubBuf, uint8_t ubAddr)
{
    switch(ubAddr)
    {
        case SI7210_OTP_PU_A0: // the power up values have an address gap
            for(uint8_t ubOff = 0; ubOff < 3; ubOff++)
            {
                *(pubBuf + ubOff) = si7210_read_otp_register(SI7210_OTP_PU_A0 + ubOff);
            }
            for(uint8_t ubOff = 0; ubOff < 3; ubOff++)
            {
                *(pubBuf + ubOff + 3) = si7210_read_otp_register(SI7210_OTP_PU_A3 + ubOff);
            }
            break;

        case SI7210_OTP_SSNC_A0:
        case SI7210_OTP_LSNC_A0:
        case SI7210_OTP_SSCN_A0:
        case SI7210_OTP_LSCN_A0:
        case SI7210_OTP_SSCC_A0:
        case SI7210_OTP_LSCC_A0:
            for(uint8_t ubOff = 0; ubOff < 6; ubOff++)
            {
                *(pubBuf + ubOff) = si7210_read_otp_register(ubAddr + ubOff);
            }
            break;

        default:    // invalid address
            return;
    }
}
void si7210_set_analog_cfg(uint8_t *pubBuf)
{
    for(uint8_t ubOff = 0; ubOff < 3; ubOff++)
    {
        si7210_write_register(SI7210_A0 + ubOff, *(pubBuf + ubOff));
    }
    for(uint8_t ubOff = 0; ubOff < 3; ubOff++)
    {
        si7210_write_register(SI7210_A3 + ubOff, *(pubBuf + 3 + ubOff));
    }
}

void si7210_cfg_digital_filtering(uint8_t ubBurstSize, uint8_t ubSampleAvgSize, uint8_t ubFIRmode)
{
    si7210_write_register(SI7210_DFBURST, ((ubBurstSize << SI7210_BIT_OFF_DFBURSTSIZE) & 0xE0) | ((ubSampleAvgSize << SI7210_BIT_OFF_DFBW) & 0x1E) | (ubFIRmode & 0x01));
}

void si7210_cfg_out_pin(uint8_t ubPinPol, uint8_t ubFieldPol, uint8_t ubDecisionPointCenter, uint8_t ubhysteresis)
{
    if(ubDecisionPointCenter > 127)
        ubDecisionPointCenter = 127;

    if(ubhysteresis > 63)
        ubhysteresis = 63;

    si7210_write_register(SI7210_SWOP, ((ubPinPol & 0x01) << 7) | ubDecisionPointCenter);
    si7210_write_register(SI7210_SWHYST, (ubFieldPol << SI7210_BIT_OFF_SWFIELDPOLSEL) | ubhysteresis);
}

float si7210_get_data()
{
    si7210_rmw_register(SI7210_MEAS, 0b11111000, SI7210_BIT_ONEBURST);

    while(si7210_read_register(SI7210_MEAS) & 0x80);

    uint16_t usDspsig = ((uint16_t)si7210_read_register(SI7210_DSPSIGH) << 8) & 0x7F00;
    usDspsig |= (uint16_t)si7210_read_register(SI7210_DSPSIGL) & 0x00FF;

    float fField = ((float)usDspsig - 16384.f) * l_fScale;

    si7210_rmw_register(SI7210_MEAS, 0b11111000, 0);

    return fField;
}
void si7210_set_scale(float fScale)
{
    l_fScale = fScale;
}