#ifndef __GPIO_H__
#define __GPIO_H__

#include <em_device.h>
#include "utils.h"

// LED MACROS
#define LED_HIGH()      PERI_REG_BIT_SET(&(GPIO->P[1].DOUT)) = BIT(0)
#define LED_LOW()       PERI_REG_BIT_CLEAR(&(GPIO->P[1].DOUT)) = BIT(0)
#define LED_TOGGLE()    GPIO->P[1].DOUTTGL = BIT(0);

// BUZZER MACROS
#define BUZZ_ON()        PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(15)
#define BUZZ_OFF()       PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(15)
#define BUZZ_TOGGLE()    GPIO->P[0].DOUTTGL = BIT(15);

// RFM MACROS
#define RFM_RESET()         PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(4)
#define RFM_UNRESET()       PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(4)
#define RFM_SELECT()        PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(3)
#define RFM_UNSELECT()      PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(3)
#define RFM_IRQ()           PERI_REG_BIT(&(GPIO->P[0].DIN, 6)

// GSM MACROS
// TODO

// TFT MACROS
#define ILI9488_RESET()     PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(12)
#define ILI9488_UNRESET()   PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(12)
#define ILI9488_SELECT()    PERI_REG_BIT_CLEAR(&(GPIO->P[3].DOUT)) = BIT(3)
#define ILI9488_UNSELECT()  PERI_REG_BIT_SET(&(GPIO->P[3].DOUT)) = BIT(3)
#define ILI9488_IRQ()       PERI_REG_BIT(&(GPIO->P[0].DIN, 13)
#define ILI9488_SETUP_DAT() PERI_REG_BIT_SET(&(GPIO->P[0].DOUT)) = BIT(14)
#define ILI9488_SETUP_CMD() PERI_REG_BIT_CLEAR(&(GPIO->P[0].DOUT)) = BIT(14)
#define TFT_BL_ON()         PERI_REG_BIT_SET(&(GPIO->P[1].DOUT)) = BIT(10)
#define TFT_BL_OFF()        PERI_REG_BIT_CLEAR(&(GPIO->P[1].DOUT)) = BIT(10)

// BUTTON MACROS
#define BTN_1_STATE()       PERI_REG_BIT(&(GPIO->P[1].DIN, 1)
#define BTN_2_STATE()       PERI_REG_BIT(&(GPIO->P[1].DIN, 2)
#define BTN_3_STATE()       PERI_REG_BIT(&(GPIO->P[1].DIN, 3)

// BAT / POW MACROS
#define BAT_STDBY()         PERI_REG_BIT(&(GPIO->P[2].DIN, 6)
#define BAT_CHRG()          PERI_REG_BIT(&(GPIO->P[2].DIN, 7)
#define VREG_ERR()          PERI_REG_BIT(&(GPIO->P[5].DIN, 11)
#define VIN_DETECT()        PERI_REG_BIT(&(GPIO->P[5].DIN, 12)

// WIFI COPROCESSOR MACROS
#define WIFI_SELECT()       PERI_REG_BIT_CLEAR(&(GPIO->P[2].DOUT)) = BIT(8)
#define WIFI_UNSELECT()     PERI_REG_BIT_SET(&(GPIO->P[2].DOUT)) = BIT(8)
#define WIFI_IRQ()          PERI_REG_BIT(&(GPIO->P[4].DIN, 3)
#define WIFI_RESET()        PERI_REG_BIT_CLEAR(&(GPIO->P[4].DOUT)) = BIT(4)
#define WIFI_UNRESET()      PERI_REG_BIT_SET(&(GPIO->P[4].DOUT)) = BIT(4)

// CCS811 MACROS
#define CCS811_RESET()      PERI_REG_BIT_CLEAR(&(GPIO->P[4].DOUT)) = BIT(11)
#define CCS811_UNRESET()    PERI_REG_BIT_SET(&(GPIO->P[4].DOUT)) = BIT(11)
#define CCS811_WAKE_SET()   PERI_REG_BIT_SET(&(GPIO->P[4].DOUT)) = BIT(10)
#define CCS811_WAKE_CLR()   PERI_REG_BIT_CLEAR(&(GPIO->P[4].DOUT)) = BIT(10)
#define CCS811_IRQ()        PERI_REG_BIT(&(GPIO->P[4].DIN, 9)

// MAG MACROS
#define MAG_STATE()         PERI_REG_BIT(&(GPIO->P[4].DIN, 15)

void gpio_init();

#endif  // __GPIO_H__
