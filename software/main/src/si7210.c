#include "si7210.h"

static float fFieldScale = 0.f;

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
	si7210_write_register(SI7210_REG_OTPADDR, ubRegister); // Load OTP address to read

    si7210_write_register(SI7210_REG_OTPREAD, SI7210_REG_OTPREAD_OTP_READ_EN); // Enable read to start OTP read

    while(si7210_read_register(SI7210_REG_OTPREAD) & SI7210_REG_OTPREAD_OTP_BUSY); // Wait for the read to complete

    return si7210_read_register(SI7210_REG_OTPDAT); // Get the data loaded from the OTP memory
}

uint8_t si7210_init()
{
	if(((si7210_get_chip_id() >> 4) & 0xFF) != 0x01) // Check chip id (0x1 for all Si7210 parts.)
		return 0;

    si7210_write_register(SI7210_REG_SLTIME, 25);
    si7210_write_register(SI7210_REG_SWTAMPER, (63 << 2));
    si7210_write_register(SI7210_REG_MEAS, SI7210_REG_MEAS_USESTORE);

    uint8_t ubAnalogCfg[6] = {0};
    si7210_load_otp_analog_cfg(ubAnalogCfg, SI7210_OTP_SSCN_A0);
    si7210_set_analog_cfg(ubAnalogCfg);
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
    return si7210_read_register(SI7210_REG_CHIP_ID);
}
uint32_t si7210_get_serial_num()
{
    uint32_t ulSNum = 0;

    ulSNum |= ((uint32_t)si7210_read_otp_register(SI7210_OTP_SERIANUM0) << 24) & 0xFF000000;
    ulSNum |= ((uint32_t)si7210_read_otp_register(SI7210_OTP_SERIANUM1) << 16) & 0x00FF0000;
    ulSNum |= ((uint32_t)si7210_read_otp_register(SI7210_OTP_SERIANUM2) << 8)  & 0x0000FF00;
    ulSNum |= ((uint32_t)si7210_read_otp_register(SI7210_OTP_SERIANUM3) << 0)  & 0x000000FF;

    return ulSNum;
}

void si7210_load_otp_analog_cfg(uint8_t *pubBuf, uint8_t ubAddr)
{
    if(!pubBuf)
        return;

    switch(ubAddr)
    {
        case SI7210_OTP_PU_A0: // The power up values have an address gap
        {
            for(uint8_t i = 0; i < 3; i++)
                *(pubBuf + i) = si7210_read_otp_register(SI7210_OTP_PU_A0 + i);
            for(uint8_t i = 0; i < 3; i++)
                *(pubBuf + i + 3) = si7210_read_otp_register(SI7210_OTP_PU_A3 + i);
        }
        break;
        case SI7210_OTP_SSNC_A0:
        case SI7210_OTP_LSNC_A0:
        case SI7210_OTP_SSCN_A0:
        case SI7210_OTP_LSCN_A0:
        case SI7210_OTP_SSCC_A0:
        case SI7210_OTP_LSCC_A0:
        {
            for(uint8_t i = 0; i < 6; i++)
                *(pubBuf + i) = si7210_read_otp_register(ubAddr + i);
        }
        break;
        default:
            return;
    }
}
void si7210_set_analog_cfg(uint8_t *pubBuf)
{
    if(!pubBuf)
        return;

    for(uint8_t i = 0; i < 3; i++)
        si7210_write_register(SI7210_REG_A0 + i, *(pubBuf + i));

    for(uint8_t i = 0; i < 3; i++)
        si7210_write_register(SI7210_REG_A3 + i, *(pubBuf + 3 + i));
}

void si7210_cfg_digital_filtering(uint8_t ubBurstSize, uint8_t ubSampleAvgSize, uint8_t ubIIRMode)
{
    si7210_write_register(SI7210_REG_DF, ((ubBurstSize << 5) & SI7210_REG_DF_DF_BURSTSIZE) | ((ubSampleAvgSize << 1) & SI7210_REG_DF_DF_BW) | (!!ubIIRMode & SI7210_REG_DF_DF_IIR));
}
void si7210_cfg_out_pin(uint8_t ubPinPol, uint8_t ubFieldPol, uint8_t ubDecisionPointCenter, uint8_t ubHysteresis)
{
    if(ubDecisionPointCenter > 127)
        ubDecisionPointCenter = 127;

    if(ubHysteresis > 63)
        ubHysteresis = 63;

    si7210_write_register(SI7210_REG_SWOP, (!!ubPinPol << 7) | (ubDecisionPointCenter & SI7210_REG_SWOP_SW_OP));
    si7210_write_register(SI7210_REG_SWHYST, ((ubFieldPol << 6) & SI7210_REG_SWHYST_SW_FIELDPOLSEL) | (ubHysteresis & SI7210_REG_SWHYST_SW_HYST));
}

void si7210_set_scale(float fScale)
{
    fFieldScale = fScale;
}

float si7210_read_mag_field()
{
    si7210_rmw_register(SI7210_REG_MEAS, 0b11111000, SI7210_REG_MEAS_ONEBURST);

    while(si7210_read_register(SI7210_REG_MEAS) & 0x80);

    uint16_t usDspsig = ((uint16_t)si7210_read_register(SI7210_REG_DSPSIGH) << 8) & 0x7F00;
    usDspsig |= (uint16_t)si7210_read_register(SI7210_REG_DSPSIGL) & 0x00FF;

    float fField = ((float)usDspsig - 16384.f) * fFieldScale;

    si7210_rmw_register(SI7210_REG_MEAS, 0b11111000, 0);

    return fField;
}