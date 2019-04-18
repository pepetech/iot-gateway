#include "usart.h"

#if defined(USART0_MODE_SPI)
void usart0_init(uint32_t ulBaud, uint8_t ubMode, uint8_t ubBitMode, int8_t bMISOLocation, int8_t bMOSILocation, uint8_t ubCLKLocation)
{
    if(bMISOLocation > -1 && bMISOLocation > AFCHANLOC_MAX)
        return;

    if(bMOSILocation > -1 && bMOSILocation > AFCHANLOC_MAX)
        return;

    if(ubCLKLocation > AFCHANLOC_MAX)
        return;

    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_USART0;

    USART0->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX | UART_CMD_TXTRIDIS | USART_CMD_RXBLOCKDIS | USART_CMD_TXDIS | USART_CMD_RXDIS;

    USART0->CTRL = USART_CTRL_TXBIL_EMPTY | USART_CTRL_CSMA_NOACTION | (ubBitMode == USART_SPI_MSB_FIRST ? USART_CTRL_MSBF : 0) | (ubMode & 1 ? USART_CTRL_CLKPHA_SAMPLETRAILING : USART_CTRL_CLKPHA_SAMPLELEADING) | (ubMode & 2 ? USART_CTRL_CLKPOL_IDLEHIGH : USART_CTRL_CLKPOL_IDLELOW) | USART_CTRL_SYNC;
    USART0->FRAME = USART_FRAME_DATABITS_EIGHT;
    USART0->CLKDIV = (uint32_t)((((float)HFPER_CLOCK_FREQ / (2.f * ulBaud)) - 1.f) * 256.f);

    USART0->ROUTEPEN = (bMISOLocation >= 0 ? USART_ROUTEPEN_RXPEN : 0) | (bMOSILocation >= 0 ? USART_ROUTEPEN_TXPEN : 0) | USART_ROUTEPEN_CLKPEN;
    USART0->ROUTELOC0 = ((uint32_t)(bMISOLocation >= 0 ? bMISOLocation : 0) << _USART_ROUTELOC0_RXLOC_SHIFT) | ((uint32_t)(bMOSILocation >= 0 ? bMOSILocation : 0) << _USART_ROUTELOC0_TXLOC_SHIFT) | ((uint32_t)ubCLKLocation << _USART_ROUTELOC0_CLKLOC_SHIFT);

    USART0->CMD = USART_CMD_MASTEREN | USART_CMD_TXEN | USART_CMD_RXEN;
}
uint8_t usart0_spi_transfer_byte(const uint8_t ubData)
{
    while(!(USART0->STATUS & USART_STATUS_TXBL));

    USART0->TXDATA = ubData;

    while(!(USART0->STATUS & USART_STATUS_TXC));

    return USART0->RXDATA;
}
void usart0_spi_transfer(const uint8_t* pubSrc, uint32_t ulSize, uint8_t* pubDst)
{
	if(pubSrc)
	{
		while(ulSize--)
		{
			if(pubDst)
				*(pubDst++) = usart0_spi_transfer_byte(*(pubSrc++));
			else
				usart0_spi_transfer_byte(*(pubSrc++));
		}
	}
	else if(pubDst)
	{
		while(ulSize--)
			*(pubDst++) = usart0_spi_transfer_byte(0x00);
	}
}
#else   // USART0_MODE
static volatile uint8_t *pubUSART0DMABuffer = NULL;
static volatile uint8_t *pubUSART0FIFO = NULL;
static volatile uint16_t usUSART0FIFOWritePos, usUART2FIFOReadPos;
static ldma_descriptor_t __attribute__ ((aligned (4))) pUSART0DMADescriptor[2];

