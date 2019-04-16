#ifndef __UART_H__
#define __UART_H__

#include <c_types.h>
#include "registers.h"
#include "ets_func.h"

void ICACHE_FLASH_ATTR uart0_init(uint32_t baud);
void ICACHE_FLASH_ATTR uart0_write_byte(uint8_t data);
uint8_t ICACHE_FLASH_ATTR uart0_read_byte();
static inline uint8_t uart0_available()
{
	return REG(UART0_STATUS) & (UART_RXFIFO_CNT_M << UART_RXFIFO_CNT_S);
}
static inline void uart0_flush()
{
	REG(UART0_CONF0) |= UART_RXFIFO_RST; // Clear RX buffer
    REG(UART0_CONF0) &= ~UART_RXFIFO_RST;
}
static inline void uart0_write(uint8_t *src, uint32_t size)
{
	while(size--)
		uart0_write_byte(*src++);
}
static inline void uart0_read(uint8_t *dst, uint32_t size)
{
	while(size--)
		*dst++ = uart0_read_byte();
}

void ICACHE_FLASH_ATTR uart1_init(uint32_t baud);
void ICACHE_FLASH_ATTR uart1_write_byte(uint8_t data);
uint8_t ICACHE_FLASH_ATTR uart1_read_byte();
static inline uint8_t uart1_available()
{
	return REG(UART1_STATUS) & (UART_RXFIFO_CNT_M << UART_RXFIFO_CNT_S);
}
static inline void uart1_flush()
{
	REG(UART1_CONF0) |= UART_RXFIFO_RST; // Clear RX buffer
    REG(UART1_CONF0) &= ~UART_RXFIFO_RST;
}
static inline void uart1_write(uint8_t *src, uint32_t size)
{
	while(size--)
		uart1_write_byte(*src++);
}
static inline void uart1_read(uint8_t *dst, uint32_t size)
{
	while(size--)
		*dst++ = uart1_read_byte();
}

#endif
