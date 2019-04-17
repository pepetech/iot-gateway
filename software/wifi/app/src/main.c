#include "user_config.h"

static ets_timer_t tick_timer;
static ets_timer_t sntp_timer;
static uint8_t update_scheduled = 0;
static uint32_t update_new_version = 0;

// Forward declarations
static void ICACHE_FLASH_ATTR tick_timer_callback(void *arg);
static void ICACHE_FLASH_ATTR sntp_timer_callback(void *arg);

static void ICACHE_FLASH_ATTR wifi_init();
static void ICACHE_FLASH_ATTR wifi_connect(const char *ssid, const char *password);
static void ICACHE_FLASH_ATTR wifi_scan_callback(void *arg, uint32_t status);
static void ICACHE_FLASH_ATTR wifi_event_handler(System_Event_t *evt);

static void ICACHE_FLASH_ATTR ssl_init();

static uint8_t ICACHE_FLASH_ATTR sdk_init(uint8_t adc_mode, uint8_t xtal_freq);

static void ICACHE_FLASH_ATTR sntp_start(uint32_t timeout);

static void ICACHE_FLASH_ATTR ota_event_handler(uint32_t status, void *arg);

static uint32_t ICACHE_FLASH_ATTR get_flash_sector_count();
static uint64_t ICACHE_FLASH_ATTR get_system_tick();

void ICACHE_FLASH_ATTR user_rf_pre_init();
uint32_t ICACHE_FLASH_ATTR user_rf_cal_sector_set();
void ICACHE_FLASH_ATTR user_init();
void ICACHE_FLASH_ATTR user_main();

// Functions
void tick_timer_callback(void *arg)
{
    uint64_t now_us = get_system_tick();
    uint64_t now_ms = now_us / 1000;

    static uint64_t last_sys_info_check = 0;

    if(now_ms - last_sys_info_check > 1000)
    {
        last_sys_info_check = now_ms;

        uint32_t curr_free_heap = system_get_free_heap_size();
        static uint32_t last_free_heap;

        if(last_free_heap != curr_free_heap)
        {
            last_free_heap = curr_free_heap;

            DBGPRINTLN_CTX("Free heap: [%lu]", curr_free_heap);
        }

        //BGPRINTLN_CTX("System tick: [%lu]", now_ms);
    }
}
void sntp_timer_callback(void *arg)
{
    uint32_t timestamp = sntp_get_current_timestamp();

    if (!timestamp)
        return;

    ets_timer_disarm(&sntp_timer);

    char* time_str = sntp_get_real_time(timestamp);

    time_str[ets_strlen(time_str) - 1] = 0;

    DBGPRINTLN_CTX("Got SNTP time: [%s]", time_str);

    if(!update_scheduled)
    {
        DBGPRINTLN_CTX("  Waiing...");
    }
    else
    {
        char device_id[16];
        uint8_t mac[6];

        wifi_get_macaddr(STATION_IF, mac);
        ets_sprintf(device_id, "%02X%02X%02X%02X%02X%02X", MAC2STR(mac));

        DBGPRINTLN_CTX("  Starting OTA update...");

        ota_start(OTA_HOST, OTA_PORT, SSL_BUFFER_SIZE, DEVICE, device_id, BUILD_VERSION, update_new_version, timestamp, ota_event_handler);
    }
}

