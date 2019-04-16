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
static inline void ili9488_send_pixel_data(rgb565_t usColor)
{
    ILI9488_SELECT();
    ILI9488_SETUP_DAT();
    usart1_spi_transfer_byte(RGB565_EXTRACT_RED(usColor));
    usart1_spi_transfer_byte(RGB565_EXTRACT_GREEN(usColor));
    usart1_spi_transfer_byte(RGB565_EXTRACT_BLUE(usColor));
    ILI9488_UNSELECT();
}
static inline void ili9488_set_window(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1)
{
    

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
uint32_t ili9488_read_id()
{
    uint8_t ubBuf[3];
    ili9488_send_cmd(ILI9488_RD_DISP_ID, NULL, 0);
    ili9488_read_data(ubBuf, 3);
    return ((uint32_t)ubBuf[0] << 16) | ((uint32_t)ubBuf[1] << 8) | (uint32_t)ubBuf[2];
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
            usMaxWidth = ILI9488_TFTWIDTH;
            usMaxHeigth = ILI9488_TFTHEIGHT;
            break;
        case 1:
            ubBuf = ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR;
            usMaxWidth = ILI9488_TFTHEIGHT;
            usMaxHeigth = ILI9488_TFTWIDTH;
            break;
        case 2:
            ubBuf = ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR;
            usMaxWidth = ILI9488_TFTWIDTH;
            usMaxHeigth = ILI9488_TFTHEIGHT;
            break;
        case 3:
            ubBuf = ILI9488_MADCTL_MX | ILI9488_MADCTL_MY | ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR;
            usMaxWidth = ILI9488_TFTHEIGHT;
            usMaxHeigth = ILI9488_TFTHEIGHT;
            break;
        default:
            return;
            break;
    }
    ili9488_send_cmd(ILI9488_MEM_A_CTL, &ubBuf, 1);
}
void ili9488_set_invert(uint8_t ubOnOff)
{
    ili9488_send_cmd((ubOnOff ? ILI9488_INV_ON : ILI9488_INV_OFF), NULL, 0);
}

void ili9488_set_scroll_area(uint8_t ubRotation, uint16_t usTopFixedArea, uint16_t usBottomFixedArea)
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

void ili9488_fill_screen(uint8_t ubRotation, rgb565_t usColor)
{
    ili9488_set_window(0, 0, usMaxWidth, usMaxHeigth);

    for(uint32_t ulI = ILI9488_TFTWIDTH * ILI9488_TFTHEIGHT; ulI > 0; ulI--)
    {
        ili9488_send_pixel_data(usColor);
    }
}

