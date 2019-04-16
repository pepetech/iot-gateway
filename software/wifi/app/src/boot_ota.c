#include "boot_ota.h"

#define	UPGRADE_FLAG_IDLE	0x00
#define	UPGRADE_FLAG_START	0x01
#define	UPGRADE_FLAG_FINISH	0x02

#define OTA_NETWORK_TIMEOUT 10000

#define HTTP_REQUEST_FORMAT "GET /fwdownload HTTP/1.1\r\n"				\
							"Host: %s:%d\r\n"							\
							"Cache-Control: no-cache\r\n"				\
							"User-Agent: ESP8266\r\n"					\
							"Accept: application/octet-stream\r\n"		\
							"Connection: keep-alive\r\n"				\
							"X-SSL-Buffer-Size: %d\r\n"					\
							"X-Device: %s\r\n"							\
							"X-Device-ID: %s\r\n"						\
							"X-Current-Version: %d\r\n"					\
							"X-New-Version: %d\r\n"						\
							"X-App-ID: %d\r\n"							\
							"X-Current-Timestamp: %d\r\n"				\
							"X-Signature: "MD5STRL"\r\n"				\
							"\r\n"


static ota_status_t* ota_update_status;
static ets_timer_t ota_update_timer;

static uint8_t ICACHE_FLASH_ATTR calc_chksum(uint8_t *start, uint8_t *end)
{
	uint8_t chksum = CHKSUM_INIT;

	while(start < end)
	{
		chksum ^= *start;
		start++;
	}

	return chksum;
}

uint8_t ota_boot_get_config(boot_config_t *conf)
{
    if(!conf)
        return 0;

    if(spi_flash_read(BOOT_CONFIG_SECTOR * SECTOR_SIZE, (uint32_t*)conf, sizeof(boot_config_t)) != SPI_FLASH_RESULT_OK)
        return 0;

	return 1;
}
uint8_t ota_boot_set_config(boot_config_t *conf)
{
    if(!conf)
        return 0;

	uint8_t *buffer = (uint8_t*)ets_zalloc(SECTOR_SIZE);

	if (!buffer)
        return 0;

	conf->chksum = calc_chksum((uint8_t*)conf, (uint8_t*)&conf->chksum);

    if(spi_flash_read(BOOT_CONFIG_SECTOR * SECTOR_SIZE, (uint32_t*)((void*)buffer), SECTOR_SIZE) != SPI_FLASH_RESULT_OK)
    {
        ets_free(buffer);

        return 0;
    }

	ets_memcpy(buffer, conf, sizeof(boot_config_t));

	if(spi_flash_erase_sector(BOOT_CONFIG_SECTOR) != SPI_FLASH_RESULT_OK)
    {
        ets_free(buffer);

        return 0;
    }

    if(spi_flash_write(BOOT_CONFIG_SECTOR * SECTOR_SIZE, (uint32_t*)((void*)buffer), SECTOR_SIZE) != SPI_FLASH_RESULT_OK)
    {
        ets_free(buffer);

        return 0;
    }

	ets_free(buffer);

	return 1;
}
uint8_t ota_boot_get_rtc_config(boot_rtc_config_t *conf)
{
    if(!conf)
        return 0;

    if (!system_rtc_mem_read(BOOT_RTC_ADDR, conf, sizeof(boot_rtc_config_t)))
        return 0;

	return conf->magic == BOOT_RTC_MAGIC && conf->chksum == calc_chksum((uint8_t*)conf, (uint8_t*)&conf->chksum);
}
uint8_t ota_boot_set_rtc_config(boot_rtc_config_t *conf)
{
	conf->chksum = calc_chksum((uint8_t*)conf, (uint8_t*)&conf->chksum);

	return system_rtc_mem_write(BOOT_RTC_ADDR, conf, sizeof(boot_rtc_config_t));
}

