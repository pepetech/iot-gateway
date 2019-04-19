#include "tft.h"

void tft_bl_init(uint32_t ulFrequency)
{
    CMU->HFPERCLKEN1 |= CMU_HFPERCLKEN1_WTIMER2;

    WTIMER2->CTRL = WTIMER_CTRL_RSSCOIST | WTIMER_CTRL_PRESC_DIV1 | WTIMER_CTRL_CLKSEL_PRESCHFPERCLK | WTIMER_CTRL_FALLA_NONE | WTIMER_CTRL_RISEA_NONE | WTIMER_CTRL_MODE_UP;
    WTIMER2->TOP = (HFPER_CLOCK_FREQ / ulFrequency) - 1;
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
    if(fBrightness > 1.f)
        fBrightness = 1.f;
    if(fBrightness < 0.f)
        fBrightness = 0.f;

    WTIMER2->CC[1].CCVB = WTIMER2->TOP * fBrightness;
}

void tft_draw_fast_v_line(uint16_t usX, uint16_t usY0, uint16_t usY1, rgb565_t xColor)
{
    if(usY0 > usY1)
        SWAP(usY0, usY1);

    if(!ili9488_set_window(usX, usY0, usX, usY1))
        return;

    uint32_t ulDataLen = usY1 - usY0 + 1;

    while(ulDataLen--)
        ili9488_send_pixel_data(xColor);
}
void tft_draw_fast_h_line(uint16_t usX0, uint16_t usY0, uint16_t usX1, rgb565_t xColor)
{
    if(usX0 > usX1)
        SWAP(usX0, usX1);

    if(!ili9488_set_window(usX0, usY0, usX1, usY0))
        return;

    uint32_t ulDataLen = usX1 - usX0 + 1;

    while(ulDataLen--)
        ili9488_send_pixel_data(xColor);
}
void tft_draw_line(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1, rgb565_t xColor)
{
    if(usX0 == usX1)
    {
        tft_draw_fast_v_line(usX0, usY0, usY1, xColor);

        return;
    }

    if(usY0 == usY1)
    {
        tft_draw_fast_h_line(usX0, usY0, usX1, xColor);

        return;
    }

	uint8_t ubSteep = ABS(usY1 - usY0) > ABS(usX1 - usX0);
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

	while(usX0 <= usX1)
	{
		if(ubSteep)
			ili9488_set_pixel_color(usY0, usX0, xColor);
        else
			ili9488_set_pixel_color(usX0, usY0, xColor);

		bErr -= ubdy;

		if(bErr < 0)
		{
			usY0 += bYstep;
			bErr += ubdx;
		}

        usX0++;
	}
}
void tft_draw_rectangle(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1, rgb565_t xColor, uint8_t ubFill)
{
    if(usX0 > usX1)
        SWAP(usX0, usX1);

    if(usY0 > usY1)
        SWAP(usY0, usY1);

    if(ubFill)
    {
        if(!ili9488_set_window(usX0, usY0, usX1, usY1))
            return;

        uint32_t ulDataLen = (usY1 - usY0 + 1) * (usX1 - usX0 + 1);

        while(ulDataLen--)
            ili9488_send_pixel_data(xColor);
    }
    else
    {
        tft_draw_fast_h_line(usX0, usY0, usX1, xColor);
        tft_draw_fast_v_line(usX1, usY0, usY1, xColor);
        tft_draw_fast_v_line(usX0, usY0, usY1, xColor);
        tft_draw_fast_h_line(usX0, usY1, usX1, xColor);
    }
}
void tft_draw_circle(uint16_t usX, uint16_t usY, uint16_t usR, rgb565_t xColor, uint8_t ubFill)
{
    int16_t sF = 1 - usR;
    int16_t sDdFx = 1;
    int16_t sDdFy = -2 * usR;
    int16_t sXh = 0;
    int16_t sYh = usR;

    if(ubFill)
    {
        for(int16_t usI = usY - usR; usI <= usY + usR; usI++)
            ili9488_set_pixel_color(usX, usI, xColor);
    }
    else
    {
        ili9488_set_pixel_color(usX, usY + usR, xColor);
        ili9488_set_pixel_color(usX, usY - usR, xColor);
        ili9488_set_pixel_color(usX + usR, usY, xColor);
        ili9488_set_pixel_color(usX - usR, usY, xColor);
    }

    while (sXh < sYh)
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
                ili9488_set_pixel_color(usX + sXh, usI, xColor);
                ili9488_set_pixel_color(usX - sXh, usI, xColor);
            }

            for(int16_t usI = usY - sXh; usI <= usY + sXh; usI++)
            {
                ili9488_set_pixel_color(usX + sYh, usI, xColor);
                ili9488_set_pixel_color(usX - sYh, usI, xColor);
            }
        }
        else
        {
            ili9488_set_pixel_color(usX + sXh, usY + sYh, xColor);
            ili9488_set_pixel_color(usX - sXh, usY + sYh, xColor);
            ili9488_set_pixel_color(usX + sXh, usY - sYh, xColor);
            ili9488_set_pixel_color(usX - sXh, usY - sYh, xColor);
            ili9488_set_pixel_color(usX + sYh, usY + sXh, xColor);
            ili9488_set_pixel_color(usX - sYh, usY + sXh, xColor);
            ili9488_set_pixel_color(usX + sYh, usY - sXh, xColor);
            ili9488_set_pixel_color(usX - sYh, usY - sXh, xColor);
        }
    }
}

