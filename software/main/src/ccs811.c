#include "ccs811.h"

static uint8_t ccs811_read_register(uint8_t ubRegister)
{
	uint8_t ubValue;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
        CCS811_WAKE();

		i2c0_write_byte(CCS811_I2C_ADDR, ubRegister, I2C_RESTART);
		ubValue = i2c0_read_byte(CCS811_I2C_ADDR, I2C_STOP);

        CCS811_SLEEP();
	}

	return ubValue;
}
static void ccs811_write_register(uint8_t ubRegister, uint8_t ubValue)
{
	uint8_t pubBuffer[2] = {ubRegister, ubValue};

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
        CCS811_WAKE();

		i2c0_write(CCS811_I2C_ADDR, pubBuffer, 2, I2C_STOP);

        CCS811_SLEEP();
	}
}
static void ccs811_rmw_register(uint8_t ubRegister, uint8_t ubMask, uint8_t ubValue)
{
	ccs811_write_register(ubRegister, (ccs811_read_register(ubRegister) & ubMask) | ubValue);
}

uint8_t ccs811_init()
{
	delay_ms(CCS811_T_START_PON);

    ccs811_hardware_reset();

    CCS811_WAKE();

	if(!i2c0_write(CCS811_I2C_ADDR, NULL, 0, I2C_STOP)) // Check ACK from the expected address
		return 0;

    CCS811_SLEEP();

    delay_ms(10);

	if(ccs811_read_hw_id() != 0x81)
        return 0;

    return 1;
}

void ccs811_app_erase()
{
    uint8_t pubBuffer[5] = {CCS811_REG_SW_RESET, 0xE7, 0xA7, 0xE6, 0x09};

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write(CCS811_I2C_ADDR, pubBuffer, 5, I2C_STOP);

        CCS811_SLEEP();
    }

	delay_ms(CCS811_T_APP_ERASE);
}
void ccs811_app_send_data(uint8_t *pubSrc)
{
    uint8_t pubBuffer[9] = {CCS811_REG_APP_DATA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    for(uint8_t i = 1; i < 9; i++)
        pubBuffer[i] = *pubSrc++;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write(CCS811_I2C_ADDR, pubBuffer, 9, I2C_STOP);

        CCS811_SLEEP();
    }

	delay_ms(CCS811_T_APP_DATA);
}
uint8_t ccs811_app_verify()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write_byte(CCS811_I2C_ADDR, CCS811_REG_APP_VERIFY, I2C_STOP);

        CCS811_SLEEP();
    }

    delay_ms(CCS811_T_APP_VERIFY);

    while(!(ccs811_read_status() & CCS811_APP_VERIFIED))
        delay_ms(10);

    return !!(ccs811_read_status() & CCS811_APP_VALID);
}
void ccs811_app_start()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write_byte(CCS811_I2C_ADDR, CCS811_REG_APP_START, I2C_STOP);

        CCS811_SLEEP();
    }

    delay_ms(CCS811_T_APP_START);
}

void ccs811_software_reset()
{
    uint8_t pubBuffer[5] = {CCS811_REG_SW_RESET, 0x11, 0xE5, 0x72, 0x8A};

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write(CCS811_I2C_ADDR, pubBuffer, 5, I2C_STOP);

        CCS811_SLEEP();
    }

	delay_ms(CCS811_T_START_RESET);
}
void ccs811_hardware_reset()
{
    CCS811_RESET();
    delay_ms(CCS811_T_RESET);
    CCS811_UNRESET();
	delay_ms(CCS811_T_START_RESET);
}

uint16_t ccs811_read_etvoc()
{
    uint8_t pubBuffer[4];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write_byte(CCS811_I2C_ADDR, CCS811_REG_ALG_RESULT_DATA, I2C_RESTART);
		i2c0_read(CCS811_I2C_ADDR, pubBuffer, 4, I2C_STOP);

        CCS811_SLEEP();
    }

    return ((uint16_t)pubBuffer[2] << 8) | ((uint16_t)pubBuffer[3] << 0);
}
uint16_t ccs811_read_eco2()
{
    uint8_t pubBuffer[2];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write_byte(CCS811_I2C_ADDR, CCS811_REG_ALG_RESULT_DATA, I2C_RESTART);
		i2c0_read(CCS811_I2C_ADDR, pubBuffer, 2, I2C_STOP);

        CCS811_SLEEP();
    }

    return ((uint16_t)pubBuffer[0] << 8) | ((uint16_t)pubBuffer[1] << 0);
}

