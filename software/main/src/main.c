#include <em_device.h>
#include <stdlib.h>
#include <math.h>
#include "debug_macros.h"
#include "utils.h"
#include "nvic.h"
#include "atomic.h"
#include "systick.h"
#include "rmu.h"
#include "emu.h"
#include "cmu.h"
#include "gpio.h"
#include "dbg.h"
#include "msc.h"
#include "crypto.h"
#include "crc.h"
#include "trng.h"
#include "rtcc.h"
#include "adc.h"
#include "qspi.h"
#include "usart.h"
#include "i2c.h"
#include "rfm69.h"
#include "radio_protocol.h"
#include "ws2812b.h"
#include "bmp280.h"
#include "ccs811.h"
#include "si7021.h"
#include "ili9488.h"
#include "tft.h"
#include "images.h"
#include "fonts.h"
#include "ft6x06.h"

// Structs

// Radio config
#define RADIO_GATEWAY_ID        1
#define RADIO_NODE_ID           2
#define RADIO_NETWORK_ID        193
#define RADIO_AES_KEY           "TheThiccGatewayy" // Needs to be exactly 16 bytes, no zeros allowed

// Forward declarations
static void reset() __attribute__((noreturn));
static void sleep();

static uint32_t get_free_ram();

static void get_device_name(char *pszDeviceName, uint32_t ulDeviceNameSize);
static uint16_t get_device_revision();

// Variables

// ISRs
void _acmp0_1_isr()
{
    uint32_t ulFlags = ACMP0->IFC;

    if(ulFlags & ACMP_IFC_EDGE)
    {
        DBGPRINTLN_CTX("VBat status: %s", (ACMP0->STATUS & ACMP_STATUS_ACMPOUT) ? "HIGH" : "LOW");
    }
}

// Functions
void reset()
{
    SCB->AIRCR = 0x05FA0000 | _VAL2FLD(SCB_AIRCR_SYSRESETREQ, 1);

    while(1);
}
void sleep()
{
    rtcc_set_alarm(rtcc_get_time() + 5);

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; // Configure Deep Sleep (EM2/3)

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        IRQ_CLEAR(RTCC_IRQn);

        __DMB(); // Wait for all memory transactions to finish before memory access
        __DSB(); // Wait for all memory transactions to finish before executing instructions
        __ISB(); // Wait for all memory transactions to finish before fetching instructions
        __SEV(); // Set the event flag to ensure the next WFE will be a NOP
        __WFE(); // NOP and clear the event flag
        __WFE(); // Wait for event
        __NOP(); // Prevent debugger crashes

        cmu_init();
        cmu_update_clocks();
    }
}

uint32_t get_free_ram()
{
    void *pCurrentHeap = malloc(1);

    uint32_t ulFreeRAM = (uint32_t)__get_MSP() - (uint32_t)pCurrentHeap;

    free(pCurrentHeap);

    return ulFreeRAM;
}

