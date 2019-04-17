#include "ili9488.h"

static uint16_t usMaxWidth;
static uint16_t usMaxHeigth;

static inline void ili9488_send_cmd(uint8_t ubCmd, uint8_t *pubParam, uint8_t ubNParam)
{
    ILI9488_SELECT();
    ILI9488_SETUP_CMD();

    usart1_spi_transfer_byte(ubCmd);

    if(pubParam && ubNParam)
    {
        ILI9488_SETUP_DAT();

        usart1_spi_write(pubParam, ubNParam);
    }

    ILI9488_UNSELECT();
}
static inline void ili9488_read_data(uint8_t *pubData, uint8_t ubNData)
{
    if(pubData && ubNData)
    {
        ILI9488_SELECT();
        ILI9488_SETUP_DAT();

        usart1_spi_transfer_byte(0x00); // dummy byte
        usart1_spi_read(pubData, ubNData);

        ILI9488_UNSELECT();
    }
}
static inline void ili9488_send_pixel_data(rgb565_t xColor)
{
    ILI9488_SELECT();
    ILI9488_SETUP_DAT();

    usart1_spi_transfer_byte(RGB565_EXTRACT_RED(xColor));
    usart1_spi_transfer_byte(RGB565_EXTRACT_GREEN(xColor));
    usart1_spi_transfer_byte(RGB565_EXTRACT_BLUE(xColor));

    ILI9488_UNSELECT();
}
static inline uint8_t ili9488_set_window(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1)
{
    if(usX0 > usX1)
        return 0;
    if(usY0 > usY1)
        return 0;
    if(usX1 > usMaxWidth)
        return 0;
    if(usY1 > usMaxHeigth)
        return 0;

    uint8_t ubBuf[4];

    ubBuf[0] = usX0 >> 8;
    ubBuf[1] = usX0 & 0xFF; // XSTART
    ubBuf[2] = usX1 >> 8;
    ubBuf[3] = usX1 & 0xFF; // XEND
    ili9488_send_cmd(ILI9488_C_ADDR_SET, ubBuf, 4); // Column addr set

    ubBuf[0] = usY0 >> 8;
    ubBuf[1] = usY0 & 0xFF; // YSTART
    ubBuf[2] = usY1 >> 8;
    ubBuf[3] = usY1 & 0xFF; // YEND
    ili9488_send_cmd(ILI9488_P_ADDR_SET, ubBuf, 4); // Row addr set

    ili9488_send_cmd(ILI9488_RAM_WR, NULL, 0); // write to RAM

    return 1;
}

