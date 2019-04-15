#ifndef __PN532_H__
#define __PN532_H__

#include <em_device.h>
#include "usart.h"
#include "gpio.h"
#include "systick.h"
#include "debug_macros.h"

#define PN532_DEBUG
//#define PN532_READY_HW

// PN532 frame
#define PN532_PREAMBLE      0x00
#define PN532_STARTCODE1    0x00
#define PN532_STARTCODE2    0xFF
#define PN532_HOSTPN532     0xD4
#define PN532_PN532HOST     0xD5
#define PN532_POSTAMBLE     0x00

// return errors
#define PN532_INVALID_FRAME 0x00
#define PN532_NO_SPACE      0x00
#define PN532_INVALID_ACK   0x00
#define PN532_TIMEOUT       0x00
#define PN532_ERROR_FRAME   0x00

// PN532 timeouts
#define PN532_ACKTIMEOUT        10
#define PN532_RESPONSETIMEOUT   1000
#define PN532_WAKETIMEOUT       100

// PN532 Commands
// Miscellaneous
#define PN532_COMMAND_DIAGNOSE              0x00
#define PN532_COMMAND_GETFIRMWAREVERSION    0x02
#define PN532_COMMAND_GETGENERALSTATUS      0x04
#define PN532_COMMAND_READREGISTER          0x06
#define PN532_COMMAND_WRITEREGISTER         0x08
#define PN532_COMMAND_READGPIO              0x0C
#define PN532_COMMAND_WRITEGPIO             0x0E
#define PN532_COMMAND_SETSERIALBAUDRATE     0x10
#define PN532_COMMAND_SETPARAMETERS         0x12
#define PN532_COMMAND_SAMCONFIGURATION      0x14
#define PN532_COMMAND_POWERDOWN             0x16
// RF communication
#define PN532_COMMAND_RFCONFIGURATION       0x32
#define PN532_COMMAND_RFREGULATIONTEST      0x58
// Initiator
#define PN532_COMMAND_INJUMPFORDEP          0x56
#define PN532_COMMAND_INJUMPFORPSL          0x46
#define PN532_COMMAND_INLISTPASSIVETARGET   0x4A
//#define PN532_RESPONSE_INLISTPASSIVETARGET  0x4B
#define PN532_COMMAND_INATR                 0x50
#define PN532_COMMAND_INPSL                 0x4E
#define PN532_COMMAND_INDATAEXCHANGE        0x40
//#define PN532_RESPONSE_INDATAEXCHANGE       0x41
#define PN532_COMMAND_INCOMMUNICATETHRU     0x42
#define PN532_COMMAND_INDESELECT            0x44
#define PN532_COMMAND_INRELEASE             0x52
#define PN532_COMMAND_INSELECT              0x54
#define PN532_COMMAND_INAUTOPOLL            0x60
// Target
#define PN532_COMMAND_TGINITASTARGET        0x8C
#define PN532_COMMAND_TGSETGENERALBYTES     0x92
#define PN532_COMMAND_TGGETDATA             0x86
#define PN532_COMMAND_TGSETDATA             0x8E
#define PN532_COMMAND_TGSETMETADATA         0x94
#define PN532_COMMAND_TGGETINITIATORCOMMAND 0x88
#define PN532_COMMAND_TGRESPONSETOINITIATOR 0x90
#define PN532_COMMAND_TGGETTARGETSTATUS     0x8A

// GPIO
#define PN532_PORT_P3    0x00
#define PN532_PORT_P7    0x01
#define PN532_PORT_I0I1  0x02
#define PN532_P35   5
#define PN532_P34   4
#define PN532_P33   3
#define PN532_P32   2
#define PN532_P31   1
#define PN532_P30   0
#define PN532_P72   2
#define PN532_P71   1
#define PN532_I1    1
#define PN532_I0    0

// baud rates
#define PN532_BAUD_9_6K     0x00
#define PN532_BAUD_19_2K    0x01
#define PN532_BAUD_38_4K    0x02
#define PN532_BAUD_57_6K    0x03
#define PN532_BAUD_115_2K   0x04
#define PN532_BAUD_230_4K   0x05
#define PN532_BAUD_460_8K   0x06
#define PN532_BAUD_921_6K   0x07
#define PN532_BAUD_1_288M   0x08

// parameter flags (pn532_set_parameters)
/*
    table 15. Default values of internal flags
    Property                Default value
    fNADUsed                Not used
    fDIDUsed                Not used
    fAutomaticATR_RES       Yes, automatic
    fAutomaticRATS          Yes, automatic
    fISO14443-4_PICC        Yes, enabled
    fRemovePrePostAmble     No
*/
#define PN532_FLAG_NADUSED          0x01
#define PN532_FLAG_DIDUSED          0x02
#define PN532_FLAG_AUTOLATR_RES     0x04
#define PN532_FLAG_AUTORATS         0x10
#define PN532_FLAG_ISO14443_4_PICC  0x20
#define PN532_FLAG_RMPREPOSTAMBLE   0x40