void _usart0_rx_isr()
{
 /*   if(USART1->SR & USART_SR_IDLE)
	{
        REG_DISCARD(&USART1->SR); // Read SR & DR to clear interrupt flag
	    REG_DISCARD(&USART1->DR);

        uint32_t ulSize = (UART1_DMA_RX_BUFFER_SIZE >> 1) - ((DMA1_Channel5->CNDTR - 1) % (UART1_DMA_RX_BUFFER_SIZE >> 1)) - 1;

        if(ulSize)
        {
            DMA1_Channel5->CCR &= ~DMA_CCR5_EN; // Disable DMA channel

            volatile uint8_t *pubDMABufferReadPos = (DMA1_Channel5->CNDTR > (UART1_DMA_RX_BUFFER_SIZE >> 1)) ? pubUART1DMABuffer : pubUART1DMABuffer + (UART1_DMA_RX_BUFFER_SIZE >> 1);

    		while(ulSize--)
    		{
    			pubUART1FIFO[usUART1FIFOWritePos++] = *pubDMABufferReadPos++;

    			if(usUART1FIFOWritePos >= UART1_FIFO_SIZE)
    				usUART1FIFOWritePos = 0;
    		}

            DMA1->IFCR = DMA_IFCR_CGIF5 | DMA_IFCR_CTCIF5 | DMA_IFCR_CHTIF5 | DMA_IFCR_CTEIF5; // Clear all interrupt flags otherwise DMA won't start again
            DMA1_Channel5->CNDTR = UART1_DMA_RX_BUFFER_SIZE;
            DMA1_Channel5->CMAR = (uint32_t)pubUART1DMABuffer;
            DMA1_Channel5->CCR |= DMA_CCR5_EN; // Enable DMA channel
        }
    }*/
}
static void usart0_dma_isr(uint8_t ubError)
{
/*    volatile uint8_t *pubDMABufferReadPos = 0;

    if(DMA1->ISR & DMA_ISR_HTIF5)
    {
        DMA1->IFCR = DMA_IFCR_CHTIF5; // Clear Half-transfer flag

        pubDMABufferReadPos = pubUART1DMABuffer;
    }
    else if(DMA1->ISR & DMA_ISR_TCIF5)
	{
        DMA1->IFCR = DMA_IFCR_CTCIF5; // Clear Transfer complete flag

        pubDMABufferReadPos = pubUART1DMABuffer + (UART1_DMA_RX_BUFFER_SIZE >> 1);
    }

    if(pubDMABufferReadPos)
    {
        uint32_t ulSize = (UART1_DMA_RX_BUFFER_SIZE >> 1);

		while(ulSize--)
		{
			pubUART1FIFO[usUART1FIFOWritePos++] = *pubDMABufferReadPos++;

			if(usUART1FIFOWritePos >= UART1_FIFO_SIZE)
				usUART1FIFOWritePos = 0;
		}
    }*/
}

void usart0_init(uint32_t ulBaud, uint32_t ulFrameSettings, int8_t bRXLocation, int8_t bTXLocation, int8_t bCTSLocation, int8_t bRTSLocation)
{
    if(bRXLocation > -1 && bRXLocation > AFCHANLOC_MAX)
        return;

    if(bTXLocation > -1 && bTXLocation > AFCHANLOC_MAX)
        return;

    if(bCTSLocation > -1 && bCTSLocation > AFCHANLOC_MAX)
        return;

    if(bRTSLocation > -1 && bRTSLocation > AFCHANLOC_MAX)
        return;

    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_USART0;

    USART0->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX | UART_CMD_TXTRIDIS | USART_CMD_RXBLOCKDIS | USART_CMD_TXDIS | USART_CMD_RXDIS;

    free((uint8_t *)pubUSART0DMABuffer);
    free((uint8_t *)pubUSART0FIFO);

    pubUSART0DMABuffer = (volatile uint8_t *)malloc(USART0_DMA_RX_BUFFER_SIZE);

    if(!pubUSART0DMABuffer)
        return;

    memset((uint8_t *)pubUSART0DMABuffer, 0, USART0_DMA_RX_BUFFER_SIZE);

    pubUSART0FIFO = (volatile uint8_t *)malloc(USART0_FIFO_SIZE);

    if(!pubUSART0FIFO)
    {
        free((void *)pubUSART0DMABuffer);

        return;
    }

    memset((uint8_t *)pubUSART0FIFO, 0, USART0_FIFO_SIZE);

    usUSART0FIFOWritePos = 0;
    usUART2FIFOReadPos = 0;

    USART0->CTRL = USART_CTRL_TXBIL_EMPTY | USART_CTRL_CSMA_NOACTION | UART_CTRL_OVS_X16;
    USART0->CTRLX = (bCTSLocation >= 0 ? USART_CTRLX_CTSEN : 0);
    USART0->FRAME = ulFrameSettings;
    USART0->CLKDIV = ((HFPER_CLOCK_FREQ / (16 * ulBaud)) - 1) << 8;

    USART0->TIMECMP0 = UART_TIMECMP0_TSTOP_RXACT | UART_TIMECMP0_TSTART_RXEOF | 0x08; // RX Timeout after 8 baud times

    USART0->ROUTEPEN = (bRXLocation >= 0 ? USART_ROUTEPEN_RXPEN : 0) | (bTXLocation >= 0 ? USART_ROUTEPEN_TXPEN : 0) | (bCTSLocation >= 0 ? USART_ROUTEPEN_CTSPEN : 0) | (bRTSLocation >= 0 ? USART_ROUTEPEN_RTSPEN : 0);
    USART0->ROUTELOC0 = ((uint32_t)(bRXLocation >= 0 ? bRXLocation : 0) << _USART_ROUTELOC0_RXLOC_SHIFT) | ((uint32_t)(bTXLocation >= 0 ? bTXLocation : 0) << _USART_ROUTELOC0_TXLOC_SHIFT);
    USART0->ROUTELOC1 = ((uint32_t)(bCTSLocation >= 0 ? bCTSLocation : 0) << _USART_ROUTELOC1_CTSLOC_SHIFT) | ((uint32_t)(bRTSLocation >= 0 ? bRTSLocation : 0) << _USART_ROUTELOC1_RTSLOC_SHIFT);

    USART0->IFC = _USART_IFC_MASK; // Clear all flags
    IRQ_CLEAR(USART0_RX_IRQn); // Clear pending vector
    IRQ_SET_PRIO(USART0_RX_IRQn, 2, 1); // Set priority 2,1
    IRQ_ENABLE(USART0_RX_IRQn); // Enable vector
    USART0->IEN = USART_IEN_TCMP0; // Enable TCMP0 flag

    ldma_ch_config(USART0_DMA_CHANNEL, LDMA_CH_REQSEL_SOURCESEL_USART0 | LDMA_CH_REQSEL_SIGSEL_USART0RXDATAV, LDMA_CH_CFG_SRCINCSIGN_DEFAULT, LDMA_CH_CFG_DSTINCSIGN_POSITIVE, LDMA_CH_CFG_ARBSLOTS_DEFAULT, 0);
    ldma_ch_set_isr(USART0_DMA_CHANNEL, usart0_dma_isr);

    //pUSART0DMADescriptor[0].CTRL


    USART0->CMD = (bTXLocation >= 0 ? USART_CMD_TXEN : 0) | (bRXLocation >= 0 ? USART_CMD_RXEN : 0);
}
void usart0_write_byte(const uint8_t ubData)
{
    while(!(USART0->STATUS & USART_STATUS_TXBL));

    USART0->TXDATA = ubData;
}
uint8_t usart0_read_byte()
{
    if(!usart0_available())
        return 0;

    uint8_t ubData;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        ubData = pubUSART0FIFO[usUART2FIFOReadPos++];

        if(usUART2FIFOReadPos >= USART0_FIFO_SIZE)
            usUART2FIFOReadPos = 0;
    }

    return ubData;
}
uint32_t usart0_available()
{
    return (USART0_FIFO_SIZE + usUSART0FIFOWritePos - usUART2FIFOReadPos) % USART0_FIFO_SIZE;
}
void usart0_flush()
{
    usUART2FIFOReadPos = usUSART0FIFOWritePos = 0;
}
#endif  // USART0_MODE