uint8_t ili9488_init()
{
    ILI9488_RESET();
    delay_ms(10);
    ILI9488_UNRESET();
    delay_ms(120);

    uint8_t ubBuf[15];

    ubBuf[0] =  0x00;
    ubBuf[1] =  0x03;
    ubBuf[2] =  0x09;
    ubBuf[3] =  0x08;
    ubBuf[4] =  0x16;
    ubBuf[5] =  0x0A;
    ubBuf[6] =  0x3F;
    ubBuf[7] =  0x78;
    ubBuf[8] =  0x4C;
    ubBuf[9] =  0x09;
    ubBuf[10] = 0x0A;
    ubBuf[11] = 0x08;
    ubBuf[12] = 0x16;
    ubBuf[13] = 0x1A;
    ubBuf[14] = 0x0F;
    ili9488_send_cmd(ILI9488_PGAMCTRL, ubBuf, 15);  // PGAMCTRL(Positive Gamma Control)

    ubBuf[0] =  0x00;
    ubBuf[1] =  0x16;
    ubBuf[2] =  0x19;
    ubBuf[3] =  0x03;
    ubBuf[4] =  0x0F;
    ubBuf[5] =  0x05;
    ubBuf[6] =  0x32;
    ubBuf[7] =  0x45;
    ubBuf[8] =  0x46;
    ubBuf[9] =  0x04;
    ubBuf[10] = 0x0E;
    ubBuf[11] = 0x0D;
    ubBuf[12] = 0x35;
    ubBuf[13] = 0x37;
    ubBuf[14] = 0x0F;
	ili9488_send_cmd(ILI9488_NGAMCTRL, ubBuf, 15);  // NGAMCTRL(Negative Gamma Control)

    ubBuf[0] = 0x17;    //Vreg1out
    ubBuf[1] = 0x15;    //Verg2out
	ili9488_send_cmd(ILI9488_POW_CTL_1, ubBuf, 2);      //Power Control 1

    ubBuf[0] = 0x41;    //VGH,VGL
	ili9488_send_cmd(ILI9488_POW_CTL_2, ubBuf, 1);      //Power Control 2

    ubBuf[0] = 0x00;
	ubBuf[0] = 0x12;    //Vcom
	ubBuf[0] = 0x80;
	ili9488_send_cmd(ILI9488_VCOM_CTL_1, ubBuf, 3);      //Power Control 3

    ubBuf[0] = 0x48;    // MX | BGR
	ili9488_send_cmd(ILI9488_MEM_A_CTL, ubBuf, 1);      //Memory Access

    ubBuf[0] = 0x66;    //18 bit
	ili9488_send_cmd(ILI9488_PIX_FMT, ubBuf, 1);      // Interface Pixel Format

    ubBuf[0] = 0x00;
	ili9488_send_cmd(ILI9488_IF_MD_CTL, ubBuf, 1);      // Interface Mode Control

    ubBuf[0] = 0xA0;    //60Hz
	ili9488_send_cmd(ILI9488_FRMRT_CTL_1, ubBuf, 1);      //Frame rate

    ubBuf[0] = 0x02;    // 2-dot
	ili9488_send_cmd(ILI9488_INV_CTL, ubBuf, 1);      //Display Inversion Control

    ubBuf[0] = 0x02; //MCU
    ubBuf[1] = 0x02; //Source,Gate scan dieection
	ili9488_send_cmd(ILI9488_DISP_FUNC_CTL, ubBuf, 2);      //Display Function Control  RGB/MCU Interface Control

    ubBuf[0] = 0x00;    // Disable 24 bit data
	ili9488_send_cmd(ILI9488_SET_IMG_FUNC, ubBuf, 1);      // Set Image Functio

    ubBuf[0] = 0xA9;
    ubBuf[1] = 0x51;
    ubBuf[2] = 0x2C;
    ubBuf[3] = 0x82;    // D7 stream, loose
	ili9488_send_cmd(ILI9488_ADJ_CTL_3, ubBuf, 4);      // Adjust Control

	ili9488_wakeup(0);  // wake up controller, leave display off
}
uint32_t ili9488_read_id() // FIXME: returns 000000
{
    uint8_t ubBuf[3];

    ili9488_send_cmd(ILI9488_RD_DISP_ID, NULL, 0);
    ili9488_read_data(ubBuf, 3);

    return ((uint32_t)ubBuf[0] << 16) | ((uint32_t)ubBuf[1] << 8) | (uint32_t)ubBuf[2];
}
void tft_bl_init(uint32_t usFrequency)
{
    CMU->HFPERCLKEN1 |= CMU_HFPERCLKEN1_WTIMER2;

    WTIMER2->CTRL = WTIMER_CTRL_RSSCOIST | WTIMER_CTRL_PRESC_DIV1 | WTIMER_CTRL_CLKSEL_PRESCHFPERCLK | WTIMER_CTRL_FALLA_NONE | WTIMER_CTRL_RISEA_NONE | WTIMER_CTRL_MODE_UP;
    WTIMER2->TOP = HFPER_CLOCK_FREQ / usFrequency;
    WTIMER2->CNT = 0x00000000;

    WTIMER2->CC[1].CTRL = WTIMER_CC_CTRL_PRSCONF_LEVEL | WTIMER_CC_CTRL_CUFOA_NONE | WTIMER_CC_CTRL_COFOA_SET | WTIMER_CC_CTRL_CMOA_CLEAR | WTIMER_CC_CTRL_MODE_PWM;
    WTIMER2->CC[1].CCV = 0x00000000;

    WTIMER2->ROUTELOC0 = WTIMER_ROUTELOC0_CC1LOC_LOC2;
    WTIMER2->ROUTEPEN |= TIMER_ROUTEPEN_CC1PEN;

    WTIMER2->CMD = WTIMER_CMD_START;

    WTIMER2->CC[1].CCVB = 0;
}
void tft_bl_set(float fBrightness)
{
    if(fBrightness > 1)
        fBrightness = 1;
    if(fBrightness < 0)
        fBrightness = 0;

    WTIMER2->CC[1].CCVB = WTIMER2->TOP * fBrightness;
}
void ili9488_sleep()
{
    ili9488_display_off();
    delay_ms(10);
    ili9488_send_cmd(ILI9488_SLP_IN, NULL, 0);  // Internal oscillator will be stopped
    delay_ms(120);
}
void ili9488_wakeup(uint8_t ubDisplayOn)
{
    ili9488_send_cmd(ILI9488_SLP_OUT, NULL, 0); // Sleep out
	delay_ms(120);

    if(ubDisplayOn)
    {
        ili9488_display_on();
    }
}
void ili9488_display_on()
{
    ili9488_send_cmd(ILI9488_DISP_ON, NULL, 0); //Display on
}
void ili9488_display_off()
{
    ili9488_send_cmd(ILI9488_DISP_OFF, NULL, 0); //Display off
}
void ili9488_set_rotation(uint8_t ubRotation)
{
    uint8_t ubBuf;

    switch(ubRotation)
    {
        case 0:
            ubBuf = ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR;
            usMaxWidth = ILI9488_TFTWIDTH - 1;
            usMaxHeigth = ILI9488_TFTHEIGHT - 1;
            break;
        case 1:
            ubBuf = ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR;
            usMaxWidth = ILI9488_TFTHEIGHT - 1;
            usMaxHeigth = ILI9488_TFTWIDTH - 1;
            break;
        case 2:
            ubBuf = ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR;
            usMaxWidth = ILI9488_TFTWIDTH - 1;
            usMaxHeigth = ILI9488_TFTHEIGHT - 1;
            break;
        case 3:
            ubBuf = ILI9488_MADCTL_MX | ILI9488_MADCTL_MY | ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR;
            usMaxWidth = ILI9488_TFTHEIGHT - 1;
            usMaxHeigth = ILI9488_TFTHEIGHT - 1;
            break;
        default:
            return;
    }

    ili9488_send_cmd(ILI9488_MEM_A_CTL, &ubBuf, 1);
}
void ili9488_set_invert(uint8_t ubOnOff)
{
    ili9488_send_cmd((ubOnOff ? ILI9488_INV_ON : ILI9488_INV_OFF), NULL, 0);
}