uint8_t ccs811_read_status()
{
    return ccs811_read_register(CCS811_REG_STATUS);
}
uint8_t ccs811_read_internal_status()
{
    return ccs811_read_register(CCS811_REG_INTERNAL_STATUS);
}
uint8_t ccs811_read_error()
{
    return ccs811_read_register(CCS811_REG_ERROR_ID);
}
uint8_t ccs811_read_hw_id()
{
    return ccs811_read_register(CCS811_REG_HW_ID);
}
uint8_t ccs811_read_hw_version()
{
    return ccs811_read_register(CCS811_REG_HW_VERSION);
}
uint16_t ccs811_read_boot_version()
{
    uint8_t pubBuffer[2];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write_byte(CCS811_I2C_ADDR, CCS811_REG_BL_FW_VERSION, I2C_RESTART);
		i2c0_read(CCS811_I2C_ADDR, pubBuffer, 2, I2C_STOP);

        CCS811_SLEEP();
    }

    return ((uint16_t)pubBuffer[0] << 8) | ((uint16_t)pubBuffer[1] << 0);
}
uint16_t ccs811_read_app_version()
{
    uint8_t pubBuffer[2];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write_byte(CCS811_I2C_ADDR, CCS811_REG_APP_FW_VERSION, I2C_RESTART);
		i2c0_read(CCS811_I2C_ADDR, pubBuffer, 2, I2C_STOP);

        CCS811_SLEEP();
    }

    return ((uint16_t)pubBuffer[0] << 8) | ((uint16_t)pubBuffer[1] << 0);
}

void ccs811_write_meas_mode(uint8_t ubMode)
{
    ccs811_write_register(CCS811_REG_MEAS_MODE, ubMode);
}
uint8_t ccs811_read_meas_mode()
{
    return ccs811_read_register(CCS811_REG_MEAS_MODE);
}

void ccs811_write_env_data(float fTemp, float fHumid)
{
    uint8_t pubBuffer[5] = {CCS811_REG_ENV_DATA, 0x00, 0x00, 0x00, 0x00};

    uint16_t usTemp = (fTemp + 25.f) * 512.f;
    uint16_t usHumid = fHumid * 512.f;

    pubBuffer[1] = usHumid >> 8;
    pubBuffer[2] = usHumid & 0xFF;
    pubBuffer[3] = usTemp >> 8;
    pubBuffer[4] = usTemp & 0xFF;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write(CCS811_I2C_ADDR, pubBuffer, 5, I2C_STOP);

        CCS811_SLEEP();
    }
}

void ccs811_write_tresh(uint16_t usLowToMed, uint16_t usMedToHigh)
{
    uint8_t pubBuffer[5] = {CCS811_REG_THRESHOLDS, 0x00, 0x00, 0x00, 0x00};

    pubBuffer[1] = usLowToMed >> 8;
    pubBuffer[2] = usLowToMed & 0xFF;
    pubBuffer[3] = usMedToHigh >> 8;
    pubBuffer[4] = usMedToHigh & 0xFF;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write(CCS811_I2C_ADDR, pubBuffer, 5, I2C_STOP);

        CCS811_SLEEP();
    }
}

void ccs811_write_baseline(uint16_t usBaseline)
{
    uint8_t pubBuffer[5] = {CCS811_REG_BASELINE, 0x00, 0x00};

    pubBuffer[1] = usBaseline >> 8;
    pubBuffer[2] = usBaseline & 0xFF;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write(CCS811_I2C_ADDR, pubBuffer, 3, I2C_STOP);

        CCS811_SLEEP();
    }
}
uint16_t ccs811_read_baseline()
{
    uint8_t pubBuffer[2];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        CCS811_WAKE();

		i2c0_write_byte(CCS811_I2C_ADDR, CCS811_REG_BASELINE, I2C_RESTART);
		i2c0_read(CCS811_I2C_ADDR, pubBuffer, 2, I2C_STOP);

        CCS811_SLEEP();
    }

    return ((uint16_t)pubBuffer[0] << 8) | ((uint16_t)pubBuffer[1] << 0);
}