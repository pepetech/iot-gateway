#ifndef __AS5048_H_
#define __AS5048_H_

#include <em_device.h>
#include "usart.h"
#include "systick.h"
#include "gpio.h"
#include "debug_macros.h"

#define AS5048A_CLEAR_ERROR_FLAG              0x0001
#define AS5048A_PROGRAMMING_CONTROL           0x0003
#define AS5048A_OTP_REGISTER_ZERO_POS_HIGH    0x0016
#define AS5048A_OTP_REGISTER_ZERO_POS_LOW     0x0017
#define AS5048A_DIAG_AGC                      0x3FFD
#define AS5048A_MAGNITUDE                     0x3FFE
#define AS5048A_ANGLE                         0x3FFF

/**
 * Initialiser
 */
void as5048a_init();

/**
 * Utility function used to calculate even parity of word
 */
uint8_t as5048a_calc_even_parity(uint16_t usValue);

uint16_t as5048a_get_angle();

/*
 * Get and clear the error register by reading it
 */
uint16_t as5048a_get_errors();


/*
 * Read a register from the sensor
 * Takes the address of the register as a 16 bit word
 * Returns the value of the register
 */
uint16_t as5048a_read(uint16_t usRegisterAddress);

/*
 * Write to a register
 * Takes the 16-bit  address of the target register and the 16 bit word of data
 * to be written to that register
 * Returns the value of the register after the write has been performed. This
 * is read back from the sensor to ensure a sucessful write.
 */
uint16_t as5048a_write(uint16_t usRegisterAddress, uint16_t usData);

#endif // __AS5048_H_