void ili9488_set_scroll_area(uint16_t usTopFixedArea, uint16_t usBottomFixedArea)
{
    uint8_t ubBuf[6];

    ubBuf[0] = usTopFixedArea >> 8;
    ubBuf[1] = usTopFixedArea & 0x00FF;
    ubBuf[2] = (usMaxHeigth - usTopFixedArea - usBottomFixedArea) >> 8;
    ubBuf[3] = (usMaxHeigth - usTopFixedArea - usBottomFixedArea) & 0x00FF;
    ubBuf[4] = usBottomFixedArea >> 8;
    ubBuf[5] = usBottomFixedArea & 0x00FF;
    ili9488_send_cmd(ILI9488_VSCRL_DEF, ubBuf, 6); // Vertical scroll definition
}
void ili9488_scroll(uint16_t usPixs)
{
    uint8_t ubBuf[2];

    ubBuf[0] = usPixs >> 8;
    ubBuf[0] = usPixs & 0x00FF;
    ili9488_send_cmd(ILI9488_VSCRL_ADDR, ubBuf, 2); // Vertical scrolling start address
}

void ili9488_fill_screen(rgb565_t xColor)
{
    if(!ili9488_set_window(0, 0, usMaxWidth, usMaxHeigth))
        return;

    for(uint32_t ulI = ILI9488_TFTWIDTH * ILI9488_TFTHEIGHT; ulI > 0; ulI--)
        ili9488_send_pixel_data(xColor);
}

