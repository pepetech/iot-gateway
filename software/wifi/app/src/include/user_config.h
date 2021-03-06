#include <c_types.h>
#include <stdlib.h>
#include "boot_ota.h"
#include "user_interface.h"
#include "wpa2_enterprise.h"
#include "ets_func.h"
#include "registers.h"
#include "uart.h"
#include "debug_macros.h"
#include "spi_flash.h"
#include "ip_addr.h"
#include "espconn.h"
#include "ping.h"
#include "md5.h"
#include "sha1.h"
#include "sntp.h"
#include "boot_ota.h"

#define SSL_BUFFER_SIZE 4096

#define SSID_LIST_SECTOR        0x3F9
#define SSL_CA_SECTOR           0x3FA
#define RF_CAL_DATA_SECTOR      0x3FB
#define SDK_INIT_DATA_SECTOR    0x3FC

#define DEVICE "ahaha"

#define OTA_HOST    "node1.jsilvaiot.com"
#define OTA_PORT    8880


#define ADC_MODE_LOCATION   107
#define ADC_MODE_VDD        0xFF
#define ADC_MODE_TOUT_AUTO  0x00
#define ADC_MODE_TOUT_30    0x1E
#define ADC_MODE_TOUT_31    0x1F
#define ADC_MODE_TOUT_32    0x20
#define ADC_MODE_TOUT_33    0x21
#define ADC_MODE_TOUT_34    0x22
#define ADC_MODE_TOUT_35    0x23
#define ADC_MODE_TOUT_36    0x24

#define XTAL_FREQ_LOCATION  48
#define XTAL_FREQ_24M       0x02
#define XTAL_FREQ_26M       0x01
#define XTAL_FREQ_40M       0x00


#define SPI_BUFFER_SIZE 8192

#define SPI_TASK_SIZE   20
#define SPI_TASK_PRIO   2

#define SPI_SIGNAL_READ_STATUS_DONE     0
#define SPI_SIGNAL_WRITE_STATUS_DONE    1
#define SPI_SIGNAL_READ_DATA_DONE       2
#define SPI_SIGNAL_WRITE_DATA_DONE      3

#define SPI_STATUS_WRITTABLE_MASK   0x00000000 // Set writtable bits to 1