void get_device_name(char *pszDeviceName, uint32_t ulDeviceNameSize)
{
    uint8_t ubFamily = (DEVINFO->PART & _DEVINFO_PART_DEVICE_FAMILY_MASK) >> _DEVINFO_PART_DEVICE_FAMILY_SHIFT;
    const char* szFamily = "?";

    switch(ubFamily)
    {
        case 0x10: szFamily = "EFR32MG1P";  break;
        case 0x11: szFamily = "EFR32MG1B";  break;
        case 0x12: szFamily = "EFR32MG1V";  break;
        case 0x13: szFamily = "EFR32BG1P";  break;
        case 0x14: szFamily = "EFR32BG1B";  break;
        case 0x15: szFamily = "EFR32BG1V";  break;
        case 0x19: szFamily = "EFR32FG1P";  break;
        case 0x1A: szFamily = "EFR32FG1B";  break;
        case 0x1B: szFamily = "EFR32FG1V";  break;
        case 0x1C: szFamily = "EFR32MG12P"; break;
        case 0x1D: szFamily = "EFR32MG12B"; break;
        case 0x1E: szFamily = "EFR32MG12V"; break;
        case 0x1F: szFamily = "EFR32BG12P"; break;
        case 0x20: szFamily = "EFR32BG12B"; break;
        case 0x21: szFamily = "EFR32BG12V"; break;
        case 0x25: szFamily = "EFR32FG12P"; break;
        case 0x26: szFamily = "EFR32FG12B"; break;
        case 0x27: szFamily = "EFR32FG12V"; break;
        case 0x28: szFamily = "EFR32MG13P"; break;
        case 0x29: szFamily = "EFR32MG13B"; break;
        case 0x2A: szFamily = "EFR32MG13V"; break;
        case 0x2B: szFamily = "EFR32BG13P"; break;
        case 0x2C: szFamily = "EFR32BG13B"; break;
        case 0x2D: szFamily = "EFR32BG13V"; break;
        case 0x2E: szFamily = "EFR32ZG13P"; break;
        case 0x31: szFamily = "EFR32FG13P"; break;
        case 0x32: szFamily = "EFR32FG13B"; break;
        case 0x33: szFamily = "EFR32FG13V"; break;
        case 0x34: szFamily = "EFR32MG14P"; break;
        case 0x35: szFamily = "EFR32MG14B"; break;
        case 0x36: szFamily = "EFR32MG14V"; break;
        case 0x37: szFamily = "EFR32BG14P"; break;
        case 0x38: szFamily = "EFR32BG14B"; break;
        case 0x39: szFamily = "EFR32BG14V"; break;
        case 0x3A: szFamily = "EFR32ZG14P"; break;
        case 0x3D: szFamily = "EFR32FG14P"; break;
        case 0x3E: szFamily = "EFR32FG14B"; break;
        case 0x3F: szFamily = "EFR32FG14V"; break;
        case 0x47: szFamily = "EFM32G";     break;
        case 0x48: szFamily = "EFM32GG";    break;
        case 0x49: szFamily = "EFM32TG";    break;
        case 0x4A: szFamily = "EFM32LG";    break;
        case 0x4B: szFamily = "EFM32WG";    break;
        case 0x4C: szFamily = "EFM32ZG";    break;
        case 0x4D: szFamily = "EFM32HG";    break;
        case 0x51: szFamily = "EFM32PG1B";  break;
        case 0x53: szFamily = "EFM32JG1B";  break;
        case 0x55: szFamily = "EFM32PG12B"; break;
        case 0x57: szFamily = "EFM32JG12B"; break;
        case 0x64: szFamily = "EFM32GG11B"; break;
        case 0x67: szFamily = "EFM32TG11B"; break;
        case 0x6A: szFamily = "EFM32GG12B"; break;
        case 0x78: szFamily = "EZR32LG";    break;
        case 0x79: szFamily = "EZR32WG";    break;
        case 0x7A: szFamily = "EZR32HG";    break;
    }

    uint8_t ubPackage = (DEVINFO->MEMINFO & _DEVINFO_MEMINFO_PKGTYPE_MASK) >> _DEVINFO_MEMINFO_PKGTYPE_SHIFT;
    char cPackage = '?';

    if(ubPackage == 74)
        cPackage = '?';
    else if(ubPackage == 76)
        cPackage = 'L';
    else if(ubPackage == 77)
        cPackage = 'M';
    else if(ubPackage == 81)
        cPackage = 'Q';

    uint8_t ubTempGrade = (DEVINFO->MEMINFO & _DEVINFO_MEMINFO_TEMPGRADE_MASK) >> _DEVINFO_MEMINFO_TEMPGRADE_SHIFT;
    char cTempGrade = '?';

    if(ubTempGrade == 0)
        cTempGrade = 'G';
    else if(ubTempGrade == 1)
        cTempGrade = 'I';
    else if(ubTempGrade == 2)
        cTempGrade = '?';
    else if(ubTempGrade == 3)
        cTempGrade = '?';

    uint16_t usPartNumber = (DEVINFO->PART & _DEVINFO_PART_DEVICE_NUMBER_MASK) >> _DEVINFO_PART_DEVICE_NUMBER_SHIFT;
    uint8_t ubPinCount = (DEVINFO->MEMINFO & _DEVINFO_MEMINFO_PINCOUNT_MASK) >> _DEVINFO_MEMINFO_PINCOUNT_SHIFT;

    snprintf(pszDeviceName, ulDeviceNameSize, "%s%huF%hu%c%c%hhu", szFamily, usPartNumber, FLASH_SIZE >> 10, cTempGrade, cPackage, ubPinCount);
}
uint16_t get_device_revision()
{
    uint16_t usRevision;

    /* CHIP MAJOR bit [3:0]. */
    usRevision = ((ROMTABLE->PID0 & _ROMTABLE_PID0_REVMAJOR_MASK) >> _ROMTABLE_PID0_REVMAJOR_SHIFT) << 8;
    /* CHIP MINOR bit [7:4]. */
    usRevision |= ((ROMTABLE->PID2 & _ROMTABLE_PID2_REVMINORMSB_MASK) >> _ROMTABLE_PID2_REVMINORMSB_SHIFT) << 4;
    /* CHIP MINOR bit [3:0]. */
    usRevision |= (ROMTABLE->PID3 & _ROMTABLE_PID3_REVMINORLSB_MASK) >> _ROMTABLE_PID3_REVMINORLSB_SHIFT;

    return usRevision;
}