void wifi_init()
{
    uint8_t mac[6];
    char hostname[32];

    wifi_set_event_handler_cb(wifi_event_handler);

    system_phy_set_max_tpw(0);

    wifi_set_opmode_current(NULL_MODE);
    wifi_set_opmode_current(STATION_MODE);

    wifi_get_macaddr(STATION_IF, mac);

    ets_snprintf(hostname, 32, "ESP_%02X%02X%02X%02X%02X%02X", MAC2STR(mac));
    DBGPRINTLN_CTX("Set station hostname to [%s] [%d]", hostname, wifi_station_set_hostname(hostname));

    wifi_station_disconnect();
}
void wifi_connect(const char* ssid, const char* password)
{
    struct station_config config;

    ets_memset(&config, 0, sizeof(config));

    ets_strncpy(config.ssid, ssid, 32);
    ets_strncpy(config.password, password, 64);

    wifi_station_disconnect();
    wifi_station_set_config_current(&config);
    wifi_station_connect();
}
void wifi_scan_callback(void *arg, uint32_t status)
{
    DBGPRINTLN_CTX("Scan done! [%d]", status);
    DBGPRINTLN_CTX("SSID list sector [0x%X000]", SSID_LIST_SECTOR);

    if (status)
        return;

    struct bss_info *bss_link = (struct bss_info *)arg;
    char* ssid_found = 0;
    char* password_found = 0;
    char* buffer = (char*)ets_zalloc(SPI_FLASH_SEC_SIZE);

    if(!buffer)
    {
        DBGPRINTLN_CTX("  Failed to allocate buffer!");

        return;
    }

    if(spi_flash_read(SSID_LIST_SECTOR * SPI_FLASH_SEC_SIZE, (uint32_t*)((void*)buffer), SPI_FLASH_SEC_SIZE) != SPI_FLASH_RESULT_OK)
    {
        DBGPRINTLN_CTX("  Could not read SSID list!");

        ets_free(buffer);

        return;
    }

    while (bss_link)
    {
        char ssid[33];

        ets_bzero(ssid, 33);
        ets_memcpy(ssid, bss_link->ssid, bss_link->ssid_len);

        DBGPRINTLN_CTX("  SSID [%s], RSSI [%d], BSSID ["MACSTR"], AUTH [%d], CHAN [%d]", ssid, bss_link->rssi, MAC2STR(bss_link->bssid), bss_link->authmode, bss_link->channel);

        if(!ssid_found)
        {
            char* read_ptr = buffer;

            while(read_ptr - buffer < SPI_FLASH_SEC_SIZE)
            {
                if(*read_ptr == 0xFF)
                    break;

                if(!ets_strcmp(read_ptr, ssid))
                {
                    ssid_found = read_ptr;
                    password_found = read_ptr + ets_strlen(read_ptr) + 1;

                    DBGPRINTLN_CTX("    AP is known!");

                    break;
                }

                read_ptr += ets_strlen(read_ptr) + 1; // Jump current SSID & current password
                read_ptr += ets_strlen(read_ptr) + 1;
            }
        }

        bss_link = bss_link->next.stqe_next;
    }

    if(!ssid_found)
    {
        DBGPRINTLN_CTX("    No known AP found!");

        DBGPRINTLN_CTX("    Connecting to eduroam...");

        ets_free(buffer);

        struct station_config config;

        ets_memset(&config, 0, sizeof(config));

        ets_strncpy(config.ssid, "eduroam", 32);

        wifi_station_disconnect();
        wifi_station_set_config_current(&config);

        wifi_station_clear_enterprise_ca_cert();
        wifi_station_clear_enterprise_cert_key();

        wifi_station_set_wpa2_enterprise_auth(1);
        wifi_station_set_enterprise_identity("2170802@my.ipleiria.pt", 22);
        wifi_station_set_enterprise_username("2170802@my.ipleiria.pt", 22);
        wifi_station_set_enterprise_password("15621568", 8);

        //wifi_station_set_enterprise_disable_time_check(1);

        wifi_station_connect();

        return;
    }

    DBGPRINTLN_CTX("    Found a known AP, connecting...");

    wifi_connect(ssid_found, password_found);

    ets_free(buffer);
}
void wifi_event_handler(System_Event_t *evt)
{
    DBGPRINTLN_CTX("Wifi event! [%d]", evt->event);

    switch (evt->event)
    {
        case EVENT_STAMODE_CONNECTED:
        {
            DBGPRINTLN_CTX("  Connected to [%s]", evt->event_info.connected.ssid);
        }
        break;
        case EVENT_STAMODE_DISCONNECTED:
        {
            DBGPRINTLN_CTX("  Disconnected from [%s] with reason [%d]", evt->event_info.disconnected.ssid, evt->event_info.disconnected.reason);

            switch(evt->event_info.disconnected.reason)
            {
                case REASON_AUTH_EXPIRE:
                case REASON_NO_AP_FOUND:
                {
                    DBGPRINTLN_CTX("  Lost AP, rescanning...");

                    wifi_set_opmode_current(NULL_MODE);
                    wifi_set_opmode_current(STATION_MODE);

                    wifi_station_scan(NULL, wifi_scan_callback);
                }
                break;
                case REASON_ASSOC_LEAVE:
                {
                    // Do nothing as we disconnected on purpose
                }
                break;
                default:
                {
                    DBGPRINTLN_CTX("  Reconnecting...");

                    wifi_station_connect();
                }
                break;
            }
        }
        break;
        case EVENT_STAMODE_AUTHMODE_CHANGE:
        {
            DBGPRINTLN_CTX("  Auth mode changed from [%d] to [%d]", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
        }
        break;
        case EVENT_STAMODE_GOT_IP:
        {
            DBGPRINTLN_CTX("  Got IP ["IPSTR", "IPSTR", "IPSTR"]", IP2STR(&evt->event_info.got_ip.ip), IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));

            sntp_start(200);
        }
        break;
        case EVENT_OPMODE_CHANGED:
        {
            DBGPRINTLN_CTX("  Op mode changed from [%d] to [%d]", evt->event_info.opmode_changed.old_opmode, evt->event_info.opmode_changed.new_opmode);

            if(evt->event_info.opmode_changed.new_opmode == STATION_MODE)
            {
                DBGPRINTLN_CTX("    Scanning for APs...");

                wifi_station_scan(NULL, wifi_scan_callback);
            }
        }
        break;
    }
}

