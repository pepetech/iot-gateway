#ifndef __BOOT_H__
#define __BOOT_H__

#include <c_types.h>

#define MAX_ROMS			4
#define BOOT_CONFIG_SECTOR	1
#define BOOT_RTC_ADDR		64
#define BOOT_GPIO_NUM		16
#define BOOT_BAUD			115200
//#define BOOT_NO_ASM

#define BOOT_CONFIG_MAGIC 	0xE1

#define MODE_STANDARD		0x00
#define MODE_GPIO_ROM		0x01
#define MODE_TEMP_ROM		0x02
#define MODE_ERASE_SDK_DATA	0x04

#define BOOT_RTC_MAGIC 0x2334AE68
#define BOOT_RTC_READ 1
#define BOOT_RTC_WRITE 0

#define ROM_MAGIC_OLD 0xE9
#define ROM_MAGIC_NEW 0xEA

// buffer size, must be at least 0x10 (size of rom_header_new structure)
#define BUFFER_SIZE 0x100
#define SECTOR_SIZE 0x1000
#define READ_SIZE 0x1000 // Max 1 sector

#define CHKSUM_INIT 0xEF

#ifndef UART_CLK_FREQ
	#define UART_CLK_FREQ (26000000 * 2) // reset apb freq = 2x crystal freq: http://esp8266-re.foogod.com/wiki/Serial_UART
#endif

typedef struct
{
	uint8_t magic;           ///< Our magic, identifies boot configuration - should be BOOT_CONFIG_MAGIC
	uint8_t mode;            ///< Boot loader mode (MODE_STANDARD | MODE_GPIO_ROM | MODE_GPIO_SKIP)
	uint8_t current_rom;     ///< Currently selected ROM (will be used for next standard boot)
	uint8_t gpio_rom;        ///< ROM to use for GPIO boot (hardware switch) with mode set to MODE_GPIO_ROM
	uint32_t roms[MAX_ROMS]; ///< Flash addresses of each ROM
	uint8_t rom_count;       ///< Quantity of ROMs available to boot
	uint8_t unused[2];       ///< Padding (not used)
	uint8_t chksum;          ///< Checksum of this configuration structure
} boot_config_t;

typedef struct
{
	uint32_t magic;           ///< Magic, identifies rBoot RTC data - should be RBOOT_RTC_MAGIC
	uint8_t next_mode;        ///< The next boot mode, defaults to MODE_STANDARD - can be set to MODE_TEMP_ROM
	uint8_t last_mode;        ///< The last (this) boot mode - can be MODE_STANDARD, MODE_GPIO_ROM or MODE_TEMP_ROM
	uint8_t last_rom;         ///< The last (this) boot rom number
	uint8_t temp_rom;         ///< The next boot rom number when next_mode set to MODE_TEMP_ROM
	uint8_t chksum;           ///< Checksum of this structure this will be updated for you passed to the API
} boot_rtc_config_t;

// functions we'll call by address
typedef void (* loader_fn_t)(uint32_t);
typedef void (* rom_entry_fn_t)();

typedef struct
{
	uint8_t* address;
	uint32_t length;
} section_header_t;

typedef struct
{
	uint8_t magic;
	uint8_t sect_count;
	uint8_t flash_mode;
	uint8_t flash_size_freq;
	rom_entry_fn_t entry;
} ram_header_t;

typedef struct
{
	uint8_t magic;
	uint8_t sect_count;
	uint8_t flash_mode;
	uint8_t flash_size_freq;
	rom_entry_fn_t entry;
	section_header_t irom_header;
} rom_header_t;

#endif
