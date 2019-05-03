
#include "ft6x36.h"

#define FT6X36_TOUCH_FIFO_SIZE 32

static volatile struct {
    uint8_t ubEvent;
    uint16_t usX;
    uint16_t usY;
} pxTouchFIFO[FT6X36_TOUCH_FIFO_SIZE];
static volatile uint8_t ubTouchFIFORd = 0, ubTouchFIFOWr = 0;

static ft6x36_callback_fn_t pfTouchCallback = NULL;

static uint8_t ft6x36_read_register(uint8_t ubRegister)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c1_write_byte(FT6X06_I2C_ADDR, ubRegister, I2C_RESTART);
		return i2c1_read_byte(FT6X06_I2C_ADDR, I2C_STOP);
	}
}
static void ft6x36_write_register(uint8_t ubRegister, uint8_t ubValue)
{
	uint8_t pubBuffer[2];

    pubBuffer[0] = ubRegister;
    pubBuffer[1] = ubValue;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c1_write(FT6X06_I2C_ADDR, pubBuffer, 2, I2C_STOP);
	}
}
static void ft6x36_rmw_register(uint8_t ubRegister, uint8_t ubMask, uint8_t ubValue)
{
	ft6x36_write_register(ubRegister, (ft6x36_read_register(ubRegister) & ubMask) | ubValue);
}

uint8_t ft6x36_init()
{
	if(!i2c1_write(FT6X06_I2C_ADDR, NULL, 0, I2C_STOP)) // Check ACK from the expected address
		return 0;

    ft6x36_write_register(FT6X06_PERIODACTIVE, 0x01);
    ft6x36_write_register(FT6X06_TH_GROUP, 127);

    return 1;
}
void ft6x36_isr()
{
    uint8_t ubBuf[4];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		i2c1_write_byte(FT6X06_I2C_ADDR, 0x03, I2C_RESTART);
        i2c1_read(FT6X06_I2C_ADDR, ubBuf, 4, I2C_STOP);
	}

    pxTouchFIFO[ubTouchFIFOWr].ubEvent = (ubBuf[0] >> 6) & 0x03;
    pxTouchFIFO[ubTouchFIFOWr].usX = ((uint16_t)(ubBuf[0] & 0x03) << 8) | (uint16_t)ubBuf[1];
    pxTouchFIFO[ubTouchFIFOWr].usY = ((uint16_t)(ubBuf[2] & 0x03) << 8) | (uint16_t)ubBuf[3];

    ubTouchFIFOWr++;
    if(ubTouchFIFOWr >= FT6X36_TOUCH_FIFO_SIZE)
        ubTouchFIFOWr = 0;
}
void ft6x36_tick()
{
    while((FT6X36_TOUCH_FIFO_SIZE + ubTouchFIFOWr - ubTouchFIFORd) % FT6X36_TOUCH_FIFO_SIZE)
    {
        if(pfTouchCallback)
			pfTouchCallback(pxTouchFIFO[ubTouchFIFORd].ubEvent, pxTouchFIFO[ubTouchFIFORd].usX, pxTouchFIFO[ubTouchFIFORd].usY);

        ubTouchFIFORd++;
        if(ubTouchFIFORd >= FT6X36_TOUCH_FIFO_SIZE)
            ubTouchFIFORd = 0;
    }
}

void ft6x36_set_callback(ft6x36_callback_fn_t pfFunc)
{
    pfTouchCallback = pfFunc;
}

uint8_t ft6x36_get_vendor_id()
{
    return ft6x36_read_register(FT6X06_FOCALTECH_ID);
}
uint8_t ft6x36_get_chip_id()
{
    return ft6x36_read_register(FT6X06_CIPHER);
}
uint8_t ft6x36_get_firmware_version()
{
    return ft6x36_read_register(FT6X06_FIRMID);
}