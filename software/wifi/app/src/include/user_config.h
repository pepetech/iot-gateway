#include <c_types.h>
#include <stdlib.h>
#include "boot_ota.h"
#include "user_interface.h"
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
#include "mqtt.h"
#include "ws2812.h"
#include "boot_ota.h"

#define DEVICE             "leiria_tvled"

#define SSL_BUFFER_SIZE 4096

#define SSID_LIST_SECTOR        0x3F9
#define SSL_CA_SECTOR           0x3FA
#define RF_CAL_DATA_SECTOR      0x3FB
#define SDK_INIT_DATA_SECTOR    0x3FC

#define OTA_HOST    "node1.jsilvaiot.com"
#define OTA_PORT    8880

#define MQTT_HOST       "mqtt.jsilvaiot.com"
#define MQTT_PORT       8883
#define MQTT_USER       DEVICE
#define MQTT_PASS       "eJ5yCQpF88dF4HGm"

#define MQTT_BASE_TOPIC                 "devices/" DEVICE
#define MQTT_ID_TOPIC                       MQTT_BASE_TOPIC "/id"
#define MQTT_STATUS_TOPIC                   MQTT_BASE_TOPIC "/status"
#define MQTT_ENABLE_TOPIC                   MQTT_BASE_TOPIC "/enable"
#define MQTT_BASE_DATA_TOPIC                MQTT_BASE_TOPIC "/data"
#define MQTT_DATA_COLOR_TOPIC                   MQTT_BASE_DATA_TOPIC "/color"
#define MQTT_DATA_EFFECT_TOPIC                  MQTT_BASE_DATA_TOPIC "/effect"
#define MQTT_BASE_FIRMWARE_TOPIC            MQTT_BASE_TOPIC "/firmware"
#define MQTT_FIRMWARE_LATEST_TOPIC              MQTT_BASE_FIRMWARE_TOPIC "/latest"

#define ADC_MODE_VDD 0xFF
#define ADC_MODE_TOUT_AUTO 0x00
#define ADC_MODE_TOUT_30 0x1E
#define ADC_MODE_TOUT_31 0x1F
#define ADC_MODE_TOUT_32 0x20
#define ADC_MODE_TOUT_33 0x21
#define ADC_MODE_TOUT_34 0x22
#define ADC_MODE_TOUT_35 0x23
#define ADC_MODE_TOUT_36 0x24

#define WIFI_LED_BIT    0
#define WIFI_LED_IOMUX  IOMUX_GPIO0
#define WIFI_LED_FUNC   IOMUX_GPIO0_FUNC_GPIO

#define RGB_DATA_BIT    2
#define RGB_DATA_IOMUX  IOMUX_GPIO2
#define RGB_DATA_FUNC   IOMUX_GPIO2_FUNC_GPIO
#define RGB_COUNT 24
