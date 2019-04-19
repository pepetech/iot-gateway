#include "gpio.h"

void gpio_init()
{
    CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_GPIO;

    // NC - Not Connected (not available in mcu package)
    // NR - Not routed (no routing to pin on pcb, floating)

    // Port A
    GPIO->P[0].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (6 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (6 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[0].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL                 // US3_MOSI_RFM - Location 0
                      | GPIO_P_MODEL_MODE1_INPUTPULLFILTER          // US3_MISO_RFM - Location 0
                      | GPIO_P_MODEL_MODE2_PUSHPULL                 // US3_CLK_RFM - Location 0
                      | GPIO_P_MODEL_MODE3_PUSHPULL                 // US3_CS_RFM - Location 0
                      | GPIO_P_MODEL_MODE4_PUSHPULL                 // RFM_RESET
                      | GPIO_P_MODEL_MODE5_DISABLED                 // NR
                      | GPIO_P_MODEL_MODE6_INPUTPULLFILTER          // RFM_IRQ
                      | GPIO_P_MODEL_MODE7_INPUTPULLFILTER;         // GSM_STATUS
    GPIO->P[0].MODEH  = GPIO_P_MODEH_MODE8_INPUTPULLFILTER          // GSM_RF_SYNC
                      | GPIO_P_MODEH_MODE9_PUSHPULL                 // GSM_PWR_KEY
                      | GPIO_P_MODEH_MODE10_PUSHPULL                // GSM_LED_EN
                      | GPIO_P_MODEH_MODE11_DISABLED                // NR
                      | GPIO_P_MODEH_MODE12_PUSHPULL                // TFT_RESET
                      | GPIO_P_MODEH_MODE13_INPUTPULLFILTER         // TFT_IRQ
                      | GPIO_P_MODEH_MODE14_PUSHPULL                // TFT_DC
                      | GPIO_P_MODEH_MODE15_PUSHPULL;               // TIM3_CC2_BUZZ - Location 0
    GPIO->P[0].DOUT   = BIT(3) | BIT(10) | BIT(12) | BIT(13);
    GPIO->P[0].OVTDIS = 0;

    // Port B
    GPIO->P[1].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (6 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (6 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[1].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL                 // LED_DI
                      | GPIO_P_MODEL_MODE1_INPUTPULLFILTER          // BTN_1
                      | GPIO_P_MODEL_MODE2_INPUTPULLFILTER          // BTN_2
                      | GPIO_P_MODEL_MODE3_INPUTPULLFILTER          // BTN_3
                      | GPIO_P_MODEL_MODE4_DISABLED                 // NR
                      | GPIO_P_MODEL_MODE5_DISABLED                 // NR
                      | GPIO_P_MODEL_MODE6_DISABLED                 // NR
                      | GPIO_P_MODEL_MODE7_DISABLED;                // MAIN_LFXTAL_P
    GPIO->P[1].MODEH  = GPIO_P_MODEH_MODE8_DISABLED                 // MAIN_LFXTAL_N
                      | GPIO_P_MODEH_MODE9_DISABLED                 // NR
                      | GPIO_P_MODEH_MODE10_PUSHPULL                // WTIM2_CC1_TFT_BL - Location 2
                      | GPIO_P_MODEH_MODE11_WIREDANDPULLUPFILTER    // I2C1_SDA_TFT - Location 1
                      | GPIO_P_MODEH_MODE12_WIREDANDPULLUPFILTER    // I2C1_SCL_TFT - Location 1
                      | GPIO_P_MODEH_MODE13_DISABLED                // MAIN_HFXTAL_P
                      | GPIO_P_MODEH_MODE14_DISABLED                // MAIN_HFXTAL_N
                      | GPIO_P_MODEH_MODE15_DISABLED;               // NC
    GPIO->P[1].DOUT   = BIT(0) | BIT(1) | BIT(2) | BIT(3);
    GPIO->P[1].OVTDIS = 0;

    // Port C
    GPIO->P[2].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (6 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (7 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[2].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL                 // US2_RTS_GSM - Location 0
                      | GPIO_P_MODEL_MODE1_INPUTPULLFILTER          // US2_CTS_GSM - Location 0
                      | GPIO_P_MODEL_MODE2_PUSHPULL                 // US2_TX_GSM - Location 0
                      | GPIO_P_MODEL_MODE3_INPUTPULLFILTER          // US2_RX_GSM - Location 0
                      | GPIO_P_MODEL_MODE4_INPUTPULLFILTER          // GSM_RING
                      | GPIO_P_MODEL_MODE5_PUSHPULL                 // GSM_DTR
                      | GPIO_P_MODEL_MODE6_INPUTPULLFILTER          // BAT_STDBY
                      | GPIO_P_MODEL_MODE7_INPUTPULLFILTER;         // BAT_CHRG
    GPIO->P[2].MODEH  = GPIO_P_MODEH_MODE8_PUSHPULL                 // US0_CS_WIFI - Location 2
                      | GPIO_P_MODEH_MODE9_PUSHPULL                 // US0_CLK_WIFI - Location 2
                      | GPIO_P_MODEH_MODE10_INPUTPULLFILTER         // US0_MISO_WIFI - Location 2
                      | GPIO_P_MODEH_MODE11_PUSHPULL                // US0_MOSI_WIFI - Location 2
                      | GPIO_P_MODEH_MODE12_DISABLED                // NC
                      | GPIO_P_MODEH_MODE13_DISABLED                // NC
                      | GPIO_P_MODEH_MODE14_DISABLED                // NC
                      | GPIO_P_MODEH_MODE15_DISABLED;               // NC
    GPIO->P[2].DOUT   = BIT(0) | BIT(1) | BIT(3) | BIT(4) | BIT(5) | BIT(6) | BIT(7) | BIT(8);
    GPIO->P[2].OVTDIS = 0;

    // Port D
    GPIO->P[3].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (6 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (6 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[3].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL                 // US1_MOSI_TFT - Location 1
                      | GPIO_P_MODEL_MODE1_INPUTPULLFILTER          // US1_MISO_TFT - Location 1
                      | GPIO_P_MODEL_MODE2_PUSHPULL                 // US1_CLK_TFT - Location 1
                      | GPIO_P_MODEL_MODE3_PUSHPULL                 // US1_CS_TFT - Location 1
                      | GPIO_P_MODEL_MODE4_DISABLED                 // 5V0_SENSE
                      | GPIO_P_MODEL_MODE5_DISABLED                 // 4V2_SENSE
                      | GPIO_P_MODEL_MODE6_DISABLED                 // VBAT_SENSE
                      | GPIO_P_MODEL_MODE7_DISABLED;                // VIN_SENSE
    GPIO->P[3].MODEH  = GPIO_P_MODEH_MODE8_DISABLED                 // NR
                      | GPIO_P_MODEH_MODE9_PUSHPULL                 // QSPI0_DQ0 - Location 0
                      | GPIO_P_MODEH_MODE10_PUSHPULL                // QSPI0_DQ1 - Location 0
                      | GPIO_P_MODEH_MODE11_PUSHPULL                // QSPI0_DQ2 - Location 0
                      | GPIO_P_MODEH_MODE12_PUSHPULL                // QSPI0_DQ3 - Location 0
                      | GPIO_P_MODEH_MODE13_DISABLED                // NC
                      | GPIO_P_MODEH_MODE14_DISABLED                // NC
                      | GPIO_P_MODEH_MODE15_DISABLED;               // NC
    GPIO->P[3].DOUT   = BIT(3);
    GPIO->P[3].OVTDIS = BIT(4) | BIT(5) | BIT(6) | BIT(7);

    // Port E
    GPIO->P[4].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (6 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (6 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[4].MODEL  = GPIO_P_MODEL_MODE0_DISABLED                 // NR
                      | GPIO_P_MODEL_MODE1_DISABLED                 // NR
                      | GPIO_P_MODEL_MODE2_DISABLED                 // NR
                      | GPIO_P_MODEL_MODE3_INPUTPULLFILTER          // WIFI_IRQ
                      | GPIO_P_MODEL_MODE4_PUSHPULL                 // WIFI_EN
                      | GPIO_P_MODEL_MODE5_DISABLED                 // NR
                      | GPIO_P_MODEL_MODE6_DISABLED                 // NR
                      | GPIO_P_MODEL_MODE7_DISABLED;                // NR
    GPIO->P[4].MODEH  = GPIO_P_MODEH_MODE8_DISABLED                 // NR
                      | GPIO_P_MODEH_MODE9_INPUTPULLFILTER          // CCS811_IRQ
                      | GPIO_P_MODEH_MODE10_PUSHPULL                // CCS811_WAKE
                      | GPIO_P_MODEH_MODE11_PUSHPULL                // CCS811_RESET
                      | GPIO_P_MODEH_MODE12_WIREDANDPULLUPFILTER    // I2C0_SDA_SENS - Location 6
                      | GPIO_P_MODEH_MODE13_WIREDANDPULLUPFILTER    // I2C0_SCL_SENS - Location 6
                      | GPIO_P_MODEH_MODE14_DISABLED                // NR
                      | GPIO_P_MODEH_MODE15_INPUTPULLFILTER;        // MAG_ALERT
    GPIO->P[4].DOUT   = BIT(9) | BIT(10) | BIT(11);
    GPIO->P[4].OVTDIS = 0;

    // Port F
    GPIO->P[5].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (6 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (6 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[5].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL                 // DBG_SWCLK - Location 0
                      | GPIO_P_MODEL_MODE1_PUSHPULL                 // DBG_SWDIO - Location 0
                      | GPIO_P_MODEL_MODE2_PUSHPULL                 // DBG_SWO - Location 0
                      | GPIO_P_MODEL_MODE3_DISABLED                 // NC
                      | GPIO_P_MODEL_MODE4_DISABLED                 // NC
                      | GPIO_P_MODEL_MODE5_DISABLED                 // NR
                      | GPIO_P_MODEL_MODE6_PUSHPULL                 // QSPI0_SCLK - Location 0
                      | GPIO_P_MODEL_MODE7_PUSHPULL;                // QSPI0_CS0 - Location 0
    GPIO->P[5].MODEH  = GPIO_P_MODEH_MODE8_DISABLED                 // NR
                      | GPIO_P_MODEH_MODE9_DISABLED                 // NR
                      | GPIO_P_MODEH_MODE10_DISABLED                // NR
                      | GPIO_P_MODEH_MODE11_INPUTPULLFILTER         // VREG_ERR
                      | GPIO_P_MODEH_MODE12_DISABLED                // VIN_DETECT
                      | GPIO_P_MODEH_MODE13_DISABLED                // NC
                      | GPIO_P_MODEH_MODE14_DISABLED                // NC
                      | GPIO_P_MODEH_MODE15_DISABLED;               // NC
    GPIO->P[5].DOUT   = BIT(7) | BIT(11);
    GPIO->P[5].OVTDIS = 0;

    // Debugger Route
    GPIO->ROUTEPEN &= ~(GPIO_ROUTEPEN_TDIPEN | GPIO_ROUTEPEN_TDOPEN);   // Disable JTAG
    GPIO->ROUTEPEN |= GPIO_ROUTEPEN_SWVPEN;                             // Enable SWO
    GPIO->ROUTELOC0 = GPIO_ROUTELOC0_SWVLOC_LOC0;                       // SWO on PF2
}

void play_sound(uint16_t usFrequency, uint32_t ulTime)
{
    static uint32_t ubInit = 0;

    if(!ubInit)
    {
        CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER3;

        TIMER3->CTRL = TIMER_CTRL_RSSCOIST | TIMER_CTRL_PRESC_DIV1 | TIMER_CTRL_CLKSEL_PRESCHFPERCLK | TIMER_CTRL_FALLA_NONE | TIMER_CTRL_RISEA_NONE | TIMER_CTRL_MODE_UP;
        TIMER3->CNT = 0x00000000;

        TIMER3->CC[2].CTRL = TIMER_CC_CTRL_PRSCONF_LEVEL | TIMER_CC_CTRL_CUFOA_NONE | TIMER_CC_CTRL_COFOA_TOGGLE | TIMER_CC_CTRL_CMOA_NONE | TIMER_CC_CTRL_MODE_OUTPUTCOMPARE;

        TIMER3->ROUTELOC0 = TIMER_ROUTELOC0_CC2LOC_LOC0;

        ubInit = 1;
    }

    if(!usFrequency)
    {
        TIMER3->ROUTEPEN &= ~TIMER_ROUTEPEN_CC2PEN;
        TIMER3->CMD = TIMER_CMD_STOP;

        return;
    }

    TIMER3->TOP = (HFPER_CLOCK_FREQ / (usFrequency << 1)) - 1; // Double the frequency

    TIMER3->CMD = TIMER_CMD_START;
    TIMER3->ROUTEPEN |= TIMER_ROUTEPEN_CC2PEN;

    if(!ulTime)
        return;

    delay_ms(ulTime);

    TIMER3->ROUTEPEN &= ~TIMER_ROUTEPEN_CC2PEN;
    TIMER3->CMD = TIMER_CMD_STOP;
}