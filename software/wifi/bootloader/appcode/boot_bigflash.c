#include <c_types.h>
#include "ets_func.h"
#include "boot.h"

#define FLASH_CACHE_USE_32K 1 // 1 = 32K, 0 = 16K

extern uint32_t SPIRead(uint32_t, void*, uint32_t);
extern void Cache_Read_Enable(uint8_t, uint8_t, uint8_t);

void Cache_Read_Enable_New()
{
	static uint8_t mmap_1 = 0xFF;
	static uint8_t mmap_2 = 0xFF;

	if (mmap_1 == 0xFF)
	{
		uint32_t val;
		boot_config_t conf;

		SPIRead(BOOT_CONFIG_SECTOR * 0x1000, &conf, sizeof(boot_config_t));

		boot_rtc_config_t rtc_conf;
		uint8_t off = (uint8_t*)&rtc_conf.last_rom - (uint8_t*)&rtc_conf;

		val = *(volatile uint32_t*)(0x60001100 + (BOOT_RTC_ADDR * 4) + (off & ~(uint32_t)3));
		val = conf.roms[((uint8_t*)&val)[off & 3]];

		val /= 0x100000; // Divide by 1 MB to know which one to map

		mmap_2 = val >> 1;
		mmap_1 = val % 2;

		//ets_printf("mmap %d,%d,1\r\n", mmap_1, mmap_2);
	}

	Cache_Read_Enable(mmap_1, mmap_2, FLASH_CACHE_USE_32K);
}
