#ifndef __RADIO_PROTOCOL_H__
#define __RADIO_PROTOCOL_H__

#include <stdio.h>

#define RADIO_CMD_REQUEST_FLAG             0x80

#define RADIO_CMD_PING			           0x00
#define RADIO_CMD_LED_SET                  0x01
#define RADIO_CMD_UID_READ	               0x02
#define RADIO_CMD_SYS_RESET    	           0x0F

#define RADIO_CMD_TEMP_DATA		           0x10
#define RADIO_CMD_BAT_DATA		           0x11

#define RADIO_CMD_RADIO_FREQ_CFG           0x40
#define RADIO_CMD_RADIO_POWER_CFG	       0x41
#define RADIO_CMD_REPORT_CFG	           0x42
#define RADIO_CMD_SLEEP_CFG	               0x43

#define RADIO_CMD_FOTA_QUERY_INFO          0x50
#define RADIO_CMD_FOTA_SEND_CHUNK          0x51

typedef struct radio_cmd_ping_req_t radio_cmd_ping_req_t;
typedef struct radio_cmd_ping_res_t radio_cmd_ping_res_t;
typedef struct radio_cmd_led_set_req_t radio_cmd_led_set_req_t;
typedef struct radio_cmd_led_set_res_t radio_cmd_led_set_res_t;
typedef struct radio_cmd_uid_read_req_t radio_cmd_uid_read_req_t;
typedef struct radio_cmd_uid_read_res_t radio_cmd_uid_read_res_t;
typedef struct radio_cmd_sys_reset_req_t radio_cmd_sys_reset_req_t;
typedef struct radio_cmd_sys_reset_res_t radio_cmd_sys_reset_res_t;
typedef struct radio_cmd_temp_data_req_t radio_cmd_temp_data_req_t;
typedef struct radio_cmd_temp_data_res_t radio_cmd_temp_data_res_t;
typedef struct radio_cmd_bat_data_req_t radio_cmd_bat_data_req_t;
typedef struct radio_cmd_bat_data_res_t radio_cmd_bat_data_res_t;
typedef struct radio_cmd_adxl345_data_req_t radio_cmd_adxl345_data_req_t;
typedef struct radio_cmd_adxl345_data_res_t radio_cmd_adxl345_data_res_t;
typedef struct radio_cmd_radio_freq_cfg_req_t radio_cmd_radio_freq_cfg_req_t;
typedef struct radio_cmd_radio_freq_cfg_res_t radio_cmd_radio_freq_cfg_res_t;
typedef struct radio_cmd_radio_power_cfg_req_t radio_cmd_radio_power_cfg_req_t;
typedef struct radio_cmd_radio_power_cfg_res_t radio_cmd_radio_power_cfg_res_t;
typedef struct radio_cmd_report_cfg_req_t radio_cmd_report_cfg_req_t;
typedef struct radio_cmd_report_cfg_res_t radio_cmd_report_cfg_res_t;
typedef struct radio_cmd_sleep_cfg_req_t radio_cmd_sleep_cfg_req_t;
typedef struct radio_cmd_sleep_cfg_res_t radio_cmd_sleep_cfg_res_t;
typedef struct radio_cmd_fota_query_info_req_t radio_cmd_fota_query_info_req_t;
typedef struct radio_cmd_fota_query_info_res_t radio_cmd_fota_query_info_res_t;
typedef struct radio_cmd_fota_send_chunk_req_t radio_cmd_fota_send_chunk_req_t;
typedef struct radio_cmd_fota_send_chunk_res_t radio_cmd_fota_send_chunk_res_t;

struct radio_cmd_ping_req_t
{
	uint8_t ubCommand;
};
struct radio_cmd_ping_res_t
{
	uint8_t ubCommand;
};
struct radio_cmd_led_set_req_t
{
	uint8_t ubCommand;
	uint8_t ubMode;
	uint16_t usValue;
};
struct radio_cmd_led_set_res_t
{
	uint8_t ubCommand;
};
struct radio_cmd_uid_read_req_t
{
	uint8_t ubCommand;
};
struct radio_cmd_uid_read_res_t
{
	uint8_t ubCommand;
    uint16_t usID0_15;
    uint16_t usID16_31;
    uint32_t ulID32_63;
    uint32_t ulID64_96;
};
struct radio_cmd_sys_reset_req_t
{
	uint8_t ubCommand;
};
struct radio_cmd_sys_reset_res_t
{
	uint8_t ubCommand;
};
struct radio_cmd_temp_data_req_t
{
	uint8_t ubCommand;
	uint32_t ulSamples;
};
struct radio_cmd_temp_data_res_t
{
	uint8_t ubCommand;
	double gWaterTemp;
};
struct radio_cmd_bat_data_req_t
{
	uint8_t ubCommand;
	uint32_t ulSamples;
};
struct radio_cmd_bat_data_res_t
{
	uint8_t ubCommand;
	double gBatteryVoltage;
	uint8_t ubChargerDetected;
	uint8_t ubChargerTerminated;
	uint8_t ubBatteryLow;
};
struct radio_cmd_radio_freq_cfg_req_t
{
	uint8_t ubCommand;
	uint8_t ubMode;
};
struct radio_cmd_radio_freq_cfg_res_t
{
	uint8_t ubCommand;
};
struct radio_cmd_radio_power_cfg_req_t
{
	uint8_t ubCommand;
	int8_t bATCTargetRSSI;
};
struct radio_cmd_radio_power_cfg_res_t
{
	uint8_t ubCommand;
};
struct radio_cmd_report_cfg_req_t
{
	uint8_t ubCommand;
	uint32_t ulDataReportDelay;
	uint32_t ulAccelSamples;
	uint32_t ulTempSamples;
	uint32_t ulBatterySamples;
};
struct radio_cmd_report_cfg_res_t
{
	uint8_t ubCommand;
};
struct radio_cmd_sleep_cfg_req_t
{
	uint8_t ubCommand;
	uint32_t ulSleepTimeout;
};
struct radio_cmd_sleep_cfg_res_t
{
	uint8_t ubCommand;
};
struct radio_cmd_fota_query_info_req_t
{
	uint8_t ubCommand;
};
struct radio_cmd_fota_query_info_res_t
{
	uint8_t ubCommand;
	uint32_t ulCurrentVersion;
	char pszBuildDate[12];
	char pszBuildTime[9];
};
struct radio_cmd_fota_send_chunk_req_t
{
	uint8_t ubCommand;
};
struct radio_cmd_fota_send_chunk_res_t
{
	uint8_t ubCommand;
};

#endif // __RADIO_PROTOCOL_H__