void tft_draw_image(const image_t *pImage, uint16_t usX, uint16_t usY)
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
void tft_draw_bitmap(const uint8_t *pubBitmap, uint16_t usX, uint16_t usY, uint16_t usW, uint16_t usH, rgb565_t xColor, rgb565_t xBackColor)
{
    if(!pubBitmap)
        return;

    if(!ili9488_set_window(usX, usY, usX + usW - 1, usY + usH - 1))
        return;

    for(uint32_t ulI = 0; ulI < usW * usH; ulI++)
    {
        if(*(pubBitmap + (ulI / 8)) & (0x80 >> (ulI % 8)))
            ili9488_send_pixel_data(xColor);
        else
            ili9488_send_pixel_data(xBackColor);
    }
}

uint16_t tft_get_text_height(const font_t *pFont, uint16_t usNumLines)
{
    return (usNumLines * pFont->ubYAdvance) + pFont->ubLineOffset;
}
static uint8_t tft_search_char(char * ubStr, uint8_t ubChar)
{
    uint8_t ubCnt = 0;

    while(*ubStr++)
    {
        if(*ubStr == ubChar)
            ubCnt++;
    }

    return ubCnt;
}
static uint16_t tft_get_str_pix_len(const font_t *pFont, uint8_t *pubStr)
{
    uint16_t usLength = 0;

    while(*pubStr)
    {
        usLength += pFont->pGlyph[*pubStr - pFont->cFirstChar].ubXAdvance;
        pubStr++;
    }

    return usLength;
}

