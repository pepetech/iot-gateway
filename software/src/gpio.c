#include "gpio.h"

void gpio_init()
{
    CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_GPIO;

    // Port A
    GPIO->P[0].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[0].MODEL  = GPIO_P_MODEL_MODE0_DISABLED     // US3_MOSI_RFM
                      | GPIO_P_MODEL_MODE1_DISABLED     // US3_MISO_RFM
                      | GPIO_P_MODEL_MODE2_DISABLED     // US3_CLK_RFM
                      | GPIO_P_MODEL_MODE3_DISABLED     // US3_CS_RFM
                      | GPIO_P_MODEL_MODE4_DISABLED     // RFM_RESET
                      | GPIO_P_MODEL_MODE5_DISABLED     // NR
                      | GPIO_P_MODEL_MODE6_DISABLED     // RFM_IRQ
                      | GPIO_P_MODEL_MODE7_DISABLED;    // GSM_STATUS
    GPIO->P[0].MODEH  = GPIO_P_MODEH_MODE8_DISABLED     // GSM_RF_SYNC
                      | GPIO_P_MODEH_MODE9_DISABLED     // GSM_PWR_KEY
                      | GPIO_P_MODEH_MODE10_DISABLED    // GSM_LED_EN
                      | GPIO_P_MODEH_MODE11_DISABLED    // NR
                      | GPIO_P_MODEH_MODE12_PUSHPULL    // TFT_RESET
                      | GPIO_P_MODEH_MODE13_DISABLED    // TFT_IRQ
                      | GPIO_P_MODEH_MODE14_DISABLED    // TFT_DC
                      | GPIO_P_MODEH_MODE15_DISABLED;   // TIM3_CC2_BUZZ
    GPIO->P[0].DOUT   = 0;
    GPIO->P[0].OVTDIS = 0;

    // Port BEL_MO
EL_MO
EL_MO
    GPIO->P[1].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (7 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (7 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[1].MODEL  = GPIO_P_MODEL_MODE0_DISABLED     // LED_DI
                      | GPIO_P_MODEL_MODE1_DISABLED     // BTN_1
                      | GPIO_P_MODEL_MODE2_DISABLED     // BTN_2
                      | GPIO_P_MODEL_MODE3_DISABLED     // BTN_3
                      | GPIO_P_MODEL_MODE4_DISABLED     // NR
                      | GPIO_P_MODEL_MODE5_DISABLED     // NR
                      | GPIO_P_MODEL_MODE6_DISABLED     // NR
                      | GPIO_P_MODEL_MODE7_DISABLED;    // MAIN_LFXTAL_P
    GPIO->P[1].MODEH  = GPIO_P_MODEH_MODE8_DISABLED     // MAIN_LFXTAL_N
                      | GPIO_P_MODEH_MODE9_DISABLED     // NR
                      | GPIO_P_MODEH_MODE10_DISABLED    // WTIM2_CC1_TFT_BL
                      | GPIO_P_MODEH_MODE11_DISABLED    // I2C1_SDA_TFT
                      | GPIO_P_MODEH_MODE12_DISABLED    // I2C1_SCL_TFT
                      | GPIO_P_MODEH_MODE13_DISABLED    // MAIN_HFXTAL_P
                      | GPIO_P_MODEH_MODE14_DISABLED    // MAIN_HFXTAL_N
                      | GPIO_P_MODEH_MODE15_DISABLED;   // NC
    GPIO->P[1].DOUT   = 0;
    GPIO->P[1].OVTDIS = 0;

    // Port C
    GPIO->P[2].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[2].MODEL  = GPIO_P_MODEL_MODE0_DISABLED     // US2_RTS_GSM
                      | GPIO_P_MODEL_MODE1_DISABLED     // US2_CTS_GSM
                      | GPIO_P_MODEL_MODE2_DISABLED     // US2_TX_GSM
                      | GPIO_P_MODEL_MODE3_DISABLED     // US2_RX_GSM
                      | GPIO_P_MODEL_MODE4_DISABLED     // GSM_RING
                      | GPIO_P_MODEL_MODE5_DISABLED     // GSM_DTR
                      | GPIO_P_MODEL_MODE6_DISABLED     // BAT_STDBY
                      | GPIO_P_MODEL_MODE7_DISABLED;    // BAT_CHRG
    GPIO->P[2].MODEH  = GPIO_P_MODEH_MODE8_PUSHPULL     // US0_CS_WIFI
                      | GPIO_P_MODEH_MODE9_DISABLED     // US0_CLK_WIFI
                      | GPIO_P_MODEH_MODE10_DISABLED    // US0_MISO_WIFI
                      | GPIO_P_MODEH_MODE11_DISABLED    // US0_MOSI_WIFI
                      | GPIO_P_MODEH_MODE12_DISABLED    // NC
                      | GPIO_P_MODEH_MODE13_DISABLED    // NC
                      | GPIO_P_MODEH_MODE14_DISABLED    // NC
                      | GPIO_P_MODEH_MODE15_DISABLED;   // NC
    GPIO->P[2].DOUT   = 0;
    GPIO->P[2].OVTDIS = 0;

    // Port D
    GPIO->P[3].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[3].MODEL  = GPIO_P_MODEL_MODE0_DISABLED     // US1_MOSI_TFT
                      | GPIO_P_MODEL_MODE1_DISABLED     // US1_MISO_TFT
                      | GPIO_P_MODEL_MODE2_DISABLED     // US1_CLK_TFT
                      | GPIO_P_MODEL_MODE3_DISABLED     // US1_CS_TFT
                      | GPIO_P_MODEL_MODE4_DISABLED     // 5V0_SENSE
                      | GPIO_P_MODEL_MODE5_DISABLED     // 4V2_SENSE
                      | GPIO_P_MODEL_MODE6_DISABLED     // 
                      | GPIO_P_MODEL_MODE7_DISABLED;
    GPIO->P[3].MODEH  = GPIO_P_MODEH_MODE8_DISABLED
                      | GPIO_P_MODEH_MODE9_PUSHPULL  // QSPI0_DQ0 - Location 0
                      | GPIO_P_MODEH_MODE10_PUSHPULL // QSPI0_DQ1 - Location 0
                      | GPIO_P_MODEH_MODE11_PUSHPULL // QSPI0_DQ2 - Location 0
                      | GPIO_P_MODEH_MODE12_PUSHPULL // QSPI0_DQ3 - Location 0
                      | GPIO_P_MODEH_MODE13_DISABLED
                      | GPIO_P_MODEH_MODE14_DISABLED
                      | GPIO_P_MODEH_MODE15_DISABLED;
    GPIO->P[3].DOUT   = 0;
    GPIO->P[3].OVTDIS = 0;

    // Port E
    GPIO->P[4].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[4].MODEL  = GPIO_P_MODEL_MODE0_DISABLED
                      | GPIO_P_MODEL_MODE1_DISABLED
                      | GPIO_P_MODEL_MODE2_DISABLED
                      | GPIO_P_MODEL_MODE3_DISABLED
                      | GPIO_P_MODEL_MODE4_PUSHPULL  // WIFI_EN
                      | GPIO_P_MODEL_MODE5_DISABLED
                      | GPIO_P_MODEL_MODE6_DISABLED
                      | GPIO_P_MODEL_MODE7_DISABLED;
    GPIO->P[4].MODEH  = GPIO_P_MODEH_MODE8_DISABLED
                      | GPIO_P_MODEH_MODE9_DISABLED
                      | GPIO_P_MODEH_MODE10_DISABLED
                      | GPIO_P_MODEH_MODE11_DISABLED
                      | GPIO_P_MODEH_MODE12_DISABLED
                      | GPIO_P_MODEH_MODE13_DISABLED
                      | GPIO_P_MODEH_MODE14_DISABLED
                      | GPIO_P_MODEH_MODE15_DISABLED;
    GPIO->P[4].DOUT   = BIT(4);
    GPIO->P[4].OVTDIS = 0;

    // Port F
    GPIO->P[5].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[5].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL  // DBG_SWCLK - Location 0
                      | GPIO_P_MODEL_MODE1_PUSHPULL  // DBG_SWDIO - Location 0
                      | GPIO_P_MODEL_MODE2_PUSHPULL  // DBG_SWO - Location 0
                      | GPIO_P_MODEL_MODE3_DISABLED
                      | GPIO_P_MODEL_MODE4_DISABLED
                      | GPIO_P_MODEL_MODE5_DISABLED
                      | GPIO_P_MODEL_MODE6_PUSHPULL  // QSPI0_SCLK - Location 0
                      | GPIO_P_MODEL_MODE7_PUSHPULL; // QSPI0_CS0 - Location 0
    GPIO->P[5].MODEH  = GPIO_P_MODEH_MODE8_DISABLED
                      | GPIO_P_MODEH_MODE9_DISABLED
                      | GPIO_P_MODEH_MODE10_DISABLED
                      | GPIO_P_MODEH_MODE11_DISABLED
                      | GPIO_P_MODEH_MODE12_DISABLED
                      | GPIO_P_MODEH_MODE13_DISABLED
                      | GPIO_P_MODEH_MODE14_DISABLED
                      | GPIO_P_MODEH_MODE15_DISABLED;
    GPIO->P[5].DOUT   = BIT(7);
    GPIO->P[5].OVTDIS = 0;

    // Debugger Route
    GPIO->ROUTEPEN &= ~(GPIO_ROUTEPEN_TDIPEN | GPIO_ROUTEPEN_TDOPEN); // Disable JTAG
    GPIO->ROUTEPEN |= GPIO_ROUTEPEN_SWVPEN; // Enable SWO
    GPIO->ROUTELOC0 = GPIO_ROUTELOC0_SWVLOC_LOC0; // SWO on PF2
}