int init()
{
    rmu_init(RMU_CTRL_PINRMODE_FULL, RMU_CTRL_SYSRMODE_EXTENDED, RMU_CTRL_LOCKUPRMODE_EXTENDED, RMU_CTRL_WDOGRMODE_EXTENDED); // Init RMU and set reset modes

    emu_init(1); // Init EMU, ignore DCDC and switch digital power immediatly to DVDD

    cmu_hfxo_startup_calib(0x200, 0x145); // Config HFXO Startup for 1280 uA, 36 pF (18 pF + 2 pF CLOAD)
    cmu_hfxo_steady_calib(0x009, 0x145); // Config HFXO Steady for 12 uA, 36 pF (18 pF + 2 pF CLOAD)

    cmu_lfxo_calib(0x08); // Config LFXO for 10 pF (5 pF + 1 pF CLOAD)

    cmu_init(); // Init Clocks

    cmu_ushfrco_calib(1, USHFRCO_CALIB_50M, 50000000); // Enable and calibrate USHFRCO for 50 MHz
    cmu_auxhfrco_calib(1, AUXHFRCO_CALIB_32M, 32000000); // Enable and calibrate AUXHFRCO for 32 MHz

    cmu_update_clocks(); // Update Clocks

    dbg_init(); // Init Debug module
    dbg_swo_config(BIT(0) | BIT(1), 6000000); // Init SWO channels 0 and 1 at 6 MHz

    msc_init(); // Init Flash, RAM and caches

    systick_init(); // Init system tick

    gpio_init(); // Init GPIOs
    ldma_init(); // Init LDMA
    rtcc_init(); // Init RTCC
    trng_init(); // Init TRNG
    crypto_init(); // Init Crypto engine
    crc_init(); // Init CRC calculation unit
    adc_init(); // Init ADCs
    qspi_init(); // Init QSPI memory

    float fAVDDHighThresh, fAVDDLowThresh;
    float fDVDDHighThresh, fDVDDLowThresh;
    float fIOVDDHighThresh, fIOVDDLowThresh;

    emu_vmon_avdd_config(1, 3.1f, &fAVDDLowThresh, 3.22f, &fAVDDHighThresh); // Enable AVDD monitor
    emu_vmon_dvdd_config(1, 2.5f, &fDVDDLowThresh); // Enable DVDD monitor
    emu_vmon_iovdd_config(1, 3.15f, &fIOVDDLowThresh); // Enable IOVDD monitor

    fDVDDHighThresh = fDVDDLowThresh + 0.026f; // Hysteresis from datasheet
    fIOVDDHighThresh = fIOVDDLowThresh + 0.026f; // Hysteresis from datasheet

    usart0_init(9000000, 0, USART_SPI_MSB_FIRST, 2, 2, 2);  // SPI0 at 9MHz on Location 2 MISO:PC10 MOSI:PC11 CLK:PC9 ESP8266 WIFI-COPROCESSOR
    usart1_init(18000000, 0, USART_SPI_MSB_FIRST, 1, 1, 1);  // SPI1 at 18MHz on Location 1 MISO:PD1 MOSI:PD0 CLK:PD2 ILI9488 Display
    usart2_init(115200, UART_FRAME_STOPBITS_ONE | UART_FRAME_PARITY_NONE | USART_FRAME_DATABITS_EIGHT, 0, 0, -1, -1); // USART2 at 115200Baud on Location 0 RTS-PC0 CTS-PC1 TX-PC2 RX-PC3 GSM
    usart3_init(9000000, 0, USART_SPI_MSB_FIRST, 0, 0, 0); // SPI3 at 9MHz on Location 0 MISO-PA1 MOSI-PA0 CLK-PA2 RFM

    i2c0_init(I2C_NORMAL, 6, 6); // Init I2C0 at 100 kHz on location 6 SCL:PE13 SDA:PE12 Sensors
    i2c1_init(I2C_NORMAL, 1, 1); // Init I2C1 at 100 kHz on location 1 SCL:PB12 SDA:PB11 TFT Touch Controller

    char szDeviceName[32];

    get_device_name(szDeviceName, 32);

    DBGPRINTLN_CTX("Device: %s", szDeviceName);
    DBGPRINTLN_CTX("Device Revision: 0x%04X", get_device_revision());
    DBGPRINTLN_CTX("Calibration temperature: %hhu C", (DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT);
    DBGPRINTLN_CTX("Flash Size: %hu KiB", FLASH_SIZE >> 10);
    DBGPRINTLN_CTX("RAM Size: %hu KiB", SRAM_SIZE >> 10);
    DBGPRINTLN_CTX("Free RAM: %lu KiB", get_free_ram() >> 10);
    DBGPRINTLN_CTX("Unique ID: %08X-%08X", DEVINFO->UNIQUEH, DEVINFO->UNIQUEL);

    DBGPRINTLN_CTX("RMU - Reset cause: %hhu", rmu_get_reset_reason());
    DBGPRINTLN_CTX("RMU - Reset state: %hhu", rmu_get_reset_state());

    rmu_clear_reset_reason();

    DBGPRINTLN_CTX("CMU - HFXO Clock: %.1f MHz", (float)HFXO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - HFRCO Clock: %.1f MHz", (float)HFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - USHFRCO Clock: %.1f MHz", (float)USHFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - AUXHFRCO Clock: %.1f MHz", (float)AUXHFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - LFXO Clock: %.3f kHz", (float)LFXO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - LFRCO Clock: %.3f kHz", (float)LFRCO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - ULFRCO Clock: %.3f kHz", (float)ULFRCO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - HFSRC Clock: %.1f MHz", (float)HFSRC_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HF Clock: %.1f MHz", (float)HF_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFBUS Clock: %.1f MHz", (float)HFBUS_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFCORE Clock: %.1f MHz", (float)HFCORE_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFEXP Clock: %.1f MHz", (float)HFEXP_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPER Clock: %.1f MHz", (float)HFPER_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPERB Clock: %.1f MHz", (float)HFPERB_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPERC Clock: %.1f MHz", (float)HFPERC_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFLE Clock: %.1f MHz", (float)HFLE_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - QSPI Clock: %.1f MHz", (float)QSPI_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - SDIO Clock: %.1f MHz", (float)SDIO_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - USB Clock: %.1f MHz", (float)USB_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - ADC0 Clock: %.1f MHz", (float)ADC0_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - ADC1 Clock: %.1f MHz", (float)ADC1_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - DBG Clock: %.1f MHz", (float)DBG_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - AUX Clock: %.1f MHz", (float)AUX_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - LFA Clock: %.3f kHz", (float)LFA_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LESENSE Clock: %.3f kHz", (float)LESENSE_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - RTC Clock: %.3f kHz", (float)RTC_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LCD Clock: %.3f kHz", (float)LCD_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LETIMER0 Clock: %.3f kHz", (float)LETIMER0_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LETIMER1 Clock: %.3f kHz", (float)LETIMER1_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFB Clock: %.3f kHz", (float)LFB_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LEUART0 Clock: %.3f kHz", (float)LEUART0_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LEUART1 Clock: %.3f kHz", (float)LEUART1_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - SYSTICK Clock: %.3f kHz", (float)SYSTICK_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - CSEN Clock: %.3f kHz", (float)CSEN_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFC Clock: %.3f kHz", (float)LFC_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFE Clock: %.3f kHz", (float)LFE_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - RTCC Clock: %.3f kHz", (float)RTCC_CLOCK_FREQ / 1000);

    DBGPRINTLN_CTX("EMU - AVDD Fall Threshold: %.2f mV!", fAVDDLowThresh * 1000);
    DBGPRINTLN_CTX("EMU - AVDD Rise Threshold: %.2f mV!", fAVDDHighThresh * 1000);
    DBGPRINTLN_CTX("EMU - AVDD Voltage: %.2f mV", adc_get_avdd());
    DBGPRINTLN_CTX("EMU - AVDD Status: %s", g_ubAVDDLow ? "LOW" : "OK");
    DBGPRINTLN_CTX("EMU - DVDD Fall Threshold: %.2f mV!", fDVDDLowThresh * 1000);
    DBGPRINTLN_CTX("EMU - DVDD Rise Threshold: %.2f mV!", fDVDDHighThresh * 1000);
    DBGPRINTLN_CTX("EMU - DVDD Voltage: %.2f mV", adc_get_dvdd());
    DBGPRINTLN_CTX("EMU - DVDD Status: %s", g_ubDVDDLow ? "LOW" : "OK");
    DBGPRINTLN_CTX("EMU - IOVDD Fall Threshold: %.2f mV!", fIOVDDLowThresh * 1000);
    DBGPRINTLN_CTX("EMU - IOVDD Rise Threshold: %.2f mV!", fIOVDDHighThresh * 1000);
    DBGPRINTLN_CTX("EMU - IOVDD Voltage: %.2f mV", adc_get_iovdd());
    DBGPRINTLN_CTX("EMU - IOVDD Status: %s", g_ubIOVDDLow ? "LOW" : "OK");
    DBGPRINTLN_CTX("EMU - Core Voltage: %.2f mV", adc_get_corevdd());
    DBGPRINTLN_CTX("EMU - 5V0 Voltage: %.2f mV", adc_get_5v0());
    DBGPRINTLN_CTX("EMU - 4V2 Voltage: %.2f mV", adc_get_4v2());
    DBGPRINTLN_CTX("EMU - VBAT Voltage: %.2f mV", adc_get_vbat());
    DBGPRINTLN_CTX("EMU - VIN Voltage: %.2f mV", adc_get_vin());

    play_sound(3500, 250);
    delay_ms(100);

    // Assert CCS811 control signals otherwise the I2C scan won't detect it
    CCS811_UNRESET();
    delay_ms(10);
    CCS811_WAKE();

    DBGPRINTLN_CTX("Scanning I2C bus 0...");

    for(uint8_t a = 0x08; a < 0x78; a++)
        if(i2c0_write(a, NULL, 0, I2C_STOP))
            DBGPRINTLN_CTX("  Address 0x%02X ACKed!", a);

    DBGPRINTLN_CTX("Scanning I2C bus 1...");

    for(uint8_t a = 0x08; a < 0x78; a++)
        if(i2c1_write(a, NULL, 0, I2C_STOP))
            DBGPRINTLN_CTX("  Address 0x%02X ACKed!", a);

    if(bmp280_init())
        DBGPRINTLN_CTX("BMP280 init OK!");
    else
        DBGPRINTLN_CTX("BMP280 init NOK!");

    if(ccs811_init())
        DBGPRINTLN_CTX("CCS811 init OK!");
    else
        DBGPRINTLN_CTX("CCS811 init NOK!");

    if(si7021_init())
        DBGPRINTLN_CTX("SI7021 init OK!");
    else
        DBGPRINTLN_CTX("SI7021 init NOK!");

    if(ili9488_init())
        DBGPRINTLN_CTX("ILI9488 init OK!");
    else
        DBGPRINTLN_CTX("ILI9488 init NOK!");

    if(ft6x06_init())
        DBGPRINTLN_CTX("FT6236 init OK!");
    else
        DBGPRINTLN_CTX("FT6236 init NOK!");

    if(rfm69_init(RADIO_GATEWAY_ID, RADIO_NETWORK_ID, RADIO_AES_KEY))
        DBGPRINTLN_CTX("RFM69 init OK!");
    else
        DBGPRINTLN_CTX("RFM69 init NOK!");

    return 0;
}
int main()
{
    play_sound(3000, 50);
    delay_ms(50);
    play_sound(3000, 50);

    ws2812b_init();
    ws2812b_set_color(0, 0, 142, 255);

    // CLK OUT to check if the clock was properly calibrated
    //CMU->ROUTELOC0 = CMU_ROUTELOC0_CLKOUT1LOC_LOC1;
    //CMU->ROUTEPEN |= CMU_ROUTEPEN_CLKOUT1PEN;
    //CMU->CTRL |= CMU_CTRL_CLKOUTSEL1_HFXO;

    // ----------------- Testing battery monitoring with OpAmp + Analog Comparator ----------------- //
    CMU->HFPERCLKEN1 |= CMU_HFPERCLKEN1_VDAC0;

    VDAC0->OPA[1].CTRL = VDAC_OPA_CTRL_OUTSCALE_FULL | VDAC_OPA_CTRL_HCMDIS | (0x3 << _VDAC_OPA_CTRL_DRIVESTRENGTH_SHIFT); // Enable full drive strength, no rail-to-rail inputs
    VDAC0->OPA[1].TIMER = (0x001 << _VDAC_OPA_TIMER_SETTLETIME_SHIFT) | (0x05 << _VDAC_OPA_TIMER_WARMUPTIME_SHIFT) | (0x00 << _VDAC_OPA_TIMER_STARTUPDLY_SHIFT); // Recommended settings
    VDAC0->OPA[1].MUX = VDAC_OPA_MUX_RESSEL_RES1 | VDAC_OPA_MUX_RESINMUX_DISABLE | VDAC_OPA_MUX_NEGSEL_UG | VDAC_OPA_MUX_POSSEL_POSPAD; // Unity gain with POSPAD as non-inverting input
    VDAC0->OPA[1].OUT = 0; // Disable all outputs (NEXT1 is always connected)
    VDAC0->OPA[1].CAL = DEVINFO->OPA1CAL7; // Calibration for DRIVESTRENGTH = 0x3, INCBW = 0

    VDAC0->CMD = VDAC_CMD_OPA1EN; // Enable OPA1
    while(!(VDAC0->STATUS & VDAC_STATUS_OPA1ENS)); // Wait for it to be enabled

    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_ACMP0;

    ACMP0->CTRL = ACMP_CTRL_FULLBIAS | (0x20 << _ACMP_CTRL_BIASPROG_SHIFT) | ACMP_CTRL_IFALL | ACMP_CTRL_IRISE | ACMP_CTRL_INPUTRANGE_GTVDDDIV2 | ACMP_CTRL_ACCURACY_HIGH | ACMP_CTRL_PWRSEL_AVDD | ACMP_CTRL_GPIOINV_NOTINV | ACMP_CTRL_INACTVAL_LOW;
    ACMP0->INPUTSEL = ACMP_INPUTSEL_VBSEL_2V5 | ACMP_INPUTSEL_VASEL_VDD | ACMP_INPUTSEL_NEGSEL_VBDIV | ACMP_INPUTSEL_POSSEL_DACOUT1;
    ACMP0->HYSTERESIS0 = 48 << _ACMP_HYSTERESIS0_DIVVB_SHIFT; // VBat is high when >= 3.828125 V
    ACMP0->HYSTERESIS1 = 44 << _ACMP_HYSTERESIS1_DIVVB_SHIFT; // VBat is low when <= 3.515625 V

    ACMP0->IFC = _ACMP_IFC_MASK; // Clear pending IRQs
    IRQ_CLEAR(ACMP0_IRQn); // Clear pending vector
    IRQ_SET_PRIO(ACMP0_IRQn, 1, 0); // Set priority 1,0
    IRQ_ENABLE(ACMP0_IRQn); // Enable vector
    ACMP0->IEN |= ACMP_IEN_EDGE; // Enable EDGE interrupt

    ACMP0->CTRL |= ACMP_CTRL_EN; // Enable ACMP0
    while(!(ACMP0->STATUS & ACMP_STATUS_ACMPACT)); // Wait for it to be enabled
    // ----------------- Testing battery monitoring with OpAmp + Analog Comparator ----------------- //

    // BMP280 info & configuration
    DBGPRINTLN_CTX("BMP280 version: 0x%02X", bmp280_read_version());

    bmp280_write_config(BMP280_STANDBY_1000MS | BMP280_FILTER_8);
    bmp280_write_control(BMP280_TEMP_OS4 | BMP280_PRESSURE_OS16 | BMP280_MODE_NORMAL);
    DBGPRINTLN_CTX("BMP280 write control & config!");

    // CCS811 info & configuration
    DBGPRINTLN_CTX("CCS811 Hardware version: %hhu", ccs811_read_hw_version());

    uint16_t usCCSBootVersion = ccs811_read_boot_version();

    DBGPRINTLN_CTX("CCS811 Bootloader version: %hu.%hu.%hu", CCS811_SW_VERSION_MAJOR(usCCSBootVersion), CCS811_SW_VERSION_MINOR(usCCSBootVersion), CCS811_SW_VERSION_TRIVIAL(usCCSBootVersion));

    ccs811_app_start();

    uint16_t usCCSAppVersion = ccs811_read_app_version();

    DBGPRINTLN_CTX("CCS811 App version: %hu.%hu.%hu", CCS811_SW_VERSION_MAJOR(usCCSAppVersion), CCS811_SW_VERSION_MINOR(usCCSAppVersion), CCS811_SW_VERSION_TRIVIAL(usCCSAppVersion));

    ccs811_write_meas_mode(CCS811_DRIVE_MODE_1S);
    DBGPRINTLN_CTX("CCS811 Meas. mode config!");

    // SI7021 info & configuration
    DBGPRINTLN_CTX("SI7021 UID: 0x%016llX", si7021_read_unique_id());
    DBGPRINTLN_CTX("SI7021 Firmware version: 0x%02X", si7021_read_firmware_version());

    si7021_write_user(SI7021_USER_RES_RH12_T14 | SI7021_USER_HEATER_OFF);
    DBGPRINTLN_CTX("SI7021 write user register!");

    // QSPI Flash info
    uint8_t ubFlashUID[8];

    qspi_flash_read_security(0x0000, ubFlashUID, 8);

    DBGPRINTLN_CTX("QSPI Flash UID: %02X%02X%02X%02X%02X%02X%02X%02X", ubFlashUID[0], ubFlashUID[1], ubFlashUID[2], ubFlashUID[3], ubFlashUID[4], ubFlashUID[5], ubFlashUID[6], ubFlashUID[7]);
    DBGPRINTLN_CTX("QSPI Flash JEDEC ID: %06X", qspi_flash_read_jedec_id());

    WIFI_SELECT();
    WIFI_RESET();
    delay_ms(10);
    WIFI_UNRESET();
    delay_ms(100);
    //WIFI_UNSELECT();

    GSM_PWR_KEY_SET();
    delay_ms(1000);
    GSM_PWR_KEY_RSET();

    // TFT Controller info
    DBGPRINTLN_CTX("ILI9488 ID: 0x%06X", ili9488_read_id());

    // TFT Touch info
    DBGPRINTLN_CTX("FT6236 Touch controller");
    DBGPRINTLN_CTX("Vendor ID: 0x%02X", ft6x06_get_vendor_id());
    DBGPRINTLN_CTX("Chip ID: 0x%02X", ft6x06_get_chip_id());
    DBGPRINTLN_CTX("Firmware V: 0x%02X", ft6x06_get_firmware_version());
    DBGPRINTLN_CTX("Point Rate Hz: %hu", ft6x06_get_point_rate());
    DBGPRINTLN_CTX("Thresh: 0x%02X", ft6x06_get_threshold());
    ft6x06_set_threshold(128);

    // TFT Config
    tft_bl_init(2000); // Init backlight PWM at 2 kHz
    tft_bl_set(0.25f); // Set backlight to 25%
    tft_display_on(); // Turn display on
    tft_set_rotation(ILI9488_VERTICAL); // Set rotation 1 (horizontal, ribbon to the right)
    tft_fill_screen(RGB565_DARKGREY); // Fill display

    tft_graph_t *pGraph = tft_graph_create(60, 30, 200, 100, 0, 30, 5, 25, 32, .5, 1, "Temperature", "t", "C", &xSans9pFont, RGB565_WHITE, RGB565_BLACK, RGB565_YELLOW, RGB565_BLACK, RGB565_DARKGREY);
    tft_graph_draw_frame(pGraph);

    tft_terminal_t *terminal = tft_terminal_create(10, 250, 9, 300, &xSans9pFont, RGB565_GREEN, RGB565_BLACK);
    if(!terminal)
    {
        DBGPRINTLN_CTX("Could not create terminal");

        while(1);
    }

    //tft_printf(&xSans18pFont, 10, 10, RGB565_WHITE, RGB565_DARKGREY, "Display is the wey");
/*
    tft_draw_rectangle(10, 65, 295 + 15 + 5, 75 + tft_get_text_height(&xSans9pFont, 6), RGB565_DARKGREEN, 1);

    tft_textbox_t *textbox = tft_textbox_create(15, 70, 6, 295, 0, 0, &xSans9pFont, RGB565_BLUE, RGB565_WHITE);
    if(!textbox)
    {
        DBGPRINTLN_CTX("Could not allocate start textbox");

        while(1);
    }
*/
    while(1)
    {
        /* - - - - - - - - Library Tasks - - - - - - - - -*/
        rfm69_tick();
        /* - - - - - - - - Library Tasks - - - - - - - - -*/

        /* - - - - - - - - Main Tasks - - - - - - - - -*/
        static uint64_t ullLastLedUpdate = 0;

        if(g_ullSystemTick > (ullLastLedUpdate + 500))
        {
            uint32_t ullColor = trng_pop_random();
            // FIXME: sum real weird going on
            //ws2812b_set_color(0, (uint8_t)((ullColor >> 16) & 0xFF), (uint8_t)((ullColor >> 8) & 0xFF), (uint8_t)(ullColor & 0xFF));

            //static uint8_t ubLastState = 0;

            //if(!ubLastState)
            //{
            //}
            //else
            //{
            //}
            //ubLastState = !ubLastState;

            ullLastLedUpdate = g_ullSystemTick;
        }

        static uint64_t ullLastGraphRoutine = 0;

        if(g_ullSystemTick > (ullLastGraphRoutine + 1000))
        {
            static uint8_t ubCount = 0;

            if(ubCount == 31)
            {
                tft_graph_clear(pGraph);
                tft_graph_draw_frame(pGraph);
                ubCount = 0;
            }

            float fCount = ubCount;
            float fTemp = bmp280_read_temperature();

            tft_graph_draw_data(pGraph, &fCount, &fTemp, 1);

            ubCount++;

            ullLastGraphRoutine = g_ullSystemTick;
        }

        static uint64_t ullLastSwoPrint = 0;

        if(g_ullSystemTick > (ullLastSwoPrint + 10000))
        {

            DBGPRINTLN_CTX("\rADC Temp: %.2f", adc_get_temperature());
            DBGPRINTLN_CTX("EMU Temp: %.2f", emu_get_temperature());
            DBGPRINTLN_CTX("RTCC Time: %lu", rtcc_get_time());
            switch(((BAT_STDBY() << 1) | BAT_CHRG()) & 0x03)
            {
                case 0b00:
                    DBGPRINTLN_CTX("Battery State: No Vin\n\r");
                    break;
                case 0b01:
                    DBGPRINTLN_CTX("Battery State: Charging\n\r");
                    break;
                case 0b10:
                    DBGPRINTLN_CTX("Battery State: Charged\n\r");
                    break;
                case 0b11:
                    DBGPRINTLN_CTX("Battery State: Err\n\r");
                    break;
            }
            DBGPRINTLN_CTX("3V3 Fault: ");
            DBGPRINTLN_CTX("%hhu", VREG_ERR());
            DBGPRINTLN_CTX("Button states (1|2|3): %hhu|%hhu|%hhu", BTN_1_STATE(), BTN_2_STATE(), BTN_3_STATE());

            DBGPRINTLN_CTX("USART2 Available: %u", usart2_available());

            if(usart2_available())
            {
                DBGPRINT_CTX("Data: [");

                while(usart2_available())
                    DBGPRINT("%c", usart2_read_byte());

                DBGPRINTLN("]");
            }

            float fTemp = bmp280_read_temperature();
            float fPress = bmp280_read_pressure();

            tft_terminal_printf(terminal, 0, "\nBMP280 Temperature: %.2f C", fTemp);
            tft_terminal_printf(terminal, 0, "\nBMP280 Pressure: %.2f hPa", fPress);

            DBGPRINTLN_CTX("BMP280 Temperature: %.2f C", fTemp);
            DBGPRINTLN_CTX("BMP280 Pressure: %.2f hPa", fPress);

            uint16_t usETVOC = ccs811_read_etvoc();
            uint16_t usECO2 = ccs811_read_eco2();

            tft_terminal_printf(terminal, 0, "\nCCS811 eTVOC: %hu ppb", usETVOC);
            tft_terminal_printf(terminal, 0, "\nCCS811 eCO2: %hu ppm", usECO2);

            DBGPRINTLN_CTX("CCS811 eTVOC: %hu ppb", usETVOC);
            DBGPRINTLN_CTX("CCS811 eCO2: %hu ppm", usECO2);

            float fSITemp = si7021_read_temperature();
            float fSIHumid = si7021_read_humidity();

            tft_terminal_printf(terminal, 0, "\nSI7021 Temperature: %.2f C", fSITemp);
            tft_terminal_printf(terminal, 0, "\nSI7021 Humidity: %.1f %%RH", fSIHumid);

            DBGPRINTLN_CTX("SI7021 Temperature: %.2f C", fSITemp);
            DBGPRINTLN_CTX("SI7021 Humidity: %.1f %%RH", fSIHumid);

            //play_sound(2700, 10);
            tft_terminal_printf(terminal, 1, "\nFree RAM: %lu KiB", get_free_ram() >> 10);

            ullLastSwoPrint = g_ullSystemTick;
        }

        /* - - - - - - - - Main Tasks - - - - - - - - -*/

        /* - - - - - - - - Button Routines - - - - - - - - -*/
        static uint8_t ubLastBtn1State = 0;
        static uint8_t ubLastBtn2State = 0;
        static uint8_t ubLastBtn3State = 0;

        if(BTN_1_STATE() && (ubLastBtn1State != 1))
        {
            //tft_draw_image(&xSurpriseImage, 0, 0);

            ubLastBtn1State = 1;
        }
        else if(!BTN_1_STATE() && (ubLastBtn1State != 0))
        {
            tft_fill_screen(RGB565_DARKGREY);
            //tft_printf(&xSans18pFont, 10, 10, RGB565_WHITE, RGB565_DARKGREY, "Display is the wey");
            //tft_draw_rectangle(10, 65, 295 + 15 + 5, 75 + tft_get_text_height(&xSans9pFont, 6), RGB565_DARKGREEN, 1);
            //tft_textbox_clear(textbox);
            tft_terminal_printf(terminal, 1, "\nA nice surprise was shown...");

            ubLastBtn1State = 0;
        }

        if(BTN_2_STATE() && (ubLastBtn2State != 1))
        {
            tft_fill_screen(RGB565_WHITE);
            for(uint8_t x = 0; x < 4; x++)
            {
                for(uint8_t y = 0; y < 6; y++)
                {
                    tft_draw_image(&xPepeImage, 32 + (x * 64), 32 + (y * 64));
                }
            }
            ubLastBtn2State = 1;
        }
        else if(!BTN_2_STATE() && (ubLastBtn2State != 0))
        {
            tft_fill_screen(RGB565_DARKGREY);
            //tft_printf(&xSans18pFont, 10, 10, RGB565_WHITE, RGB565_DARKGREY, "Display is the wey");
            //tft_draw_rectangle(10, 65, 295 + 15 + 5, 75 + tft_get_text_height(&xSans9pFont, 6), RGB565_DARKGREEN, 1);
            //tft_textbox_clear(textbox);
            tft_terminal_printf(terminal, 1, "\nPepe army was shown...");

            ubLastBtn2State = 0;
        }

        if(BTN_3_STATE() && (ubLastBtn3State != 1))
        {
            ubLastBtn3State = 1;
        }
        else if(!BTN_3_STATE() && (ubLastBtn3State != 0))
        {
            usart2_write_byte('A');
            usart2_write_byte('T');
            usart2_write_byte('\r');
            usart2_write_byte('\n');

            uint8_t ubNBytes = 0;

            uint8_t ubBuf[ubNBytes];
            memset(ubBuf, 0x00, ubNBytes);

            ubBuf[0] = 0x03;
            ubBuf[1] = 0x05;

            tft_terminal_printf(terminal, 1, "\nTransfering %d byte(s) to wifi-coprocessor...", ubNBytes);
            DBGPRINTLN_CTX("Transfering %d byte(s) to wifi-coprocessor...", ubNBytes);
            DBGPRINTLN_CTX("Content:");
            for(uint8_t ubI = 0; ubI < ubNBytes; ubI++)
                DBGPRINTLN_CTX("\t0x%02X", ubBuf[ubI]);

            WIFI_UNSELECT();
            delay_ms(10);

            WIFI_SELECT();
            //usart0_spi_transfer(ubBuf, ubNBytes, ubBuf);
            WIFI_UNSELECT();


            DBGPRINTLN_CTX("Received:");
            for(uint8_t ubI = 0; ubI < ubNBytes; ubI++)
                DBGPRINTLN_CTX("\t0x%02X", ubBuf[ubI]);

            ubLastBtn3State = 0;
        }
        /* - - - - - - - - Button Routines - - - - - - - - -*/
    }

    return 0;
}