#ifndef __BOOT_OTA_H__
#define __BOOT_OTA_H__

#include <c_types.h>
#include <stdlib.h>
#include "ets_func.h"
#include "user_interface.h"
#include "spi_flash.h"
#include "ip_addr.h"
#include "espconn.h"
#include "md5.h"
#include "boot.h"

#define OTA_STATUS_RESOLVE 				0x00000001
#define OTA_STATUS_CONNECT 				0x00000002
#define OTA_STATUS_REQUEST	 			0x00000003
#define OTA_STATUS_WRITE_FLASH 			0x00000004
#define OTA_STATUS_FINISH 				0x00000005
#define OTA_STATUS_ERROR_FLAG			0x80000000
#define OTA_STATUS_ERROR_TCP_FLAG		0x40000000
#define OTA_STATUS_ERROR_MALLOC_FLAG	0x20000000
#define OTA_STATUS_ERROR_TIMEOUT_FLAG	0x10000000
#define OTA_STATUS_ERROR_HTTP_FLAG		0x08000000

typedef void (*ota_event_fn_t)(uint32_t, void*);

typedef struct {
	uint32_t flash_addr;
	uint32_t flash_erased_sector;
	uint8_t extra_count;
	uint8_t extra_bytes[4];
} ota_write_status_t;

typedef struct {
	ota_write_status_t write_status;
	ota_event_fn_t event_monitor;
	struct espconn* conn;
	uint8_t* http_request;
	uint16_t http_request_size;
	uint32_t current_op;
	uint8_t rom_id;
	uint32_t bin_size;
	uint32_t read_size;
} ota_status_t;

uint8_t ICACHE_FLASH_ATTR ota_boot_get_config(boot_config_t *conf);
uint8_t ICACHE_FLASH_ATTR ota_boot_set_config(boot_config_t *conf);
uint8_t ICACHE_FLASH_ATTR ota_boot_get_rtc_config(boot_rtc_config_t *conf);
uint8_t ICACHE_FLASH_ATTR ota_boot_set_rtc_config(boot_rtc_config_t *conf);

uint8_t ICACHE_FLASH_ATTR ota_write_init(ota_write_status_t *status, uint32_t addr);
uint8_t ICACHE_FLASH_ATTR ota_write(ota_write_status_t *status, uint8_t *data, uint32_t len);
uint8_t ICACHE_FLASH_ATTR ota_write_end(ota_write_status_t *status);

uint8_t ICACHE_FLASH_ATTR ota_start(const char *host, uint16_t port, uint16_t ssl_buffer_size, const char *device, const char* device_id, uint32_t current_version, uint32_t new_version, uint32_t current_timestamp, ota_event_fn_t event_monitor);

#endif