void ssl_init()
{
    DBGPRINTLN_CTX("Set SSL CA certificate validation address [0x%X000] [%d] [%d]", SSL_CA_SECTOR, ESPCONN_CLIENT, espconn_secure_ca_enable(ESPCONN_CLIENT, SSL_CA_SECTOR));
    DBGPRINTLN_CTX("Set SSL buffer size [%d] [%d]", SSL_BUFFER_SIZE, espconn_secure_set_size(ESPCONN_CLIENT, SSL_BUFFER_SIZE));
}

uint8_t sdk_init(uint8_t adc_mode, uint8_t xtal_freq)
{
    DBGPRINTLN_CTX("Init data sector [0x%X000]", SDK_INIT_DATA_SECTOR);

    uint8_t* buffer = (uint8_t*)ets_zalloc(SPI_FLASH_SEC_SIZE);

	if (!buffer)
    {
        DBGPRINTLN_CTX("  Failed not allocate buffer!");

        return 0;
    }

    if(spi_flash_read(SDK_INIT_DATA_SECTOR * SPI_FLASH_SEC_SIZE, (uint32_t*)((void*)buffer), SPI_FLASH_SEC_SIZE) != SPI_FLASH_RESULT_OK)
    {
        DBGPRINTLN_CTX("  Could not read init data!");
        ets_free(buffer);

        return 0;
    }

    DBGPRINTLN_CTX("  ADC target mode [%X] current [%X]", adc_mode, buffer[ADC_MODE_LOCATION]);
    DBGPRINTLN_CTX("  XTAL target frequency [%X] current [%X]", xtal_freq, buffer[XTAL_FREQ_LOCATION]);

    if(buffer[ADC_MODE_LOCATION] == adc_mode && buffer[XTAL_FREQ_LOCATION] == xtal_freq)
    {
        ets_free(buffer);

        return 0;
    }

    buffer[ADC_MODE_LOCATION] = adc_mode;
    buffer[XTAL_FREQ_LOCATION] = xtal_freq;

    if(spi_flash_erase_sector(SDK_INIT_DATA_SECTOR) != SPI_FLASH_RESULT_OK)
    {
        DBGPRINTLN_CTX("  Could not erase init data sector!");
        ets_free(buffer);

        return 0;
    }

    if(spi_flash_write(SDK_INIT_DATA_SECTOR * SPI_FLASH_SEC_SIZE, (uint32_t*)((void*)buffer), SPI_FLASH_SEC_SIZE) != SPI_FLASH_RESULT_OK)
    {
        DBGPRINTLN_CTX("  Could not write init data!");
        ets_free(buffer);

        return 0;
    }

    return 1;
}

void sntp_start(uint32_t timeout)
{
    DBGPRINTLN_CTX("Waiting for SNTP time...");

    sntp_stop();
    sntp_set_timezone(0);
    sntp_setservername(0, "time.google.com");
    sntp_init();

    ets_timer_disarm(&sntp_timer);
    ets_timer_setfn(&sntp_timer, sntp_timer_callback, NULL);
    ets_timer_arm_new(&sntp_timer, timeout, 1, 1);
}