uint8_t ota_write_init(ota_write_status_t *status, uint32_t addr)
{
    if(!status)
        return 0;

    status->flash_addr = addr;
	status->flash_erased_sector = (addr / SECTOR_SIZE) - 1;

    return 1;
}
uint8_t ota_write(ota_write_status_t *status, uint8_t *data, uint32_t len)
{
    if(!status)
        return 0;

    if(!len)
        return 1;

	if (!data)
		return 0;

	uint8_t* buffer = (uint8_t*)ets_zalloc(len + status->extra_count);

	if (!buffer)
        return 0;

	if (status->extra_count)
		ets_memcpy(buffer, status->extra_bytes, status->extra_count);

	ets_memcpy(buffer + status->extra_count, data, len);

	len += status->extra_count;
	status->extra_count = len % 4;
	len -= status->extra_count;

	ets_memcpy(status->extra_bytes, buffer + len, status->extra_count);

	while (((status->flash_addr + len - 1) / SECTOR_SIZE) > status->flash_erased_sector)
    {
		status->flash_erased_sector++;

		if(spi_flash_erase_sector(status->flash_erased_sector) != SPI_FLASH_RESULT_OK)
        {
            ets_free(buffer);

            return 0;
        }
	}


	if (spi_flash_write(status->flash_addr, (uint32_t*)((void*)buffer), len) != SPI_FLASH_RESULT_OK)
    {
        ets_free(buffer);

        return 0;
    }

	status->flash_addr += len;

	ets_free(buffer);

	return 1;
}
uint8_t ota_write_end(ota_write_status_t *status)
{
    if(!status->extra_count)
	   return 1;

	for (uint8_t i = status->extra_count; i < 4; i++)
		status->extra_bytes[i] = 0xFF;

    status->extra_count = 0;

    return ota_write(status, status->extra_bytes, 4);
}

static void ICACHE_FLASH_ATTR ota_callback_disconnect(void *arg);
static void ICACHE_FLASH_ATTR ota_callback_receive(void *arg, char *data, uint16_t data_size);
static void ICACHE_FLASH_ATTR ota_callback_reconnect(void *arg, int8_t err);
static void ICACHE_FLASH_ATTR ota_callback_connect(void *arg);
static void ICACHE_FLASH_ATTR ota_callback_dns_resolve(const char *name, ip_addr_t *ip, void *arg);
static void ICACHE_FLASH_ATTR ota_callback_timeout(void *arg);

