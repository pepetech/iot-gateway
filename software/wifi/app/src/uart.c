#include "uart.h"

void uart0_init(uint32_t baud)
{
	REG(UART0_CLKDIV) = VAL2FIELD(UART_CLKDIV, UART_CLK_FREQ / baud); // Set baud
    REG(UART0_CONF0) = VAL2FIELD(UART_BIT_NUM, 0x3); // 8 bits, no parity, 1 stop bit
	REG(UART0_CONF0) |= UART_TXFIFO_RST | UART_RXFIFO_RST; // Clear TX & RX buffers
    REG(UART0_CONF0) &= ~(UART_TXFIFO_RST | UART_RXFIFO_RST);
	REG(UART0_CONF1) = VAL2FIELD(UART_RXFIFO_FULL_THRHD, 128) | VAL2FIELD(UART_TXFIFO_EMPTY_THRHD, 0); // RX FIFO full on max, TX FIFO empty on min
    REG(UART0_INT_CLR) = 0xFFFFFFFF; // Clear all interrupt flags
}
void uart0_write_byte(uint8_t data)
{
	while(1)
    {
        uint32_t fifo_cnt = REG(UART0_STATUS) & (UART_TXFIFO_CNT_M << UART_TXFIFO_CNT_S);

        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT_M) < 128)
            break;
    }

    REG(UART0_FIFO) = data;
}
uint8_t uart0_read_byte()
{
	if(!uart0_available())
		return 0;

	return REG(UART0_FIFO);
}

void uart1_init(uint32_t baud)
{
	REG(UART1_CLKDIV) = VAL2FIELD(UART_CLKDIV, UART_CLK_FREQ / baud); // Set baud
    REG(UART1_CONF0) = VAL2FIELD(UART_BIT_NUM, 0x3); // 8 bits, no parity, 1 stop bit
	REG(UART1_CONF0) |= UART_TXFIFO_RST | UART_RXFIFO_RST; // Clear TX & RX buffers
    REG(UART1_CONF0) &= ~(UART_TXFIFO_RST | UART_RXFIFO_RST);
	REG(UART1_CONF1) = VAL2FIELD(UART_RXFIFO_FULL_THRHD, 128) | VAL2FIELD(UART_TXFIFO_EMPTY_THRHD, 0); // RX FIFO full on max, TX FIFO empty on min
    REG(UART1_INT_CLR) = 0xFFFFFFFF; // Clear all interrupt flags
}
void uart1_write_byte(uint8_t data)
{
	while(1)
    {
        uint32_t fifo_cnt = REG(UART1_STATUS) & (UART_TXFIFO_CNT_M << UART_TXFIFO_CNT_S);

        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT_M) < 128)
            break;
    }

    REG(UART1_FIFO) = data;
}
uint8_t uart1_read_byte()
{
	if(!uart1_available())
		return 0;

	return REG(UART1_FIFO);
}