uint8_t tft_draw_char(char cChar, const font_t *pFont, uint16_t usX, uint16_t usY, rgb565_t xColor, rgb565_t xBackColor)
{
    if((cChar < pFont->cFirstChar) || (cChar > pFont->cLastChar))
        cChar = '?';

    cChar -= pFont->cFirstChar;

    tft_draw_bitmap(pFont->pubBitmap + pFont->pGlyph[cChar].usBitmapOffset, usX + pFont->pGlyph[cChar].bXOffset, usY + pFont->ubYAdvance + pFont->pGlyph[cChar].bYOffset - 1, pFont->pGlyph[cChar].ubWidth, pFont->pGlyph[cChar].ubHeight, xColor, xBackColor);

    return pFont->pGlyph[cChar].ubXAdvance;
}
void tft_draw_string(char *pszStr, const font_t *pFont, uint16_t usX, uint16_t usY, rgb565_t xColor, rgb565_t xBackColor)
{
    uint16_t usXh = usX;
    uint16_t usYh = usY;

    while(*pszStr)
    {
        if(*pszStr == '\n')
            usYh += pFont->ubYAdvance;
        else if(*pszStr == '\r')
            usXh = usX;
        else
            usXh += tft_draw_char(*pszStr, pFont, usXh, usYh, xColor, xBackColor);

        pszStr++;
    }
}
void tft_printf(const font_t *pFont, uint16_t usX, uint16_t usY, rgb565_t xColor, rgb565_t xBackColor, const char* pszFmt, ...)
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

	tft_draw_string(pszBuf, pFont, usX, usY, xColor, xBackColor);

    free(pszBuf);

	va_end(args);
}

tft_textbox_t *tft_textbox_create(uint16_t usX, uint16_t usY, uint16_t usNumLines, uint16_t usLenght, const font_t *pFont, rgb565_t xColor, rgb565_t xBackColor)
{
    tft_textbox_t *pNewTextbox = (tft_textbox_t *)malloc(sizeof(tft_textbox_t));

    if(!pNewTextbox)
        return NULL;

    memset(pNewTextbox, 0, sizeof(tft_textbox_t));

    pNewTextbox->usX = usX;
    pNewTextbox->usY = usY;
    pNewTextbox->usNumLines = usNumLines;
    pNewTextbox->usLen = usLenght;
    pNewTextbox->pFont = pFont;
    pNewTextbox->xColor = xColor;
    pNewTextbox->xBackColor = xBackColor;

    tft_textbox_clear(pNewTextbox);

    return pNewTextbox;
}
void tft_textbox_delete(tft_textbox_t *pTextbox)
{
    free(pTextbox);
}
void tft_textbox_set_color(tft_textbox_t *pTextbox, rgb565_t xColor, rgb565_t xBackColor)
{
    pTextbox->xColor = xColor;
    pTextbox->xBackColor = xBackColor;
}
void tft_textbox_clear(tft_textbox_t *pTextbox)
{
    tft_draw_rectangle(
        pTextbox->usX,
        pTextbox->usY,
        pTextbox->usX + pTextbox->usLen - 1,
        pTextbox->usY + tft_get_text_height(pTextbox->pFont, pTextbox->usNumLines) - 1,
        pTextbox->xBackColor,
        1
        );

    pTextbox->usCursor = pTextbox->usX;
    pTextbox->usCurrentLine = 0;
}
void tft_textbox_clear_line(tft_textbox_t *pTextbox)
{
    tft_draw_rectangle(
        pTextbox->usX,
        pTextbox->usY + (pTextbox->usCurrentLine * pTextbox->pFont->ubYAdvance) + ((!!pTextbox->usCurrentLine) * pTextbox->pFont->ubLineOffset),
        pTextbox->usX + pTextbox->usLen - 1,
        pTextbox->usY + ((pTextbox->usCurrentLine + 1) * pTextbox->pFont->ubYAdvance) + pTextbox->pFont->ubLineOffset - 1,
        pTextbox->xBackColor,
        1
        );

    pTextbox->usCursor = pTextbox->usX;
}
void tft_textbox_goto(tft_textbox_t *pTextbox, uint16_t usCursor, uint16_t usLine, uint8_t ubClearLine)
{
    usCursor += pTextbox->usX;

    if(usCursor > pTextbox->usLen + pTextbox->usX)
        usCursor = pTextbox->usLen + pTextbox->usX;

    if(usCursor < pTextbox->usX)
        usCursor = pTextbox->usX;

    if(usLine > pTextbox->usNumLines - 1)
        usLine = pTextbox->usNumLines - 1;

    pTextbox->usCursor = usCursor;
    pTextbox->usCurrentLine = usLine;

    if(ubClearLine)
        tft_textbox_clear_line(pTextbox);
}
void tft_textbox_draw_string(tft_textbox_t *pTextbox, char *pszStr)
{
    while(*pszStr)
    {
        if(*pszStr == '\n')
        {
            pTextbox->usCurrentLine++;

            if(pTextbox->usCurrentLine >= pTextbox->usNumLines)
                pTextbox->usCurrentLine = 0;
        }
        else if(*pszStr == '\r')
        {
            tft_textbox_clear_line(pTextbox);
        }
        else
        {
            if((pTextbox->usCursor + pTextbox->pFont->pGlyph[*pszStr - pTextbox->pFont->cFirstChar].ubXAdvance) >= (pTextbox->usLen + pTextbox->usX - 1))
            {
                pTextbox->usCurrentLine++;

                if(pTextbox->usCurrentLine == pTextbox->usNumLines)
                    pTextbox->usCurrentLine = 0;

                tft_textbox_clear_line(pTextbox);
            }

            pTextbox->usCursor += tft_draw_char(
                *pszStr,
                pTextbox->pFont,
                pTextbox->usCursor,
                pTextbox->usY + (pTextbox->usCurrentLine * pTextbox->pFont->ubYAdvance),
                pTextbox->xColor,
                pTextbox->xBackColor
                );
        }

        pszStr++;
    }
}
void tft_textbox_printf(tft_textbox_t *pTextbox, const char* pszFmt, ...)
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

	tft_textbox_draw_string(pTextbox, pszBuf);

    free(pszBuf);

	va_end(args);
}