void ota_callback_disconnect(void *arg)
{
	struct espconn* conn = (struct espconn*)arg;

	ets_timer_disarm(&ota_update_timer);

	if (conn)
	{
		if (conn->proto.tcp)
			ets_free(conn->proto.tcp);

		ets_free(conn);
	}

	ota_event_fn_t event_monitor = ota_update_status->event_monitor;
	uint32_t current_op = ota_update_status->current_op;
	uint8_t rom_id = ota_update_status->rom_id;

	if(ota_update_status->http_request)
	{
		ets_free(ota_update_status->http_request);

		ota_update_status->http_request = 0;
	}

	ets_free(ota_update_status);

	if(current_op == OTA_STATUS_FINISH)
		event_monitor(current_op, &rom_id);
}
void ota_callback_receive(void *arg, char *data, uint16_t data_size)
{
	ets_timer_disarm(&ota_update_timer);

	if (!ota_update_status->bin_size)
	{
		if(data_size < 8 || ets_strncmp(data, "HTTP/1.1", 8))
		{
			ota_update_status->event_monitor(OTA_STATUS_ERROR_HTTP_FLAG | ota_update_status->current_op, 0);

			ets_timer_setfn(&ota_update_timer, (ets_timer_callback_fn_t)espconn_secure_disconnect, ota_update_status->conn);
			ets_timer_arm_new(&ota_update_timer, 10, 0, 1);

			return;
		}

		uint16_t status_code = strtol(data + 9, 0, 10);
		uint32_t body_size = 0;
		char *body_size_str = (char*)ets_strstr(data, "\r\nContent-Length: ");
		uint8_t *body_start = (uint8_t*)ets_strstr(data, "\r\n\r\n"); // 4

		if(body_size_str)
			body_size = strtoul(body_size_str + 18, 0, 10);

		if(!body_size_str || !body_size || !body_start)
		{
			ota_update_status->event_monitor(OTA_STATUS_ERROR_HTTP_FLAG | status_code, 0);

			ets_timer_setfn(&ota_update_timer, (ets_timer_callback_fn_t)espconn_secure_disconnect, ota_update_status->conn);
			ets_timer_arm_new(&ota_update_timer, 10, 0, 1);

			return;
		}

		if(status_code != 200)
		{
			char *error_message = (char*)ets_malloc(body_size + 1);

			if (!error_message)
			{
				ota_update_status->event_monitor(OTA_STATUS_ERROR_MALLOC_FLAG | body_size + 1, 0);

				ets_timer_setfn(&ota_update_timer, (ets_timer_callback_fn_t)espconn_secure_disconnect, ota_update_status->conn);
				ets_timer_arm_new(&ota_update_timer, 10, 0, 1);

				return;
			}

			ets_memcpy(error_message, body_start + 4, body_size);

			error_message[body_size] = 0;

			ota_update_status->event_monitor(OTA_STATUS_ERROR_HTTP_FLAG | status_code, error_message);

			ets_free(error_message);

			ets_timer_setfn(&ota_update_timer, (ets_timer_callback_fn_t)espconn_secure_disconnect, ota_update_status->conn);
			ets_timer_arm_new(&ota_update_timer, 10, 0, 1);

			return;
		}

		ota_update_status->bin_size = body_size;

		data_size -= (uint32_t)body_start + 4 - (uint32_t)data;
		data = (char*)body_start + 4;

		ota_update_status->event_monitor(ota_update_status->current_op, &ota_update_status->bin_size);
		ota_update_status->current_op = OTA_STATUS_WRITE_FLASH;
	}

	ota_update_status->read_size += data_size;

	ota_update_status->event_monitor(ota_update_status->current_op, &ota_update_status->read_size);

	if (!ota_write(&ota_update_status->write_status, (uint8_t*)data, data_size))
	{
		ota_update_status->event_monitor(OTA_STATUS_ERROR_FLAG | ota_update_status->current_op, 0);

		ets_timer_setfn(&ota_update_timer, (ets_timer_callback_fn_t)espconn_secure_disconnect, ota_update_status->conn);
		ets_timer_arm_new(&ota_update_timer, 10, 0, 1);

		return;
	}

	if (ota_update_status->read_size == ota_update_status->bin_size)
	{
		ota_update_status->current_op = OTA_STATUS_FINISH;

		ets_timer_setfn(&ota_update_timer, (ets_timer_callback_fn_t)espconn_secure_disconnect, ota_update_status->conn);
		ets_timer_arm_new(&ota_update_timer, 10, 0, 1);
	}
	else if (ota_update_status->conn->state != ESPCONN_READ)
	{
		ota_update_status->event_monitor(OTA_STATUS_ERROR_TCP_FLAG | ota_update_status->current_op, 0);

		ets_timer_setfn(&ota_update_timer, (ets_timer_callback_fn_t)espconn_secure_disconnect, ota_update_status->conn);
		ets_timer_arm_new(&ota_update_timer, 10, 0, 1);
	}
	else
	{
		ets_timer_setfn(&ota_update_timer, ota_callback_timeout, NULL);
		ets_timer_arm_new(&ota_update_timer, OTA_NETWORK_TIMEOUT, 0, 1);
	}
}
void ota_callback_reconnect(void *arg, int8_t err)
{
	ets_timer_disarm(&ota_update_timer);

	ota_update_status->event_monitor(OTA_STATUS_ERROR_TCP_FLAG | ota_update_status->current_op, &err);

	ota_callback_disconnect(ota_update_status->conn);
}
void ota_callback_connect(void *arg)
{
	ets_timer_disarm(&ota_update_timer);

	espconn_regist_disconcb(ota_update_status->conn, ota_callback_disconnect);
	espconn_regist_recvcb(ota_update_status->conn, ota_callback_receive);

	ota_update_status->event_monitor(ota_update_status->current_op, ota_update_status->http_request);
	ota_update_status->current_op = OTA_STATUS_REQUEST;

	espconn_secure_send(ota_update_status->conn, ota_update_status->http_request, ota_update_status->http_request_size);

	ets_timer_setfn(&ota_update_timer, ota_callback_timeout, NULL);
	ets_timer_arm_new(&ota_update_timer, OTA_NETWORK_TIMEOUT, 0, 1);

	ets_free(ota_update_status->http_request);

	ota_update_status->http_request = 0;
}
void ota_callback_dns_resolve(const char *name, ip_addr_t *ip, void *arg)
{
	if (!ip)
	{
		ota_update_status->event_monitor(OTA_STATUS_ERROR_FLAG | ota_update_status->current_op, 0);
		ota_callback_disconnect(ota_update_status->conn);

		return;
	}

	if(name)
		ets_memcpy(ota_update_status->conn->proto.tcp->remote_ip, ip, sizeof(ip_addr_t));

	espconn_regist_connectcb(ota_update_status->conn, ota_callback_connect);
	espconn_regist_reconcb(ota_update_status->conn, ota_callback_reconnect);

	ota_update_status->event_monitor(ota_update_status->current_op, ip);
	ota_update_status->current_op = OTA_STATUS_CONNECT;

	espconn_secure_connect(ota_update_status->conn);

	ets_timer_disarm(&ota_update_timer);
	ets_timer_setfn(&ota_update_timer, ota_callback_timeout, NULL);
	ets_timer_arm_new(&ota_update_timer, OTA_NETWORK_TIMEOUT, 0, 1);
}
void ota_callback_timeout(void *arg)
{
	ets_timer_disarm(&ota_update_timer);

	if(!ota_update_status)
		return;

	ota_update_status->event_monitor(OTA_STATUS_ERROR_TIMEOUT_FLAG | ota_update_status->current_op, 0);

	if(ota_update_status->current_op == OTA_STATUS_REQUEST || ota_update_status->current_op == OTA_STATUS_WRITE_FLASH)
		espconn_secure_disconnect(ota_update_status->conn);
	else if(ota_update_status->current_op == OTA_STATUS_CONNECT)
		ota_callback_disconnect(ota_update_status->conn);
}