// sam configuration
#define PN532_SAM_NORMAL_MODE   0x01
#define PN532_SAM_VIRTUAL_CARD  0x02
#define PN532_SAM_WIRPN532_PN532HOSTED_CARD    0x03
#define PN532_SAM_DUAL_CARD     0x04
#define PN532_SAM_IRQ           0x01
#define PN532_SAM_NOT_IRQ       0x00
#define PN532_SAM_TIMEOUT_MIN   0x00    // timeout LSB = 50ns
#define PN532_SAM_TIMEOUT_1S    0x14    // 50ns * 20 = 1s
#define PN532_SAM_TIMEOUT_MAX   0xFF    // timeoput = 12.75 sec

// power down
#define PN532_WAKE_I2C      0x80
#define PN532_WAKE_GPIO     0x40
#define PN532_WAKE_SPI      0x20
#define PN532_WAKE_HSU      0x10
#define PN532_WAKE_RF_LVL   0x08
#define PN532_WAKE_INT1     0x02
#define PN532_WAKE_INT2     0x01
#define PN532_WAKE_IRQ      0x01
#define PN532_WAKE_NO_IRQ   0x00

// RFConfiguration
#define PN532_RF_ITEM_RF_FIELD      0x01
#define PN532_RF_FIELD_AUTORFCA     0x02
#define PN532_RF_FIELD_RFON         0x01

#define PN532_RF_ITEM_TIMINGS       0x02
#define PN532_RF_ITEM_MAXRTYCOM     0x04
#define PN532_RF_ITEM_MAXRETRIES    0x05
#define PN532_RF_ITEM_ANSETTINGBAUD106KTA           0x0A
#define PN532_RF_ITEM_ANSETTINGBAUD212K424K         0x0B
#define PN532_RF_ITEM_ANSETTINGTB                   0x0C
#define PN532_RF_ITEM_ANSETTINGBAUD212K424K848KWISO 0x0D

// RF regulation test
#define PN532_RFREG_106K    0x00
#define PN532_RFREG_212K    0x10
#define PN532_RFREG_424K    0x20
#define PN532_RFREG_848K    0x30
#define PN532_RFREG_MIFARE  0x00
#define PN532_RFREG_FELICA  0x02

// inlist target
#define PN532_MIFARE_ISO14443A 0x00

// pn532 interface funcions
uint8_t pn532_init();

void pn532_wake();

uint8_t pn532_write_command(uint8_t ubCommand, uint8_t* ubParameters, uint8_t ubNParameters);
uint8_t pn532_read_response(uint8_t ubCommand, uint8_t* ubBuf, uint8_t ubLength);

void pn532_write_frame(uint8_t* ubPayload, uint8_t ubLength);
uint8_t pn532_read_frame(uint8_t* ubPayload, uint8_t ubLength);

uint8_t pn532_read_ack();
void pn532_write_ack(uint8_t ubNack);

uint8_t pn532_ready();

// pn532 RF communication commands
uint32_t pn532_get_version();

void pn532_Get_General_Status();    // TODO

uint8_t pn532_read_register(uint16_t usAddr);
uint8_t pn532_write_register(uint16_t usAddr, uint8_t ubValue);

uint8_t pn532_read_gpio(uint8_t ubPort);
uint8_t pn532_write_gpio(uint8_t ubPort, uint8_t ubPins);

uint8_t pn532_set_serial_baud(uint8_t ubBaud);   // useless in SPI

uint8_t pn532_set_parameters(uint8_t ubFlags);

uint8_t pn532_sam_configuration(uint8_t ubMode, uint8_t ubTimeout, uint8_t ubIrq);

uint8_t pn532_power_down(uint8_t ubWakeSrcs, uint8_t ubGenIrq);

// RF communication
uint8_t pn532_rf_configuration(uint8_t ubCfgItem, uint8_t *pubCfgData, uint8_t ubCfgDataLen);

uint8_t pn532_set_passive_activation_retries(uint8_t ubRetries);

uint8_t pn532_set_rf_field(uint8_t ubRfFieldFlags);

uint8_t pn532_rf_regulation_test(uint8_t ubSpeedAndFramming);

// Initiator
uint8_t pn532_inlist_passive_target(uint8_t ubMaxTg, uint8_t ubBrTg, uint8_t *pubInitData, uint8_t ubInitDataLen, uint8_t *pubTg1, uint8_t *pubTg2, uint8_t ubTgDataLen);
uint8_t pn532_read_passive_target_id(uint8_t *pubUid, uint8_t *pubUidLen);


#endif  // __PN532_H__