void ota_event_handler(uint32_t status, void *arg)
{
    uint8_t is_error = !!(status & OTA_STATUS_ERROR_FLAG);
    uint8_t is_tcp_error = !!(status & OTA_STATUS_ERROR_TCP_FLAG);
    uint8_t is_mem_error = !!(status & OTA_STATUS_ERROR_MALLOC_FLAG);
    uint8_t is_timeout_error = !!(status & OTA_STATUS_ERROR_TIMEOUT_FLAG);
    uint8_t is_http_error = !!(status & OTA_STATUS_ERROR_HTTP_FLAG);

    uint8_t has_error = is_error || is_tcp_error || is_mem_error || is_timeout_error || is_http_error;

    status &= ~(OTA_STATUS_ERROR_FLAG | OTA_STATUS_ERROR_TCP_FLAG | OTA_STATUS_ERROR_MALLOC_FLAG | OTA_STATUS_ERROR_TIMEOUT_FLAG | OTA_STATUS_ERROR_HTTP_FLAG);

    DBGPRINTLN_CTX("OTA event! [%d %d %d %d %d] [%d]", is_error, is_tcp_error, is_mem_error, is_timeout_error, is_http_error, status);

    if(is_mem_error)
    {
        DBGPRINTLN_CTX("  Failed to allocate memory! [%d]", status);

        return;
    }

    if(is_tcp_error)
    {
        DBGPRINT_CTX("  TCP error");

        if(arg)
            DBGPRINT(" [%d]", *(int8_t*)arg);

        DBGPRINT(" at stage");
    }
    else if(is_http_error)
    {
        DBGPRINT_CTX("  HTTP error");

        if(status >= 100)
            DBGPRINT(" [%d]", status);

        if(arg)
            DBGPRINT(" [%s]", (char*)arg);

        if(status < 100)
            DBGPRINT(" at stage");
        else
            DBGPRINT("\n");
    }
    else if(is_timeout_error)
        DBGPRINT_CTX("  Timed out at stage");
    else if(is_error)
        DBGPRINT_CTX("  Error at stage");

    switch(status)
    {
        case OTA_STATUS_RESOLVE:
        {
            if(has_error)
            {
                DBGPRINTLN(" RESOLVE!");

                update_scheduled = 0;
                update_new_version = 0;

                sntp_start(5000);

                return;
            }

            DBGPRINTLN_CTX("  Successfully resolved hostname to "IPSTR"!", IP2STR((ip_addr_t*)arg));
        }
        break;
        case OTA_STATUS_CONNECT:
        {
            if(has_error)
            {
                DBGPRINTLN(" CONNECT!");

                update_scheduled = 0;
                update_new_version = 0;

                sntp_start(5000);

                return;
            }

            // arg is uint8_t* request data
            DBGPRINTLN_CTX("  Successfully connected!");
        }
        break;
        case OTA_STATUS_REQUEST:
        {
            if(has_error)
            {
                DBGPRINTLN(" REQUEST!");

                update_scheduled = 0;
                update_new_version = 0;

                sntp_start(5000);

                return;
            }

            DBGPRINTLN_CTX("  Got HTTP response! Firmware size [%d]", *(uint32_t*)arg);
        }
        break;
        case OTA_STATUS_WRITE_FLASH:
        {
            if(has_error)
            {
                DBGPRINTLN(" WRITE_FLASH!");

                update_scheduled = 0;
                update_new_version = 0;

                sntp_start(5000);

                return;
            }

            DBGPRINTLN_CTX("  Writting flash! Total written [%d]", *(uint32_t*)arg);
        }
        break;
        case OTA_STATUS_FINISH:
        {
            if(has_error)
            {
                DBGPRINTLN(" FINISH!");

                update_scheduled = 0;
                update_new_version = 0;

                sntp_start(5000);

                return;
            }

            DBGPRINTLN_CTX("  Successfully updated the firmware! New app ID [%d]", *(uint8_t*)arg);

            update_scheduled = 0;
            update_new_version = 0;

            boot_config_t cfg;

            DBGPRINTLN_CTX("  Getting boot config...");

            if(!ota_boot_get_config(&cfg))
            {
                DBGPRINTLN_CTX("    Failed to get boot config!");

                sntp_start(5000);

                return;
            }

            DBGPRINTLN_CTX("    Current app ID [%d]", cfg.current_rom);

            cfg.current_rom = *(uint8_t*)arg;

            DBGPRINTLN_CTX("    Setting boot config...");

            if(!ota_boot_set_config(&cfg))
            {
                DBGPRINTLN_CTX("      Failed to set boot config!");

                sntp_start(5000);

                return;
            }

            DBGPRINTLN_CTX("      Rebooting...");

            system_restart();
        }
        break;
    }
}

uint32_t get_flash_sector_count()
{
    static uint32_t count = 0;

    if(!count)
    {
        switch (system_get_flash_size_map())
        {
            case FLASH_SIZE_4M_MAP_256_256:
                count = 128;
                break;
            case FLASH_SIZE_8M_MAP_512_512:
                count =  256;
                break;
            case FLASH_SIZE_16M_MAP_512_512:
            case FLASH_SIZE_16M_MAP_1024_1024:
                count =  512;
                break;
            case FLASH_SIZE_32M_MAP_512_512:
            case FLASH_SIZE_32M_MAP_1024_1024:
                count =  1024;
                break;
            case FLASH_SIZE_64M_MAP_1024_1024:
                count =  2048;
                break;
            case FLASH_SIZE_128M_MAP_1024_1024:
                count =  4096;
                break;
        }
    }

    return count;
}
uint64_t get_system_tick()
{
    static uint32_t high = 0;
    static uint32_t low = 0;
    uint32_t a_low = REG(WDEV_SYS_TIME);

    if(a_low < low)
        high++;

    low = a_low;

    return ((uint64_t)high << 32) | low;
}

