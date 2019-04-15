#include "as5048a.h"

/**
 * Initialiser
 */
void as5048a_init()
{

}

/**
 * Utility function used to calculate even parity of word
 */
uint8_t as5048a_calc_even_parity(uint16_t usValue)
{
	uint8_t ubCnt = 0;

	for (uint8_t ubI = 0; ubI < 16; ubI++)
	{
		if(usValue & 0x1)
		{
			ubCnt++;
		}
		usValue >>= 1;
	}
	return ubCnt & 0x1;
}

uint16_t as5048a_get_angle()
{
    return as5048a_read(AS5048A_ANGLE);
}


/*
 * Get and clear the error register by reading it
 */
uint16_t as5048a_get_errors()
{
	return as5048a_read(AS5048A_CLEAR_ERROR_FLAG);
}


/*
 * Read a register from the sensor
 * Takes the address of the register as a 16 bit word
 * Returns the value of the register
 */
uint16_t as5048a_read(uint16_t usRegisterAddress)
{
	uint16_t usCommand = 0b0100000000000000; // PAR=0 R/W=R
	usCommand |= usRegisterAddress;

	//Add a parity bit on the the MSB
	usCommand |= ((uint16_t)as5048a_calc_even_parity(usCommand) << 15);

	//Send the command
	AS5048A_SELECT();
	usart0_spi_transfer_byte(usCommand & 0xFF);
	usart0_spi_transfer_byte((usCommand >> 8) & 0xFF);
	AS5048A_UNSELECT();

    delay_ms(1);

    uint16_t usResponse = 0;

	//Now read the response
	AS5048A_SELECT();
	usResponse = ((uint16_t)usart0_spi_transfer_byte(0x00) << 8);
	usResponse |= (uint16_t)usart0_spi_transfer_byte(0x00);
	AS5048A_UNSELECT();

    delay_ms(1);
    
	//Check if the error bit is set
	if(usResponse & 0x4000) 
    {
        DBGPRINTLN_CTX("error bit set");
	}

	//Return the data, stripping the parity and error bits
	return usResponse & 0x3FFF;
}


/*
 * Write to a register
 * Takes the 16-bit  address of the target register and the 16 bit word of data
 * to be written to that register
 * Returns the value of the register after the write has been performed. This
 * is read back from the sensor to ensure a sucessful write.
 */
uint16_t as5048a_write(uint16_t usRegisterAddress, uint16_t usData) 
{
	uint16_t usCommand = 0b0000000000000000; // PAR=0 R/W=W
	usCommand |= usRegisterAddress;

	//Add a parity bit on the the MSB
	usCommand |= ((uint16_t)as5048a_calc_even_parity(usCommand) << 15);

	//Send the command
	AS5048A_SELECT();
	usart0_spi_transfer_byte(usCommand & 0xFF);
	usart0_spi_transfer_byte((usCommand >> 8) & 0xFF);
	AS5048A_UNSELECT();
	
    delay_ms(1);

	uint16_t usDataToSend = 0b0000000000000000;
	usDataToSend |= usData;

	//Craft another packet including the data and parity
	usDataToSend |= ((uint16_t)as5048a_calc_even_parity(usDataToSend) << 15);

	//Send the data
	AS5048A_SELECT();
	usart0_spi_transfer_byte(usDataToSend & 0xFF);
	usart0_spi_transfer_byte((usDataToSend >> 8) & 0xFF);
	AS5048A_UNSELECT();

    delay_ms(1);
	
    uint16_t usReturnData = 0;

	//Send a NOP to get the new data in the register
	AS5048A_SELECT();
	usReturnData = ((uint16_t)usart0_spi_transfer_byte(0x00) << 8);
	usReturnData = (uint16_t)usart0_spi_transfer_byte(0x00);
	AS5048A_UNSELECT();

    delay_ms(1);

	//Return the data, stripping the parity and error bits
	return usReturnData & 0x3FFF;
}