void ili9488_draw_pixel(uint16_t usX, uint16_t usY, rgb565_t xColor)
{
    if(!ili9488_set_window(usX, usY, usX + 1, usY + 1))
        return;

    ili9488_send_pixel_data(xColor);
}
void ili9488_draw_fast_v_line(uint16_t usX, uint16_t usY0, uint16_t usY1, rgb565_t xColor)
{
    if(usY0 > usY1)
        SWAP(usY0, usY1);

    if(!ili9488_set_window(usX, usY0, usX, usY1))
        return;

    for(uint16_t usH = usY1 - usY0; usH > 0; usH--)
        ili9488_send_pixel_data(xColor);
}
void ili9488_draw_fast_h_line(uint16_t usX0, uint16_t usY, uint16_t usX1, rgb565_t xColor)
{
    if(usX0 > usX1)
        SWAP(usX0, usX1);

    if(!ili9488_set_window(usX0, usY, usX1, usY))
        return;

    for(uint16_t usW = usX1 - usX0; usW > 0; usW--)
        ili9488_send_pixel_data(xColor);
}
void ili9488_draw_line(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1, rgb565_t xColor)
{
    if(usX0 == usX1)
    {
        ili9488_draw_fast_v_line(usX0, usY0, usY1, xColor);

        return;
    }
    if(usY0 == usY1)
    {
        ili9488_draw_fast_h_line(usX0, usY0, usX1, xColor);

        return;
    }

	uint8_t ubSteep = ABS(usY1 - usY0)  > ABS(usX1 - usX0);
	uint8_t ubdx, ubdy;
	int8_t bErr;
	int8_t bYstep;

	if(ubSteep)
	{
        SWAP(usX0, usY0);
        SWAP(usX1, usY1);
	}

	if(usX0 > usX1)
	{
        SWAP(usX0, usX1);
        SWAP(usY0, usY1);
	}

	ubdx = usX1 - usX0;
	ubdy = ABS(usY1 - usY0);

	bErr = ubdx / 2;

	if(usY0 < usY1)
		bYstep = 1;
    else
		bYstep = -1;

	for(; usX0 <= usX1; usX0++)
	{
		if(ubSteep)
			ili9488_draw_pixel(usY0, usX0, xColor);
        else
			ili9488_draw_pixel(usX0, usY0, xColor);

		bErr -= ubdy;

		if(bErr < 0)
		{
			usY0 += bYstep;
			bErr += ubdx;
		}
	}
}
void ili9488_draw_rectangle(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1, rgb565_t xColor, uint8_t ubFill)
{
    if(usX0 > usX1)
        SWAP(usX0, usX1);

    if(usY0 > usY1)
        SWAP(usY0, usY1);

    if(ubFill)
    {
        if(!ili9488_set_window(usX0, usY0, usX1, usY1))
            return;

        for(uint16_t usH = usY1 - usY0; usH > 0; usH--)
        {
            for(uint16_t usW = usX1 - usX0; usW > 0; usW--)
                ili9488_send_pixel_data(xColor);
        }
    }
    else
    {
        ili9488_draw_fast_h_line(usX0, usY0, usX1, xColor);
        ili9488_draw_fast_v_line(usX1, usY0, usY1, xColor);
        ili9488_draw_fast_v_line(usX0, usY0, usY1, xColor);
        ili9488_draw_fast_h_line(usX0, usY1, usX1, xColor);
    }
}
void ili9488_draw_circle(uint16_t usX, uint16_t usY, uint16_t usR, rgb565_t xColor, uint8_t ubFill)
{
    int16_t sF = 1 - usR;
    int16_t sDdFx = 1;
    int16_t sDdFy = -2 * usR;
    int16_t sXh = 0;
    int16_t sYh = usR;

    if(ubFill)
    {
        for(int16_t usI = usY - usR; usI <= usY + usR; usI++)
            ili9488_draw_pixel(usX, usI, xColor);
    }
    else
    {
        ili9488_draw_pixel(usX, usY + usR, xColor);
        ili9488_draw_pixel(usX, usY - usR, xColor);
        ili9488_draw_pixel(usX + usR, usY, xColor);
        ili9488_draw_pixel(usX - usR, usY, xColor);
    }

    while (sXh<sYh)
    {
        if (sF >= 0)
        {
            sYh--;
            sDdFy += 2;
            sF += sDdFy;
        }

        sXh++;
        sDdFx += 2;
        sF += sDdFx;

        if(ubFill)
        {
            for(int16_t usI = usY - sYh; usI <= usY + sYh; usI++)
            {
                ili9488_draw_pixel(usX + sXh, usI, xColor);
                ili9488_draw_pixel(usX - sXh, usI, xColor);
            }

            for(int16_t usI = usY - sXh; usI <= usY + sXh; usI++)
            {
                ili9488_draw_pixel(usX + sYh, usI, xColor);
                ili9488_draw_pixel(usX - sYh, usI, xColor);
            }
        }
        else
        {
            ili9488_draw_pixel(usX + sXh, usY + sYh, xColor);
            ili9488_draw_pixel(usX - sXh, usY + sYh, xColor);
            ili9488_draw_pixel(usX + sXh, usY - sYh, xColor);
            ili9488_draw_pixel(usX - sXh, usY - sYh, xColor);
            ili9488_draw_pixel(usX + sYh, usY + sXh, xColor);
            ili9488_draw_pixel(usX - sYh, usY + sXh, xColor);
            ili9488_draw_pixel(usX + sYh, usY - sXh, xColor);
            ili9488_draw_pixel(usX - sYh, usY - sXh, xColor);
        }
    }
}

