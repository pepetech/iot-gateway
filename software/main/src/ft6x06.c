#include "ft6x06.h"

uint8_t ft6x06_init()
{

	if(!i2c1_write(FT6X06_I2C_ADDR, NULL, 0, I2C_STOP)) // Check ACK from the expected address
		return 0;

    return 1;
}

static uint8_t ft6x06_read_register(uint8_t ubRegister)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c1_write_byte(FT6X06_I2C_ADDR, ubRegister, I2C_RESTART);
		return i2c1_read_byte(FT6X06_I2C_ADDR, I2C_STOP);
	}
}
static void ft6x06_write_register(uint8_t ubRegister, uint8_t ubValue)
{
	uint8_t pubBuffer[2];

    pubBuffer[0] = ubRegister;
    pubBuffer[1] = ubValue;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c0_write(FT6X06_I2C_ADDR, pubBuffer, 2, I2C_STOP);
	}
}
static void ft6x06_rmw_register(uint8_t ubRegister, uint8_t ubMask, uint8_t ubValue)
{
	ft6x06_write_register(ubRegister, (ft6x06_read_register(ubRegister) & ubMask) | ubValue);
}

uint8_t ft6x06_get_vendor_id()
{
    return ft6x06_read_register(FT6X06_FOCALTECH_ID);
}
uint8_t ft6x06_get_chip_id()
{
    return ft6x06_read_register(FT6X06_CIPHER);
}
uint8_t ft6x06_get_firmware_version()
{
    return ft6x06_read_register(FT6X06_FIRMID);
}

uint8_t ft6x06_get_point_rate()
{
    return ft6x06_read_register(FT6X06_PERIODACTIVE);
}
uint8_t ft6x06_get_threshold()
{
    return ft6x06_read_register(FT6X06_TH_GROUP);
}
void ft6x06_set_threshold(uint8_t ubThreshold)
{
    ft6x06_write_register(FT6X06_TH_GROUP, ubThreshold);
}

uint8_t ft6x06_get_touch_stat()
{
    return ft6x06_read_register(FT6X06_TD_STATUS);
}

void ft6x06_get_points(touch_points_t *pTpData)
{
    uint8_t ubBuf[14];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c1_write_byte(FT6X06_I2C_ADDR, 0x01, I2C_RESTART);
        i2c1_read(FT6X06_I2C_ADDR, ubBuf, 14, I2C_STOP);
	}

    pTpData->ubID = ubBuf[0];
    pTpData->ubStat = ubBuf[1];
    pTpData->ubEvnt1 = (ubBuf[2] >> 6) & 0x03;
    pTpData->usX1 = ((uint16_t)(ubBuf[2] & 0x03) << 8) | (uint16_t)ubBuf[3];
    pTpData->usX1 = ((uint16_t)(ubBuf[4] & 0x03) << 8) | (uint16_t)ubBuf[5];
    pTpData->ubZ1 = ubBuf[6];
    pTpData->ubA1 = ubBuf[7];
    pTpData->ubEvnt2 = (ubBuf[8] >> 6) & 0x03;
    pTpData->usX2 = ((uint16_t)(ubBuf[8] & 0x03) << 8) | (uint16_t)ubBuf[9];
    pTpData->usX2 = ((uint16_t)(ubBuf[10] & 0x03) << 8) | (uint16_t)ubBuf[11];
    pTpData->ubZ2 = ubBuf[12];
    pTpData->ubA2 = ubBuf[13];
}