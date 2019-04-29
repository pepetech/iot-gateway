#include "bmp280.h"

static uint16_t DIG_T1;
static int16_t DIG_T2;
static int16_t DIG_T3;

static uint16_t DIG_P1;
static int16_t DIG_P2;
static int16_t DIG_P3;
static int16_t DIG_P4;
static int16_t DIG_P5;
static int16_t DIG_P6;
static int16_t DIG_P7;
static int16_t DIG_P8;
static int16_t DIG_P9;

static int32_t T_FINE;

static uint8_t bmp280_read_register(uint8_t ubRegister)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c0_write_byte(BMP280_I2C_ADDR, ubRegister, I2C_RESTART);
		return i2c0_read_byte(BMP280_I2C_ADDR, I2C_STOP);
	}
}
static void bmp280_write_register(uint8_t ubRegister, uint8_t ubValue)
{
	uint8_t pubBuffer[2];

    pubBuffer[0] = ubRegister;
    pubBuffer[1] = ubValue;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c0_write(BMP280_I2C_ADDR, pubBuffer, 2, I2C_STOP);
	}
}
static void bmp280_rmw_register(uint8_t ubRegister, uint8_t ubMask, uint8_t ubValue)
{
	bmp280_write_register(ubRegister, (bmp280_read_register(ubRegister) & ubMask) | ubValue);
}

uint8_t bmp280_init()
{
	delay_ms(BMP280_T_START_PON);

	if(!i2c0_write(BMP280_I2C_ADDR, NULL, 0, I2C_STOP)) // Check ACK from the expected address
		return 0;

	if(bmp280_read_id() != 0x58)
        return 0;

    bmp280_software_reset();

    while(bmp280_read_status() & BMP280_READING_CALIB)
        delay_ms(BMP280_T_CALIB_READ);

    uint8_t pubBuffer[24];

    pubBuffer[0] = BMP280_REG_DIG_T1_H;
    pubBuffer[1] = BMP280_REG_DIG_T1_L;
    pubBuffer[2] = BMP280_REG_DIG_T2_H;
    pubBuffer[3] = BMP280_REG_DIG_T2_L;
    pubBuffer[4] = BMP280_REG_DIG_T3_H;
    pubBuffer[5] = BMP280_REG_DIG_T3_L;

    pubBuffer[6] = BMP280_REG_DIG_P1_H;
    pubBuffer[7] = BMP280_REG_DIG_P1_L;
    pubBuffer[8] = BMP280_REG_DIG_P2_H;
    pubBuffer[9] = BMP280_REG_DIG_P2_L;
    pubBuffer[10] = BMP280_REG_DIG_P3_H;
    pubBuffer[11] = BMP280_REG_DIG_P3_L;
    pubBuffer[12] = BMP280_REG_DIG_P4_H;
    pubBuffer[13] = BMP280_REG_DIG_P4_L;
    pubBuffer[14] = BMP280_REG_DIG_P5_H;
    pubBuffer[15] = BMP280_REG_DIG_P5_L;
    pubBuffer[16] = BMP280_REG_DIG_P6_H;
    pubBuffer[17] = BMP280_REG_DIG_P6_L;
    pubBuffer[18] = BMP280_REG_DIG_P7_H;
    pubBuffer[19] = BMP280_REG_DIG_P7_L;
    pubBuffer[20] = BMP280_REG_DIG_P8_H;
    pubBuffer[21] = BMP280_REG_DIG_P8_L;
    pubBuffer[22] = BMP280_REG_DIG_P9_H;
    pubBuffer[23] = BMP280_REG_DIG_P9_L;

    for(uint8_t i = 0; i < 24; i++)
        pubBuffer[i] = bmp280_read_register(pubBuffer[i]);

    DIG_T1 = (pubBuffer[0] << 8) | pubBuffer[1];
    DIG_T2 = (int16_t)((pubBuffer[2] << 8) | pubBuffer[3]);
    DIG_T3 = (int16_t)((pubBuffer[4] << 8) | pubBuffer[5]);

    DIG_P1 = (pubBuffer[6] << 8) | pubBuffer[7];
    DIG_P2 = (int16_t)((pubBuffer[8] << 8) | pubBuffer[9]);
    DIG_P3 = (int16_t)((pubBuffer[10] << 8) | pubBuffer[11]);
    DIG_P4 = (int16_t)((pubBuffer[12] << 8) | pubBuffer[13]);
    DIG_P5 = (int16_t)((pubBuffer[14] << 8) | pubBuffer[15]);
    DIG_P6 = (int16_t)((pubBuffer[16] << 8) | pubBuffer[17]);
    DIG_P7 = (int16_t)((pubBuffer[18] << 8) | pubBuffer[19]);
    DIG_P8 = (int16_t)((pubBuffer[20] << 8) | pubBuffer[21]);
    DIG_P9 = (int16_t)((pubBuffer[22] << 8) | pubBuffer[23]);

    return 1;
}