void ili9488_draw_image(const image_t *pImage, uint16_t usX, uint16_t usY)
{
    if(!pImage)
        return;

    if(!pImage->pPixels)
        return;

    if(!ili9488_set_window(usX, usY, usX + pImage->usWidth - 1, usY + pImage->usHeight - 1))
        return;

    rgb565_t *pPixels = pImage->pPixels;
    uint32_t ulImgSize = pImage->usWidth * pImage->usHeight;

    while(ulImgSize--)
        ili9488_send_pixel_data(*pPixels++);
}
void ili9488_draw_bitmap(const uint8_t *pubBitmap, uint16_t usX, uint16_t usY, uint16_t usW, uint16_t usH, rgb565_t xColor, rgb565_t xBackColor)
{
    if(!pubBitmap)
        return;

    if(!ili9488_set_window(usX, usY, usX + usW - 1, usY + usH - 1))
        return;

    for(uint32_t ulI = 0; ulI < usW * usH; ulI++)
    {
        if(*(pubBitmap + (ulI / 8u)) & (0x80 >> (ulI % 8ul)))
            ili9488_send_pixel_data(xColor);
        else
            ili9488_send_pixel_data(xBackColor);
    }
}

uint8_t ili9488_draw_char(uint8_t ubChar, const font_t *pxFont, uint16_t usX, uint16_t usY, rgb565_t xColor, rgb565_t xBackColor)
{
    if((ubChar < pxFont->ubFirstChar) || (ubChar > pxFont->ubLastChar))
        ubChar = '?';

    ubChar -= pxFont->ubFirstChar;

    ili9488_draw_bitmap(pxFont->pubBitmap + pxFont->pGlyph[ubChar].usBitmapOffset, usX + pxFont->pGlyph[ubChar].bXOffset, usY + pxFont->ubYAdvance + pxFont->pGlyph[ubChar].bYOffset, pxFont->pGlyph[ubChar].ubWidth, pxFont->pGlyph[ubChar].ubHeight, xColor, xBackColor);

    return pxFont->pGlyph[ubChar].ubXAdvance;
}
void ili9488_draw_string(uint8_t *pubStr, const font_t *pxFont, uint16_t usX, uint16_t usY, rgb565_t xColor, rgb565_t xBackColor)
{
    uint16_t usXh = usX;
    uint16_t usYh = usY;

    while(*pubStr)
    {
        if(*pubStr == '\n')
            usYh += pxFont->ubYAdvance;
        else if(*pubStr == '\r')
            usXh = usX;
        else
            usXh += ili9488_draw_char(*pubStr, pxFont, usXh, usYh, xColor, xBackColor);

        pubStr++;
    }
}
void ili9488_printf(const font_t *pxFont, uint16_t usX, uint16_t usY, rgb565_t xColor, rgb565_t xBackColor , const char* pszFmt, ...)
{
    va_list args;
	va_start(args, pszFmt);

    uint32_t ulStrLen = vsnprintf(NULL, 0, pszFmt, args);
	char *pszBuf = (char *)malloc(ulStrLen + 1);

    if(!pszBuf)
    {
        va_end(args);
        return;
    }

    vsnprintf(pszBuf, ulStrLen + 1, pszFmt, args);

	ili9488_draw_string(pszBuf, pxFont, usX, usY, xColor, xBackColor);

    free(pszBuf);

	va_end(args);
}

