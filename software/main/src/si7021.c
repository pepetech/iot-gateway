#include "si7021.h"

static uint8_t si7021_read_register(uint8_t ubRegister)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c0_write_byte(SI7021_I2C_ADDR, ubRegister, I2C_RESTART);
		return i2c0_read_byte(SI7021_I2C_ADDR, I2C_STOP);
	}
}
static uint16_t si7021_read_register16(uint8_t ubRegister)
{
	uint16_t usValue;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c0_write_byte(SI7021_I2C_ADDR, ubRegister, I2C_RESTART);
		i2c0_read(SI7021_I2C_ADDR, (uint8_t *)&usValue, 2, I2C_STOP);
	}

	return (usValue << 8) | (usValue >> 8);
}
static void si7021_write_register(uint8_t ubRegister, uint8_t ubValue)
{
	uint8_t pubBuffer[2];

    pubBuffer[0] = ubRegister;
    pubBuffer[1] = ubValue;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c0_write(SI7021_I2C_ADDR, pubBuffer, 2, I2C_STOP);
	}
}
static void si7021_rmw_register(uint8_t ubRegister, uint8_t ubMask, uint8_t ubValue)
{
	si7021_write_register(ubRegister, (si7021_read_register(ubRegister) & ubMask) | ubValue);
}

uint8_t si7021_init()
{
	delay_ms(SI7021_T_START);

	if(!i2c0_write(SI7021_I2C_ADDR, NULL, 0, I2C_STOP)) // Check ACK from the expected address
		return 0;

    si7021_software_reset();

    return 1;
}

void si7021_software_reset()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c0_write_byte(SI7021_I2C_ADDR, SI7021_CMD_RESET, I2C_STOP);
	}

	delay_ms(SI7021_T_START);
}

float si7021_read_temperature()
{
    uint16_t usTemp = si7021_read_register16(SI7021_CMD_MEAS_TEMP_HOLD);

    return (((float)usTemp * 175.72f) / 65536.f) - 46.85f;
}
float si7021_read_humidity()
{
    uint16_t usHumid = si7021_read_register16(SI7021_CMD_MEAS_RH_HOLD);

    return (((float)usHumid * 125.f) / 65536.f) - 6.f;
}

void si7021_set_heater_current(uint8_t ubCurrent)
{
    si7021_write_register(SI7021_CMD_WRITE_HEATER, ubCurrent);
}
uint8_t si7021_get_heater_current()
{
    return si7021_read_register(SI7021_CMD_READ_HEATER);
}


void si7021_write_user(uint8_t ubUser)
{
    si7021_write_register(SI7021_CMD_WRITE_USER, ubUser);
}
uint8_t si7021_read_user()
{
    return si7021_read_register(SI7021_CMD_READ_USER);
}

uint8_t si7021_read_firmware_version()
{
    uint8_t pubBuffer[2];

    pubBuffer[0] = SI7021_CMD_READ_FW_VERSION1;
    pubBuffer[1] = SI7021_CMD_READ_FW_VERSION2;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
		i2c0_write(SI7021_I2C_ADDR, pubBuffer, 2, I2C_RESTART);
		return i2c0_read_byte(SI7021_I2C_ADDR, I2C_STOP);
    }
}
uint64_t si7021_read_unique_id()
{
    uint8_t pubSNA[8];
    uint8_t pubBuffer[2];

    pubBuffer[0] = SI7021_CMD_READ_UID_A1;
    pubBuffer[1] = SI7021_CMD_READ_UID_A2;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
		i2c0_write(SI7021_I2C_ADDR, pubBuffer, 2, I2C_RESTART);
		i2c0_read(SI7021_I2C_ADDR, pubSNA, 8, I2C_STOP);
    }

    uint8_t pubSNB[6];

    pubBuffer[0] = SI7021_CMD_READ_UID_B1;
    pubBuffer[1] = SI7021_CMD_READ_UID_B2;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
		i2c0_write(SI7021_I2C_ADDR, pubBuffer, 2, I2C_RESTART);
		i2c0_read(SI7021_I2C_ADDR, pubSNB, 6, I2C_STOP);
    }

    return ((uint64_t)pubSNA[0] << 56) | ((uint64_t)pubSNA[2] << 48) | ((uint64_t)pubSNA[4] << 40) | ((uint64_t)pubSNA[6] << 32) | ((uint64_t)pubSNB[0] << 24) | ((uint64_t)pubSNB[1] << 16) | ((uint64_t)pubSNB[3] << 8) | ((uint64_t)pubSNB[4] << 0);
}