void user_rf_pre_init()
{
    system_set_os_print(0);
    //system_phy_set_powerup_option(3);
}
uint32_t user_rf_cal_sector_set()
{
    return RF_CAL_DATA_SECTOR;
}
void user_init()
{
    REG(DPORT_CPU_CLOCK) = DPORT_CPU_CLOCK_X2; // Set clock to 160 MHz
    ets_update_cpu_frequency(160); // Re-calibrate timers

    /*
    REG(RTC_GPIO_CFG(3)) &= ~(RTC_GPIO_CFG3_PIN_FUNC_M << RTC_GPIO_CFG3_PIN_FUNC_S);
    REG(RTC_GPIO_CFG(3)) |= RTC_GPIO_CFG3_PIN_FUNC_RTC_GPIO0; // Configure iomux as RTCGPIO0 (GPIO16)
    REG(RTC_GPIO_CONF) &= ~RTC_GPIO_CONF_OUT_ENABLE; // Disable control by RTC, controlled by user now
    REG(RTC_GPIO_ENABLE) |= BIT(0); // Set RTCGPIO0 as output
    REG(RTC_GPIO_OUT) |= BIT(0); // Set RTCGPIO0 high

    REG(WIFI_LED_IOMUX) &= ~IOMUX_PIN_FUNC_MASK;
    REG(WIFI_LED_IOMUX) |= WIFI_LED_FUNC;
    */

    REG(IOMUX_GPIO1) &= ~IOMUX_PIN_FUNC_MASK;
    REG(IOMUX_GPIO1) |= IOMUX_GPIO1_FUNC_UART0_TXD;
    REG(IOMUX_GPIO3) &= ~IOMUX_PIN_FUNC_MASK;
    REG(IOMUX_GPIO3) |= IOMUX_GPIO3_FUNC_UART0_RXD;

    REG(IOMUX_GPIO2) &= ~IOMUX_PIN_FUNC_MASK;
    REG(IOMUX_GPIO2) |= IOMUX_GPIO2_FUNC_UART1_TXD;

    /*
    REG(GPIO_ENABLE_OUT_SET) = BIT(WIFI_LED_BIT) | BIT(RGB_DATA_BIT); // Both as outputs
    REG(GPIO_OUT_SET) = BIT(WIFI_LED_BIT); // Wifi LED active LOW, turn off
    REG(GPIO_OUT_CLEAR) = BIT(RGB_DATA_BIT); // RGB data LOW
    */

    uart1_init(115200);

    ets_install_putc1((ets_putc_fn_t)uart1_write_byte);
    ets_install_putc2(NULL);

    DBGPRINTLN();

    DBGPRINTLN_CTX("SDK version: [%s]", system_get_sdk_version());
    DBGPRINTLN_CTX("Build version: [v%lu]", BUILD_VERSION);
    DBGPRINTLN_CTX("Reset reason: [%lu]", *(uint32_t*)system_get_rst_info());
    DBGPRINTLN_CTX("System clock: [%luM] [%lu]", ets_get_cpu_frequency(), REG(DPORT_CPU_CLOCK) & DPORT_CPU_CLOCK_X2);
    DBGPRINTLN_CTX("Chip ID: [%X]", system_get_chip_id());
    DBGPRINTLN_CTX("Flash ID: [%X]", spi_flash_get_id());
    DBGPRINTLN_CTX("Flash sector count: [%lu]", get_flash_sector_count());
    DBGPRINTLN_CTX("Free heap: [%lu]", system_get_free_heap_size());

    if(sdk_init(ADC_MODE_VDD, XTAL_FREQ_24M))
    {
        DBGPRINTLN_CTX("  Restarting system to apply SDK init changes...");

        system_restart();

        return;
    }

    wifi_init();
    ssl_init();

    ets_timer_disarm(&tick_timer);
    ets_timer_setfn(&tick_timer, tick_timer_callback, NULL);
    ets_timer_arm_new(&tick_timer, 100, 1, 1);

    system_init_done_cb(user_main);
}
void user_main()
{
    uint16_t sys_vdd = system_get_vdd33();

    DBGPRINTLN_CTX("System VDD: [%d]", (uint32_t)(sys_vdd * 1000.f / 1024.f));
}