void bmp280_software_reset()
{
	bmp280_write_register(BMP280_REG_SW_RESET, 0xB6);
	delay_ms(BMP280_T_START_RESET);
}

float bmp280_read_temperature()
{
	uint8_t pubBuffer[3];

    pubBuffer[0] = BMP280_REG_TEMP_H;
    pubBuffer[1] = BMP280_REG_TEMP_L;
    pubBuffer[2] = BMP280_REG_TEMP_XL;

	for(uint8_t i = 0; i < 3; i++)
		pubBuffer[i] = bmp280_read_register(pubBuffer[i]);

	int32_t var1, var2, T, adc_T;

	adc_T = pubBuffer[0];
	adc_T <<= 8;
	adc_T |= pubBuffer[1];
	adc_T <<= 4;
	adc_T |= pubBuffer[2] >> 4;

	if(adc_T == 0x80000)
		return 0.f;

	var1 = ((((adc_T >> 3) - ((int32_t)DIG_T1 << 1))) * ((int32_t)DIG_T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((int32_t)DIG_T1)) * ((adc_T >> 4) - ((int32_t)DIG_T1))) >> 12) * ((int32_t)DIG_T3)) >> 14;
	T_FINE = var1 + var2;
	T = (T_FINE * 5 + 128) >> 8;

	return (float)T / 100.f;
}
float bmp280_read_pressure()
{
	uint8_t pubBuffer[3];

    pubBuffer[0] = BMP280_REG_PRESSURE_H;
    pubBuffer[1] = BMP280_REG_PRESSURE_L;
    pubBuffer[2] = BMP280_REG_PRESSURE_XL;

	for(uint8_t i = 0; i < 3; i++)
		pubBuffer[i] = bmp280_read_register(pubBuffer[i]);

	int32_t adc_P;
	int64_t var1, var2, p;

	adc_P = pubBuffer[0];
	adc_P <<= 8;
	adc_P |= pubBuffer[1];
	adc_P <<= 4;
	adc_P |= pubBuffer[2] >> 4;

	if(adc_P == 0x80000)
		return 0.f;

	bmp280_read_temperature();

	var1 = ((int64_t)T_FINE) - 128000;
	var2 = var1 * var1 * (int64_t)DIG_P6;
	var2 = var2 + ((var1*(int64_t)DIG_P5)<<17);
	var2 = var2 + (((int64_t)DIG_P4)<<35);
	var1 = ((var1 * var1 * (int64_t)DIG_P3)>>8) + ((var1 * (int64_t)DIG_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)DIG_P1)>>33;

	if (var1 == 0)
	{
		return 0.f; // avoid exception caused by division by zero
	}

	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((int64_t)DIG_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)DIG_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)DIG_P7)<<4);

	return (float)p / 25600.f;
}

void bmp280_take_forced_meas()
{
	uint16_t usCurrentControl = bmp280_read_control();

	if(!(usCurrentControl & BMP280_MODE_NORMAL))
	{
		bmp280_write_control(usCurrentControl | BMP280_MODE_FORCED);

		while(bmp280_read_status() & BMP280_MEASURING)
			delay_ms(1);
	}
}

uint8_t bmp280_read_status()
{
	return bmp280_read_register(BMP280_REG_STATUS);
}
uint8_t bmp280_read_id()
{
	return bmp280_read_register(BMP280_REG_HW_ID);
}
uint8_t bmp280_read_version()
{
	return bmp280_read_register(BMP280_REG_VERSION);
}

void bmp280_write_control(uint8_t ubControl)
{
	bmp280_write_register(BMP280_REG_CONTROL, ubControl);
}
uint8_t bmp280_read_control()
{
	return bmp280_read_register(BMP280_REG_CONTROL);
}

void bmp280_write_config(uint8_t ubConfig)
{
	bmp280_write_register(BMP280_REG_CONFIG, ubConfig);
}
uint8_t bmp280_read_config()
{
	return bmp280_read_register(BMP280_REG_CONFIG);
}
