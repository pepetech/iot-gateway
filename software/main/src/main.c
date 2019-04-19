#include <em_device.h>
#include <stdlib.h>
#include <math.h>
#include "debug_macros.h"
#include "utils.h"
#include "nvic.h"
#include "atomic.h"
#include "systick.h"
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
#include "tft.h"
#include "images.h"
#include "fonts.h"

// Structs

// Forward declarations
static void reset() __attribute__((noreturn));
static void sleep();

static uint32_t get_free_ram();

void get_device_name(char *pszDeviceName, uint32_t ulDeviceNameSize);
static uint16_t get_device_revision();

// Variables

// ISRs

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
        __NOP(); // Prevent debugger crashesdelay_ms(10);

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
    emu_init(1); // Init EMU

    cmu_hfxo_startup_calib(0x200, 0x145); // Cdelay_ms(10);onfig HFXO Startup for 1280 uA, 36 pF (18 pF + 2 pF CLOAD)
    cmu_hfxo_steady_calib(0x009, 0x145); // Codelay_ms(10);nfig HFXO Steady state for 12 uA, 36 pF (18 pF + 2 pF CLOAD)

    cmu_lfxo_calib(0x08); // Config LFXO for 1delay_ms(10);0 pF (5 pF + 1 pF CLOAD)
    cmu_init(); // Init Clocksdelay_ms(10);

    cmu_ushfrco_calib(1, USHFRCO_CALIB_50M, 50000000); // Enable and calibrate USHFRCO for 50 MHz
    cmu_auxhfrco_calib(1, AUXHFRCO_CALIB_32M, 32000000); // Enable and calibrate AUXHFRCO for 32 MHz

    cmu_update_clocks(); // Update Clocks

    dbg_init(); // Init Debug module
    dbg_swo_config(BIT(0) | BIT(1), 2000000); // Init SWO channels 0 and 1 at 2 MHz

    msc_init(); // Init Flash, RAM and caches

    systick_init(); // Init system tick

    gpio_init(); // Init GPIOs
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
    //usart2_init(115200, 0, UART_FRAME_STOPBITS_ONE, 0, 0, 0); // USART2 at 115200Baud on Location 0 RTS-PC0 CTS-PC1 TX-PC2 RX-PC3 GSM
    //usart3_init(10000000, 0, USART_SPI_MSB_FIRST, 0, 0, 0); // SPI3 at 10MHz on Location 0 MISO-PA1 MOSI-PA0 CLK-PA2 RFM

    i2c0_init(I2C_NORMAL, 6, 6); // Init I2C0 at 100 kHz on location 6 SCL:SDA PE13:PE12 Sensors
    i2c1_init(I2C_NORMAL, 1, 1); // Init I2C1 at 100 kHz on location 1 SCL:SDA PB12:PB11 TFT Touch Controller

    char szDeviceName[32];

    get_device_name(szDeviceName, 32);

    DBGPRINTLN_CTX("Device: %s", szDeviceName);
    DBGPRINTLN_CTX("Device Revision: 0x%04X", get_device_revision());
    DBGPRINTLN_CTX("Calibration temperature: %hhu C", (DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT);
    DBGPRINTLN_CTX("Flash Size: %hu kiB", FLASH_SIZE >> 10);
    DBGPRINTLN_CTX("RAM Size: %hu kiB", SRAM_SIZE >> 10);
    DBGPRINTLN_CTX("Free RAM: %lu kiB", get_free_ram() >> 10);
    DBGPRINTLN_CTX("Unique ID: %08X-%08X", DEVINFO->UNIQUEH, DEVINFO->UNIQUEL);

    DBGPRINTLN_CTX("CMU - HFXO Clock: %.1f MHz!", (float)HFXO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - HFRCO Clock: %.1f MHz!", (float)HFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - USHFRCO Clock: %.1f MHz!", (float)USHFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - AUXHFRCO Clock: %.1f MHz!", (float)AUXHFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - LFXO Clock: %.3f kHz!", (float)LFXO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - LFRCO Clock: %.3f kHz!", (float)LFRCO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - ULFRCO Clock: %.3f kHz!", (float)ULFRCO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - HFSRC Clock: %.1f MHz!", (float)HFSRC_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HF Clock: %.1f MHz!", (float)HF_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFBUS Clock: %.1f MHz!", (float)HFBUS_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFCORE Clock: %.1f MHz!", (float)HFCORE_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFEXP Clock: %.1f MHz!", (float)HFEXP_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPER Clock: %.1f MHz!", (float)HFPER_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPERB Clock: %.1f MHz!", (float)HFPERB_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPERC Clock: %.1f MHz!", (float)HFPERC_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFLE Clock: %.1f MHz!", (float)HFLE_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - QSPI Clock: %.1f MHz!", (float)QSPI_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - SDIO Clock: %.1f MHz!", (float)SDIO_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - USB Clock: %.1f MHz!", (float)USB_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - ADC0 Clock: %.1f MHz!", (float)ADC0_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - ADC1 Clock: %.1f MHz!", (float)ADC1_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - DBG Clock: %.1f MHz!", (float)DBG_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - AUX Clock: %.1f MHz!", (float)AUX_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - LFA Clock: %.3f kHz!", (float)LFA_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LESENSE Clock: %.3f kHz!", (float)LESENSE_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - RTC Clock: %.3f kHz!", (float)RTC_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LCD Clock: %.3f kHz!", (float)LCD_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LETIMER0 Clock: %.3f kHz!", (float)LETIMER0_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LETIMER1 Clock: %.3f kHz!", (float)LETIMER1_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFB Clock: %.3f kHz!", (float)LFB_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LEUART0 Clock: %.3f kHz!", (float)LEUART0_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LEUART1 Clock: %.3f kHz!", (float)LEUART1_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - SYSTICK Clock: %.3f kHz!", (float)SYSTICK_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - CSEN Clock: %.3f kHz!", (float)CSEN_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFC Clock: %.3f kHz!", (float)LFC_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFE Clock: %.3f kHz!", (float)LFE_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - RTCC Clock: %.3f kHz!", (float)RTCC_CLOCK_FREQ / 1000);

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

    //play_sound(3500, 500);
    //delay_ms(100);

    CCS811_WAKE();
    CCS811_UNRESET();

    DBGPRINTLN_CTX("Scanning I2C bus 0...");

    for(uint8_t a = 0x08; a < 0x78; a++)
    {
        if(i2c0_write(a, 0, 0, I2C_STOP))
            DBGPRINTLN_CTX("  Address 0x%02X ACKed!", a);
    }

    DBGPRINTLN_CTX("Scanning I2C bus 1...");

    for(uint8_t a = 0x08; a < 0x78; a++)
    {
        if(i2c1_write(a, 0, 0, I2C_STOP))
            DBGPRINTLN_CTX("  Address 0x%02X ACKed!", a);
    }

    return 0;
}
int main()
{
    /*
    play_sound(2700, 50);
    play_sound(3000, 50);
    play_sound(3300, 50);
    play_sound(3600, 50);
    play_sound(3900, 50);
    play_sound(4200, 50);
    play_sound(4500, 50);
    */
    //CMU->ROUTELOC0 = CMU_ROUTELOC0_CLKOUT1LOC_LOC1;
    //CMU->ROUTEPEN |= CMU_ROUTEPEN_CLKOUT1PEN;
    //CMU->CTRL |= CMU_CTRL_CLKOUTSEL1_HFXO;

    // Internal flash test
    //DBGPRINTLN_CTX("Initial calibration dump:");

    //for(init_calib_t *psCalibTbl = g_psInitCalibrationTable; psCalibTbl->pulRegister; psCalibTbl++)
    //    DBGPRINTLN_CTX("  0x%08X -> 0x%08X", psCalibTbl->ulInitialCalibration, psCalibTbl->pulRegister);

    /*
    DBGPRINTLN_CTX("Boot lock word: %08X", g_psLockBits->CLW[0]);
    DBGPRINTLN_CTX("User lock word: %08X", g_psLockBits->ULW);
    DBGPRINTLN_CTX("Mass lock word: %08X", g_psLockBits->MLW);
    msc_flash_word_write((uint32_t)&(g_psLockBits->MLW), 0xFFFFFFFD);
    DBGPRINTLN_CTX("Mass lock word: %08X", g_psLockBits->MLW);

    DBGPRINTLN_CTX("0x000FFFFC: %08X", *(volatile uint32_t *)0x000FFFFC);
    DBGPRINTLN_CTX("0x00100000: %08X", *(volatile uint32_t *)0x00100000);
    msc_flash_word_write(0x000FFFFC, 0x12344321);
    msc_flash_word_write(0x00100000, 0xABCDDCBA);
    DBGPRINTLN_CTX("0x000FFFFC: %08X", *(volatile uint32_t *)0x000FFFFC);
    DBGPRINTLN_CTX("0x00100000: %08X", *(volatile uint32_t *)0x00100000);
    msc_flash_unlock();
    MSC->WRITECMD = MSC_WRITECMD_ERASEMAIN1;
    msc_flash_lock();
    DBGPRINTLN_CTX("0x000FFFFC: %08X", *(volatile uint32_t *)0x000FFFFC);
    DBGPRINTLN_CTX("0x00100000: %08X", *(volatile uint32_t *)0x00100000);
    */

    // QSPI
    DBGPRINTLN_CTX("Flash Part ID: %06X", qspi_flash_read_jedec_id());

    uint8_t ubFlashUID[8];

    qspi_flash_read_security(0x0000, ubFlashUID, 8);

    DBGPRINTLN_CTX("Flash ID: %02X%02X%02X%02X%02X%02X%02X%02X", ubFlashUID[0], ubFlashUID[1], ubFlashUID[2], ubFlashUID[3], ubFlashUID[4], ubFlashUID[5], ubFlashUID[6], ubFlashUID[7]);

    //qspi_flash_chip_erase();

    //uint8_t rd[16];

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00008000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);

    //DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000000);
    //*(volatile uint32_t *)0xC0000000 = 0xABCDEF12;

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00000000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);

    //uint32_t wr = 0xA2B3C4D5;
    //qspi_flash_busy_wait();
    //qspi_flash_write_enable();
    //qspi_flash_cmd(QSPI_FLASH_CMD_WRITE, 0x00000004, 3, 0, 0, (uint8_t*)&wr, 4, NULL, 0);
    //qspi_flash_busy_wait();

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00000000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);

    //*(volatile uint32_t *)0xC0000000 = 0x12AB34CD;

    //////// Test for page wrapping (write beyond page boundary)

    /*
    for(uint8_t i = 0; i <= 64; i++)
        *(volatile uint32_t *)(0xC0000000 + i * 4) = 0x0123ABCD;

    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000000); // CD
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000001); // AB
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000002); // 23
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000003); // 01
    DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000000); // 0123ABCD
    DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000010); // 0123ABCD
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FC); // CD
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FD); // AB
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FE); // 23
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FF); // 01
    DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000100); // 0123ABCD
    */

    //////// Test for code copy to QSPI flash

    /*
    for(uint32_t i = 0; i < bin_v1_test_bin_qspi_len / 4; i++)
        *(volatile uint32_t *)(0x04000000 + i * 4) = *(uint32_t *)(bin_v1_test_bin_qspi + i * 4);

    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000000);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000001);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000002);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000003);

    DBGPRINTLN_CTX("QSPI Dest %08X", get_family_name);
    DBGPRINTLN_CTX("Device: %s%hu", get_family_name((DEVINFO->PART & _DEVINFO_PART_DEVICE_FAMILY_MASK) >> _DEVINFO_PART_DEVICE_FAMILY_SHIFT), (DEVINFO->PART & _DEVINFO_PART_DEVICE_NUMBER_MASK) >> _DEVINFO_PART_DEVICE_NUMBER_SHIFT);
    */

    //DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000000);
    //DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000001);
    //DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000002);
    //DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000003);
    //DBGPRINTLN_CTX("Boot RD: %02X", *(volatile uint8_t *)0x0FE10000);
    //DBGPRINTLN_CTX("Data RD: %02X", *(volatile uint8_t *)0x0FE00000);

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00000000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);

    //DBGPRINTLN_CTX("QSPI RD: %08X", *(volatile uint32_t *)0xC0000000);
    //DBGPRINTLN_CTX("QSPI RD: %08X", *(volatile uint32_t *)0xC0000004);

    WIFI_SELECT();
    WIFI_RESET();
    delay_ms(10);
    WIFI_UNRESET();
    delay_ms(100);
    //WIFI_UNSELECT();

    tft_bl_init(2000);
    tft_bl_set(0.25);

    ILI9488_UNRESET();
    ili9488_init();
    DBGPRINTLN_CTX("Display: 0x%06X", ili9488_read_id());

    ili9488_display_on();
    ili9488_set_rotation(0);
    ili9488_fill_screen(RGB565_DARKGREY);

    terminal_t *terminal = ili9488_terminal_create(10, 250, 9, 300, &xSans9pFont, RGB565_GREEN, RGB565_BLACK);
    if(!terminal)
    {
        DBGPRINTLN_CTX("could not allocate start terminal");
        while(1);
    }

    ili9488_printf(&xSans18pFont, 10, 10, RGB565_WHITE, RGB565_DARKGREY, "Display is the wey");

    ili9488_draw_rectangle(10, 65, 295 + 15 + 5, 75 + ili9488_get_text_height(&xSans9pFont, 7), RGB565_DARKGREEN, 1);

    textbox_t *textbox = ili9488_textbox_create(15, 70, 7, 295, &xSans9pFont, RGB565_BLUE, RGB565_WHITE);
    if(!textbox)
    {
        DBGPRINTLN_CTX("could not allocate start textbox");
        while(1);
    }

    while(1)
    {
        static uint8_t ubLastBtn1State = 0;
        static uint8_t ubLastBtn2State = 0;
        static uint8_t ubLastBtn3State = 0;

        if(BTN_1_STATE() && (ubLastBtn1State != 1))
        {
            ili9488_draw_image(&xSurpriseImage, 0, 0);
            ubLastBtn1State = 1;
        }
        else if(!BTN_1_STATE() && (ubLastBtn1State != 0))
        {
            ili9488_fill_screen(RGB565_DARKGREY);
            ili9488_printf(&xSans18pFont, 10, 10, RGB565_WHITE, RGB565_DARKGREY, "Display is the wey");
            ili9488_draw_rectangle(10, 65, 295 + 15 + 5, 75 + ili9488_get_text_height(&xSans9pFont, 7), RGB565_DARKGREEN, 1);
            ili9488_textbox_clear(textbox);
            ili9488_terminal_printf(terminal, 1, "\nA nice surprise was shown...");
            ubLastBtn1State = 0;
        }

        if(BTN_2_STATE() && (ubLastBtn2State != 1))
        {
            ili9488_fill_screen(RGB565_WHITE);
            for(uint8_t x = 0; x < 4; x++)
            {
                for(uint8_t y = 0; y < 6; y++)
                {
                    ili9488_draw_image(&xPepeImage, 32 + (x * 64), 32 + (y * 64));
                }
            }
            ubLastBtn2State = 1;
        }
        else if(!BTN_2_STATE() && (ubLastBtn2State != 0))
        {
            ili9488_fill_screen(RGB565_DARKGREY);
            ili9488_printf(&xSans18pFont, 10, 10, RGB565_WHITE, RGB565_DARKGREY, "Display is the wey");
            ili9488_draw_rectangle(10, 65, 295 + 15 + 5, 75 + ili9488_get_text_height(&xSans9pFont, 7), RGB565_DARKGREEN, 1);
            ili9488_textbox_clear(textbox);
            ili9488_terminal_printf(terminal, 1, "\nPepe army was shown...");
            ubLastBtn2State = 0;
        }

        if(BTN_3_STATE() && (ubLastBtn3State != 1))
        {
            ubLastBtn3State = 1;
        }
        else if(!BTN_3_STATE() && (ubLastBtn3State != 0))
        {
            uint8_t ubNBytes = 38;

            uint8_t ubBuf[ubNBytes];
            memset(ubBuf, 0x00, ubNBytes);

            ubBuf[0] = 0x03;
            ubBuf[1] = 0x05;

            ili9488_terminal_printf(terminal, 1, "\nTransfering %d byte(s) to wifi-coprocessor...", ubNBytes);
            DBGPRINTLN_CTX("Transfering %d byte(s) to wifi-coprocessor...", ubNBytes);
            DBGPRINTLN_CTX("Content:");
            for(uint8_t ubI = 0; ubI < ubNBytes; ubI++)
                DBGPRINTLN_CTX("\t0x%02X", ubBuf[ubI]);

            WIFI_UNSELECT();
            delay_ms(10);

            WIFI_SELECT();
            usart0_spi_transfer(ubBuf, ubNBytes, ubBuf);
            WIFI_UNSELECT();


            DBGPRINTLN_CTX("Received:");
            for(uint8_t ubI = 0; ubI < ubNBytes; ubI++)
                DBGPRINTLN_CTX("\t0x%02X", ubBuf[ubI]);

            ubLastBtn3State = 0;
        }

        static uint64_t ullLastTextboxUpdate = 0;
        if(g_ullSystemTick > (ullLastTextboxUpdate + 5000))
        {
            /*
            DBGPRINTLN_CTX("ADC Temp: %.2f", adc_get_temperature());
            DBGPRINTLN_CTX("EMU Temp: %.2f", emu_get_temperature());

            DBGPRINTLN_CTX("LFXO: %.2f pF", cmu_lfxo_get_cap());

            DBGPRINTLN_CTX("HFXO Startup: %.2f pF", cmu_hfxo_get_startup_cap());
            DBGPRINTLN_CTX("HFXO Startup: %.2f uA", cmu_hfxo_get_startup_current());
            DBGPRINTLN_CTX("HFXO Steady: %.2f pF", cmu_hfxo_get_steady_cap());
            DBGPRINTLN_CTX("HFXO Steady: %.2f uA", cmu_hfxo_get_steady_current());
            DBGPRINTLN_CTX("HFXO PMA [%03X]: %.2f uA", cmu_hfxo_get_pma_ibtrim(), cmu_hfxo_get_pma_current());
            DBGPRINTLN_CTX("HFXO PDA [%03X]: %.2f uA", cmu_hfxo_get_pda_ibtrim(1), cmu_hfxo_get_pda_current(0));

            DBGPRINTLN_CTX("RTCC Time: %lu", rtcc_get_time());

            DBGPRINTLN_CTX("Battery Charging: %hhu", BAT_CHRG());
            DBGPRINTLN_CTX("Battery Standby: %hhu", BAT_STDBY());
            DBGPRINTLN_CTX("3V3 Fault: %hhu", VREG_ERR());

            DBGPRINTLN_CTX("Button states (1|2|3): %hhu|%hhu|%hhu", BTN_1_STATE(), BTN_2_STATE(), BTN_3_STATE());
            */
            ili9488_textbox_set_color(textbox, RGB565_BLUE, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "\rADC Temp: ");
            ili9488_textbox_set_color(textbox, RGB565_RED, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "%.2f\n\r", adc_get_temperature());
            ili9488_textbox_set_color(textbox, RGB565_BLUE, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "EMU Temp: ");
            ili9488_textbox_set_color(textbox, RGB565_RED, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "%.2f\n\r", emu_get_temperature());
            ili9488_textbox_set_color(textbox, RGB565_BLUE, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "RTCC Time: ");
            ili9488_textbox_set_color(textbox, RGB565_RED, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "%lu\n\r", rtcc_get_time());
            ili9488_textbox_set_color(textbox, RGB565_BLUE, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "Battery Charging: ");
            ili9488_textbox_set_color(textbox, RGB565_RED, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "%hhu\n\r", BAT_CHRG());
            ili9488_textbox_set_color(textbox, RGB565_BLUE, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "Battery Standby: ");
            ili9488_textbox_set_color(textbox, RGB565_RED, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "%hhu\n\r", BAT_STDBY());
            ili9488_textbox_set_color(textbox, RGB565_BLUE, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "3V3 Fault: ");
            ili9488_textbox_set_color(textbox, RGB565_RED, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "%hhu\n\r", VREG_ERR());
            ili9488_textbox_set_color(textbox, RGB565_BLUE, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "Button states (1|2|3): ");
            ili9488_textbox_set_color(textbox, RGB565_RED, RGB565_WHITE);
            ili9488_textbox_printf(textbox, "%hhu|%hhu|%hhu\n", BTN_1_STATE(), BTN_2_STATE(), BTN_3_STATE());

            ullLastTextboxUpdate = g_ullSystemTick;
        }

        static uint64_t ullLastTask = 0;
        if(g_ullSystemTick > (ullLastTask + 5000))
        {
            //play_sound(4500, 10);

            static uint8_t ubLastState = 0;
            if(!ubLastState)
            {
            }
            else
            {
            }
            ubLastState = !ubLastState;

            ullLastTask = g_ullSystemTick;
        }
    }

    return 0;
}