#include "ws2812b.h"

static uint16_t *pusBuffer = NULL;
static ldma_descriptor_t __attribute__ ((aligned (4))) pDMADescriptor[3];

void ws2812b_init()
{
    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER1;

    free(pusBuffer);

    pusBuffer = (uint16_t *)malloc(WS2812B_NUM_LEDS * 3 * 8 * sizeof(uint16_t)); // 3 = Colors, 8 = Bits per color

    if(!pusBuffer)
        return;

    memset(pusBuffer, 0, WS2812B_NUM_LEDS * 3 * 8 * sizeof(uint16_t));

    TIMER1->CTRL = TIMER_CTRL_RSSCOIST | TIMER_CTRL_PRESC_DIV1 | TIMER_CTRL_CLKSEL_PRESCHFPERCLK | TIMER_CTRL_FALLA_NONE | TIMER_CTRL_RISEA_NONE | TIMER_CTRL_DMACLRACT | TIMER_CTRL_MODE_UP;
    TIMER1->TOP = (HFPER_CLOCK_FREQ / WS2812B_FREQ) - 1;
    TIMER1->CNT = 0x0000;

    TIMER1->CC[0].CTRL = TIMER_CC_CTRL_PRSCONF_LEVEL | TIMER_CC_CTRL_CUFOA_NONE | TIMER_CC_CTRL_COFOA_SET | TIMER_CC_CTRL_CMOA_CLEAR | (WS2812B_INV ? TIMER_CC_CTRL_OUTINV : 0) | TIMER_CC_CTRL_MODE_OUTPUTCOMPARE;
    TIMER1->CC[0].CCV = 0x0000;

    TIMER1->ROUTELOC0 = TIMER_ROUTELOC0_CC0LOC_LOC2;
    TIMER1->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN;

    ldma_ch_disable(WS2812B_DMA_CHANNEL);
    ldma_ch_peri_req_disable(WS2812B_DMA_CHANNEL);
    ldma_ch_req_clear(WS2812B_DMA_CHANNEL);

    ldma_ch_config(WS2812B_DMA_CHANNEL, LDMA_CH_REQSEL_SOURCESEL_TIMER1 | LDMA_CH_REQSEL_SIGSEL_TIMER1UFOF, LDMA_CH_CFG_SRCINCSIGN_POSITIVE, LDMA_CH_CFG_DSTINCSIGN_DEFAULT, LDMA_CH_CFG_ARBSLOTS_DEFAULT, 0);

    pDMADescriptor[0].CTRL = LDMA_CH_CTRL_STRUCTTYPE_WRITE;
    pDMADescriptor[0].IMMVAL = TIMER_CMD_START;
    pDMADescriptor[0].DST = &(TIMER1->CMD);
    pDMADescriptor[0].LINK = 0x00000010 | LDMA_CH_LINK_LINK | LDMA_CH_LINK_LINKMODE_RELATIVE;

    pDMADescriptor[1].CTRL = LDMA_CH_CTRL_DSTMODE_ABSOLUTE | LDMA_CH_CTRL_SRCMODE_ABSOLUTE | LDMA_CH_CTRL_DSTINC_NONE | LDMA_CH_CTRL_SIZE_HALFWORD | LDMA_CH_CTRL_SRCINC_ONE | LDMA_CH_CTRL_REQMODE_BLOCK | LDMA_CH_CTRL_BLOCKSIZE_UNIT1 | ((((WS2812B_NUM_LEDS * 3 * 8) - 1) << _LDMA_CH_CTRL_XFERCNT_SHIFT) & _LDMA_CH_CTRL_XFERCNT_MASK) | LDMA_CH_CTRL_STRUCTREQ | LDMA_CH_CTRL_STRUCTTYPE_TRANSFER;
    pDMADescriptor[1].SRC = pusBuffer;
    pDMADescriptor[1].DST = &(TIMER1->CC[0].CCV);
    pDMADescriptor[1].LINK = 0x00000010 | LDMA_CH_LINK_LINK | LDMA_CH_LINK_LINKMODE_RELATIVE;

    pDMADescriptor[2].CTRL = LDMA_CH_CTRL_STRUCTTYPE_WRITE;
    pDMADescriptor[2].IMMVAL = TIMER_CTRL_OSMEN;
    pDMADescriptor[2].DST = (void *)PERI_REG_BIT_SET_ADDR(&(TIMER1->CTRL));
    pDMADescriptor[2].LINK = 0x00000000;

    ldma_ch_peri_req_enable(WS2812B_DMA_CHANNEL);
    ldma_ch_enable(WS2812B_DMA_CHANNEL);
}

void ws2812b_set_color(uint16_t usLED, uint8_t ubRed, uint8_t ubGreen, uint8_t ubBlue)
{
    if(usLED >= WS2812B_NUM_LEDS)
        return;

    if(TIMER1->STATUS & TIMER_STATUS_RUNNING)
        return;

    uint32_t ulData = ((uint32_t)ubGreen << 16) | ((uint32_t)ubRed << 8) | ((uint32_t)ubBlue << 0);
    uint32_t ulBufferOffset = (uint32_t)usLED * 3 * 8;
    uint8_t ubBufferCount = 3 * 8;

    while(ubBufferCount--)
    {
        pusBuffer[ulBufferOffset++] = (ulData & BIT(23)) ? 31 : 12; // TODO: Support configurable times

        ulData <<= 1;
    }

    TIMER1->CTRL &= ~TIMER_CTRL_OSMEN;

    ldma_ch_load(WS2812B_DMA_CHANNEL, pDMADescriptor);
}