void ili9488_draw_pixel(uint16_t usX, uint16_t usY, rgb565_t usColor)
{
    ili9488_set_window(usX, usY, usX + 1, usY + 1);
    ili9488_send_pixel_data(usColor);
}
void ili9488_draw_Fast_v_line(uint16_t usX, uint16_t usY0, uint16_t usY1, rgb565_t usColor)
{
    if(usY0 > usY1)
        SWAP(usY0, usY1);

    ili9488_set_window(usX, usY0, usX, usY1);
    for(uint16_t usH = usY1 - usY0; usH > 0; usH--)
    {
        ili9488_send_pixel_data(usColor);
    }
}
void ili9488_draw_Fast_h_line(uint16_t usX0, uint16_t usY, uint16_t usX1, rgb565_t usColor)
{
    if(usX0 > usX1)
        SWAP(usX0, usX1);

    ili9488_set_window(usX0, usY, usX1, usY);
    for(uint16_t usW = usX1 - usX0; usW > 0; usW--)
    {
        ili9488_send_pixel_data(usColor);
    }
}
void ili9488_draw_line(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1, rgb565_t usColor)
{
    if(usX0 == usX1)
    {
        ili9488_draw_Fast_v_line(usX0, usY0, usY1, usColor);
        return;
    }
    if(usY0 == usY1)
    {
        ili9488_draw_Fast_h_line(usX0, usY0, usX1, usColor);
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
			ili9488_draw_pixel(usY0, usX0, usColor);
        else
			ili9488_draw_pixel(usX0, usY0, usColor);

		bErr -= ubdy;

		if(bErr < 0)
		{
			usY0 += bYstep;
			bErr += ubdx;
		}
	}
}
void ili9488_draw_rectangle(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1, rgb565_t usColor, uint8_t ubFill)
{
    if(usX0 > usX1)
        SWAP(usX0, usX1);

    if(usY0 > usY1)
        SWAP(usY0, usY1);

    if(ubFill)
    {
        ili9488_set_window(usX0, usY0, usX1, usY1);
        for(uint16_t usH = usY1 - usY0; usH > 0; usH--)
        {
            for(uint16_t usW = usX1 - usX0; usW > 0; usW--)
            {
                ili9488_send_pixel_data(usColor);
            }
        }
    }
    else
    {
        ili9488_draw_Fast_h_line(usX0, usY0, usX1, usColor);
        ili9488_draw_Fast_v_line(usX1, usY0, usY1, usColor);
        ili9488_draw_Fast_v_line(usX0, usY0, usY1, usColor);
        ili9488_draw_Fast_h_line(usX0, usY1, usX1, usColor);
    }
}
void ili9488_draw_circle(uint16_t usX, uint16_t usY, uint16_t usR, uint8_t ubFill, rgb565_t usColor)
{
	int16_t sF = 1 - usR;
	int16_t sDdFx = 1;
	int16_t sDdFy = -2 * usR;
	int16_t sXh = 0;
	int16_t sYh = usR;

    if(ubFill)
    {
        for(int16_t usI = usY - usR; usI <= usY + usR; usI++)
            ili9488_draw_pixel(usX, usI, usColor);
    }
    else
    {
        ili9488_draw_pixel(usX, usY + usR, usColor);
        ili9488_draw_pixel(usX, usY - usR, usColor);
        ili9488_draw_pixel(usX + usR, usY, usColor);
        ili9488_draw_pixel(usX - usR, usY, usColor);
    }

    while(usX < usY)
    {
        if(sF >= 0)
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
                ili9488_draw_pixel(usX + sXh, usI, usColor);
                ili9488_draw_pixel(usX - sXh, usI, usColor);
            }
            for(int16_t usI = usY - sXh; usI <= usY + sXh; usI++)
            {
                ili9488_draw_pixel(usX + sYh, usI, usColor);
                ili9488_draw_pixel(usX - sYh, usI, usColor);
            }
        }
        else
        {
            ili9488_draw_pixel(usX + sXh, usY + sYh, usColor);
            ili9488_draw_pixel(usX - sXh, usY + sYh, usColor);
            ili9488_draw_pixel(usX + sXh, usY - sYh, usColor);
            ili9488_draw_pixel(usX - sXh, usY - sYh, usColor);

            ili9488_draw_pixel(usX + sYh, usY + sXh, usColor);
            ili9488_draw_pixel(usX - sYh, usY + sXh, usColor);
            ili9488_draw_pixel(usX + sYh, usY - sXh, usColor);
            ili9488_draw_pixel(usX - sYh, usY - sXh, usColor);
        }
    }
}

void ili9488_draw_image(const rgb565_t *pusImgBuf, uint16_t usX, uint16_t usY)
{
    if(!pusImgBuf)
        return;

    ili9488_set_window(usX, usY, usX + pusImgBuf[0] - 1, usY + pusImgBuf[1] - 1);

    uint32_t ulImgSize = pusImgBuf[0] * pusImgBuf[1];

    while(ulImgSize--)
        ili9488_send_pixel_data(*pusImgBuf++);
}

// ili9488_draw_line_graph()
// ili9488_draw_bar_graph()
// draw_heat_map()
// draw_radar_spider_chart()
// draw_scatter_plot()
// gantt chart