tft_terminal_t *tft_terminal_create(uint16_t usX, uint16_t usY, uint16_t usNumLines, uint16_t usLenght, const font_t *pFont, rgb565_t xColor, rgb565_t xBackColor)
{
    tft_terminal_t *pNewTerminal = (tft_terminal_t *)malloc(sizeof(tft_terminal_t));

    if(!pNewTerminal)
        return NULL;

    memset(pNewTerminal, 0, sizeof(tft_terminal_t));

    pNewTerminal->pTextbox = (tft_textbox_t *)malloc(sizeof(tft_textbox_t));

    if(!pNewTerminal->pTextbox)
    {
        free(pNewTerminal);

        return NULL;
    }

    memset(pNewTerminal->pTextbox, 0, sizeof(tft_textbox_t));

    pNewTerminal->ppszBuf = (char **)malloc(usNumLines * sizeof(char *));

    if(!pNewTerminal->ppszBuf)
    {
        free(pNewTerminal->pTextbox);
        free(pNewTerminal);

        return NULL;
    }

    memset(pNewTerminal->ppszBuf, 0, usNumLines * sizeof(char *));

    pNewTerminal->pTextbox->pFont = pFont;
    pNewTerminal->pTextbox->usCurrentLine = usNumLines;
    pNewTerminal->pTextbox->usCursor = usX;
    pNewTerminal->pTextbox->usLen = usLenght;
    pNewTerminal->pTextbox->usNumLines = usNumLines;
    pNewTerminal->pTextbox->usX = usX;
    pNewTerminal->pTextbox->usY = usY;
    pNewTerminal->pTextbox->xBackColor = xBackColor;
    pNewTerminal->pTextbox->xColor = xColor;

    tft_textbox_clear(pNewTerminal->pTextbox);

    return pNewTerminal;
}
void tft_terminal_delete(tft_terminal_t *pTerminal)
{
    for(uint16_t usI = 0; usI < pTerminal->pTextbox->usNumLines; usI++)
        free(pTerminal->ppszBuf[usI]);

    free(pTerminal->ppszBuf);
    free(pTerminal->pTextbox);
    free(pTerminal);
}
static void tft_terminal_scroll(tft_terminal_t *pTerminal, uint16_t usNumLines)
{
    while(usNumLines--)
    {
        free(pTerminal->ppszBuf[0]);

        for(uint8_t ubI = 0; ubI < pTerminal->pTextbox->usNumLines - 1; ubI++)
            pTerminal->ppszBuf[ubI] = pTerminal->ppszBuf[ubI + 1];

        pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1] = NULL;
    }
}
void tft_terminal_draw_string(tft_terminal_t *pTerminal, char *pszStr)
{
    uint8_t ubMaxStrLen = strlen(pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1]) + strlen(pszStr);

    char *pszTempStr = (char *)malloc(ubMaxStrLen + 1);

    if(!pszTempStr)
        return;

    memset(pszTempStr, 0, ubMaxStrLen + 1);

    uint8_t ubStrLen = snprintf(pszTempStr, ubMaxStrLen + 1, pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1]);

    free(pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1]);

    while(*pszStr)
    {
        if(*pszStr == '\n')
        {
            pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1] = (char *)malloc(ubStrLen + 1);

            if(!pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1])
            {
                free(pszTempStr);

                return;
            }

            snprintf(pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1], ubStrLen + 1, pszTempStr);

            tft_terminal_scroll(pTerminal, 1);

            memset(pszTempStr, 0, ubMaxStrLen + 1);

            ubStrLen = 0;
        }
        else if(*pszStr == '\r')
        {
            memset(pszTempStr, 0, ubMaxStrLen + 1);

            ubStrLen = 0;
        }
        else
        {
            if(tft_get_str_pix_len(pTerminal->pTextbox->pFont, pszTempStr) + pTerminal->pTextbox->pFont->pGlyph[*pszStr - pTerminal->pTextbox->pFont->cFirstChar].ubXAdvance >= pTerminal->pTextbox->usLen + pTerminal->pTextbox->usX - 1)
            {
                pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1] = (char *)malloc(ubStrLen + 1);

                if(!pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1])
                {
                    free(pszTempStr);

                    return;
                }

                snprintf(pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1], ubStrLen + 1, pszTempStr);

                tft_terminal_scroll(pTerminal, 1);

                memset(pszTempStr, 0, ubMaxStrLen + 1);

                ubStrLen = 0;
            }

            *(pszTempStr + ubStrLen) = *pszStr;

            ubStrLen++;
        }

        pszStr++;
    }

    pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1] = (char *)malloc(ubStrLen + 1);

    if(!pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1])
    {
        free(pszTempStr);

        return;
    }

    snprintf(pTerminal->ppszBuf[pTerminal->pTextbox->usNumLines - 1], ubStrLen + 1, pszTempStr);

    free(pszTempStr);
}
void tft_terminal_update(tft_terminal_t *pTerminal)
{
    for(uint8_t ubI = 0; ubI < pTerminal->pTextbox->usNumLines; ubI++)
    {
        tft_textbox_goto(pTerminal->pTextbox, 0, ubI, 1);

        tft_textbox_draw_string(pTerminal->pTextbox, pTerminal->ppszBuf[ubI]);
    }
}
void tft_terminal_clear(tft_terminal_t *pTerminal)
{
    for(uint8_t ubI = 0; ubI < pTerminal->pTextbox->usNumLines; ubI++)
    {
        free(pTerminal->ppszBuf[ubI]);

        pTerminal->ppszBuf[ubI] = NULL;
    }

    tft_textbox_clear(pTerminal->pTextbox);
}
void tft_terminal_printf(tft_terminal_t *pTerminal, uint8_t ubUpdate, const char* pszFmt, ...)
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

	tft_terminal_draw_string(pTerminal, pszBuf);

    free(pszBuf);

	va_end(args);

    if(ubUpdate)
        tft_terminal_update(pTerminal);
}

// TODO: draw_line_graph()
// TODO: draw_bar_graph()
// TODO: draw_heat_map()
// TODO: draw_radar_spider_chart()
// TODO: draw_scatter_plot()
// TODO: gantt chart