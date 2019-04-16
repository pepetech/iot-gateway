#ifndef __USART_H__
#define __USART_H__

#include <em_device.h>
#include <stdlib.h>
#include <string.h>
#include "nvic.h"
#include "utils.h"
#include "atomic.h"
#include "cmu.h"
#include "ldma.h"

#define USART_LOCATION_DISABLED -1

#define USART_SPI_LSB_FIRST 0
#define USART_SPI_MSB_FIRST 1


#define USART0_MODE_SPI                 	// Define for SPI, comment out for UART
#define USART0_DMA_CHANNEL			0   	// Only relevant when in UART mode
#define USART0_DMA_RX_BUFFER_SIZE	128		// Only relevant when in UART mode
#define USART0_FIFO_SIZE			256     // Only relevant when in UART mode

#if defined(USART0_MODE_SPI)
void usart0_init(uint32_t ulBaud, uint8_t ubMode, uint8_t ubBitMode, int8_t bMISOLocation, int8_t bMOSILocation, uint8_t ubCLKLocation);
uint8_t usart0_spi_transfer_byte(const uint8_t ubData);
void usart0_spi_transfer(const uint8_t* pubSrc, uint32_t ulSize, uint8_t* pubDst);
static inline void usart0_spi_write(const uint8_t* pubSrc, uint32_t ulSize)
{
    usart0_spi_transfer(pubSrc, ulSize, NULL);
}
static inline void usart0_spi_read(uint8_t* pubDst, uint32_t ulSize)
{
    usart0_spi_transfer(NULL, ulSize, pubDst);
}
#else   // USART0_MODE
void usart0_init(uint32_t ulBaud, uint32_t ulFrameSettings, int8_t bRXLocation, int8_t bTXLocation, int8_t bCTSLocation, int8_t bRTSLocation);
void usart0_write_byte(const uint8_t ubData);
uint8_t usart0_read_byte();
uint32_t usart0_available();
void usart0_flush();
static inline void usart0_write(const uint8_t *pubSrc, uint32_t ulSize)
{
	while(ulSize--)
		usart0_write_byte(*pubSrc++);
}
static inline void usart0_read(uint8_t *pubDst, uint32_t ulSize)
{
	while(ulSize--)
		*pubDst++ = usart0_read_byte();
}
#endif  // USART0_MODE

// - - - - - - - - - - - - - - - TEMPORARY - - - - - - - - - - - - - -
void usart1_init(uint32_t ulBaud, uint8_t ubMode, uint8_t ubBitMode, int8_t bMISOLocation, int8_t bMOSILocation, uint8_t ubCLKLocation);
uint8_t usart1_spi_transfer_byte(const uint8_t ubData);
void usart1_spi_transfer(const uint8_t* pubSrc, uint32_t ulSize, uint8_t* pubDst);
static inline void usart1_spi_write(const uint8_t* pubSrc, uint32_t ulSize)
{
    usart1_spi_transfer(pubSrc, ulSize, NULL);
}
static inline void usart1_spi_read(uint8_t* pubDst, uint32_t ulSize)
{
    usart1_spi_transfer(NULL, ulSize, pubDst);
}
// - - - - - - - - - - - - - - - TEMPORARY - - - - - - - - - - - - - -

#endif  // __USART_H__