// - - - - - - - - - - - - - - - TEMPORARY - - - - - - - - - - - - - -
void usart1_init(uint32_t ulBaud, uint8_t ubMode, uint8_t ubBitMode, int8_t bMISOLocation, int8_t bMOSILocation, uint8_t ubCLKLocation)
{
    if(bMISOLocation > -1 && bMISOLocation > AFCHANLOC_MAX)
        return;

    if(bMOSILocation > -1 && bMOSILocation > AFCHANLOC_MAX)
        return;

    if(ubCLKLocation > AFCHANLOC_MAX)
        return;

    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_USART1;

    USART1->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX | UART_CMD_TXTRIDIS | USART_CMD_RXBLOCKDIS | USART_CMD_TXDIS | USART_CMD_RXDIS;

    USART1->CTRL = USART_CTRL_TXBIL_EMPTY | USART_CTRL_CSMA_NOACTION | (ubBitMode == USART_SPI_MSB_FIRST ? USART_CTRL_MSBF : 0) | (ubMode & 1 ? USART_CTRL_CLKPHA_SAMPLETRAILING : USART_CTRL_CLKPHA_SAMPLELEADING) | (ubMode & 2 ? USART_CTRL_CLKPOL_IDLEHIGH : USART_CTRL_CLKPOL_IDLELOW) | USART_CTRL_SYNC;
    USART1->FRAME = USART_FRAME_DATABITS_EIGHT;
    USART1->CLKDIV = ((HFPER_CLOCK_FREQ / (2 * ulBaud)) - 1) << 8;

    USART1->ROUTEPEN = (bMISOLocation >= 0 ? USART_ROUTEPEN_RXPEN : 0) | (bMOSILocation >= 0 ? USART_ROUTEPEN_TXPEN : 0) | USART_ROUTEPEN_CLKPEN;
    USART1->ROUTELOC0 = ((uint32_t)(bMISOLocation >= 0 ? bMISOLocation : 0) << _USART_ROUTELOC0_RXLOC_SHIFT) | ((uint32_t)(bMOSILocation >= 0 ? bMOSILocation : 0) << _USART_ROUTELOC0_TXLOC_SHIFT) | ((uint32_t)ubCLKLocation << _USART_ROUTELOC0_CLKLOC_SHIFT);

    USART1->CMD = USART_CMD_MASTEREN | USART_CMD_TXEN | USART_CMD_RXEN;
}
uint8_t usart1_spi_transfer_byte(const uint8_t ubData)
{
    while(!(USART1->STATUS & USART_STATUS_TXBL));

    USART1->TXDATA = ubData;

    while(!(USART1->STATUS & USART_STATUS_TXC));

    return USART1->RXDATA;
}
void usart1_spi_transfer(const uint8_t* pubSrc, uint32_t ulSize, uint8_t* pubDst)
{
	if(pubSrc)
	{
		while(ulSize--)
		{
			if(pubDst)
				*(pubDst++) = usart1_spi_transfer_byte(*(pubSrc++));
			else
				usart1_spi_transfer_byte(*(pubSrc++));
		}
	}
	else if(pubDst)
	{
		while(ulSize--)
			*(pubDst++) = usart1_spi_transfer_byte(0x00);
	}
}
// - - - - - - - - - - - - - - - TEMPORARY - - - - - - - - - - - - - -