textbox_t *ili9488_textbox_create(uint16_t usX, uint16_t usY, uint16_t usNumLines, uint16_t usLenght, font_t *pxFont, rgb565_t xColor, rgb565_t xBackColor, rgb565_t ulOptions)
{
    textbox_t *pxNewTextbox = malloc(sizeof(textbox_t));
    if(!pxNewTextbox)
        return NULL;

    pxNewTextbox->usX = usX;
    pxNewTextbox->usY = usY;
    pxNewTextbox->usNumLines = usNumLines;
    pxNewTextbox->usLen = usLenght;
    pxNewTextbox->pxFont = pxFont;
    pxNewTextbox->xColor = xColor;
    pxNewTextbox->xBackColor = xBackColor;

    ili9488_textbox_clear(pxNewTextbox);

    return pxNewTextbox;
}
void ili9488_textbox_delete(textbox_t *pxTextbox)
{
    if(pxTextbox)
        free(pxTextbox);
}
void ili9488_textbox_clear(textbox_t *pxTextbox)
{
    ili9488_draw_rectangle(
        pxTextbox->usX,
        pxTextbox->usY,
        pxTextbox->usX + pxTextbox->usLen - 1,
        pxTextbox->usY + (pxTextbox->usNumLines * pxTextbox->pxFont->ubYAdvance) - 1,
        pxTextbox->xBackColor,
        1
        );

    pxTextbox->usCursor = 0;
    pxTextbox->usCurrentLine = 0;
}
void ili9488_textbox_clear_line(textbox_t *pxTextbox)
{
    ili9488_draw_rectangle(
        pxTextbox->usX,
        pxTextbox->usY + (pxTextbox->usCurrentLine * pxTextbox->pxFont->ubYAdvance),
        pxTextbox->usX + pxTextbox->usLen - 1,
        pxTextbox->usY + ((pxTextbox->usCurrentLine + 1) * pxTextbox->pxFont->ubYAdvance) - 1,
        pxTextbox->xBackColor,
        1
        );

    pxTextbox->usCursor = 0;
}
void ili9488_textbox_goto(textbox_t *pxTextbox, uint16_t usCursor, uint16_t usLine)
{
    pxTextbox->usCursor = usCursor;
    pxTextbox->usCurrentLine = usLine;
}
void ili9488_textbox_draw_string(textbox_t *pxTextbox, uint8_t *pubStr)
{
    while(*pubStr)
    {
        if(*pubStr == '\n')
        {
            pxTextbox->usCurrentLine++;
            if(pxTextbox->usCurrentLine == pxTextbox->usNumLines)
                pxTextbox->usCurrentLine = 0;
        }
        else if(*pubStr == '\r')
            ili9488_textbox_clear_line(pxTextbox);
        else if((pxTextbox->usCursor + pxTextbox->pxFont->pGlyph[*pubStr - pxTextbox->pxFont->ubFirstChar].ubXAdvance) < pxTextbox->usLen)
            pxTextbox->usCursor += ili9488_draw_char(
                *pubStr,
                pxTextbox->pxFont,
                pxTextbox->usCursor,
                pxTextbox->usY + (pxTextbox->usCurrentLine * pxTextbox->pxFont->ubYAdvance),
                pxTextbox->xColor,
                pxTextbox->xBackColor
                );

        pubStr++;
    }
}
void ili9488_textbox_printf(textbox_t *pxTextbox, const char* pszFmt, ...)
{
    va_list args;
	va_start(args, pszFmt);

    uint32_t ulStrLen = vsnprintf(NULL, 0, pszFmt, args);
	char *pszBuf = (char *)malloc(ulStrLen + 1);

    if(!pszBuf)
    {
        va_end(args);
        return;
    }

    vsnprintf(pszBuf, ulStrLen + 1, pszFmt, args);

	ili9488_textbox_draw_string(pxTextbox, pszBuf);

    free(pszBuf);

	va_end(args);
}

// TODO: ili9488_draw_line_graph()
// TODO: ili9488_draw_bar_graph()
// TODO: draw_heat_map()
// TODO: draw_radar_spider_chart()
// TODO: draw_scatter_plot()
// TODO: gantt chart