uint8_t ota_start(const char *host, uint16_t port, uint16_t ssl_buffer_size, const char *device, const char* device_id, uint32_t current_version, uint32_t new_version, uint32_t current_timestamp, ota_event_fn_t event_monitor)
{
	if(!host || !port || !device || !device_id || !event_monitor)
		return 0;

	ota_update_status = (ota_status_t*)ets_zalloc(sizeof(ota_status_t));

	if(!ota_update_status)
		return 0;

	ota_update_status->event_monitor = event_monitor;

	boot_config_t cfg;

	if(!ota_boot_get_config(&cfg))
	{
		ets_free(ota_update_status);

		return 0;
	}

	if(cfg.rom_count < 2)
	{
		ets_free(ota_update_status);

		return 0;
	}

	ota_update_status->rom_id = !cfg.current_rom ? 1 : 0;

	if(!ota_write_init(&ota_update_status->write_status, cfg.roms[ota_update_status->rom_id]))
	{
		ets_free(ota_update_status);

		return 0;
	}

	ota_update_status->http_request = (uint8_t*)ets_zalloc(512);

	if(!ota_update_status->http_request)
	{
		ets_free(ota_update_status);

		return 0;
	}

	char request_hash[64];
	md5_context_t request_md5;
	uint8_t request_md5_digest[16];

	MD5Init(&request_md5);
	MD5Update(&request_md5, request_hash, ets_rosnprintf(request_hash, 64, PSTR("%s-%s-%d.%d-%d"), device, device_id, new_version, ota_update_status->rom_id + 1, current_timestamp));
	MD5Final(request_md5_digest, &request_md5);

	ota_update_status->http_request_size = ets_rosnprintf(
		(char*)ota_update_status->http_request,
		512,
		PSTR(HTTP_REQUEST_FORMAT),
		host, port,
		ssl_buffer_size,
		device,
		device_id,
		current_version,
		new_version,
		ota_update_status->rom_id + 1,
		current_timestamp,
		MD52STR(request_md5_digest)
	);

	ota_update_status->conn = (struct espconn*)ets_zalloc(sizeof(struct espconn));

	if(!ota_update_status->conn)
	{
		ets_free(ota_update_status->http_request);
		ets_free(ota_update_status);

		return 0;
	}

	ota_update_status->conn->proto.tcp = (esp_tcp*)ets_zalloc(sizeof(esp_tcp));

	if (!ota_update_status->conn->proto.tcp)
	{
		ets_free(ota_update_status->conn);
		ets_free(ota_update_status->http_request);
		ets_free(ota_update_status);

		return 0;
	}

	ota_update_status->conn->type = ESPCONN_TCP;
	ota_update_status->conn->state = ESPCONN_NONE;
	ota_update_status->conn->proto.tcp->local_port = espconn_port();
	ota_update_status->conn->proto.tcp->remote_port = port;
	*(uint32_t*)ota_update_status->conn->proto.tcp->remote_ip = 0;

	ota_update_status->current_op = OTA_STATUS_RESOLVE;

	err_t result = espconn_gethostbyname(ota_update_status->conn, host, (ip_addr_t*)ota_update_status->conn->proto.tcp->remote_ip, ota_callback_dns_resolve);

	if (result == ESPCONN_OK)
	{
		ota_callback_dns_resolve(NULL, (ip_addr_t*)ota_update_status->conn->proto.tcp->remote_ip, ota_update_status->conn);
	}
	else if (result != ESPCONN_INPROGRESS)
	{
		ets_free(ota_update_status->conn->proto.tcp);
		ets_free(ota_update_status->conn);
		ets_free(ota_update_status->http_request);
		ets_free(ota_update_status);

		return 0;
	}

	return 1;
}
