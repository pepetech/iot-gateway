#include "ets_func.h"
#include "registers.h"
#include "boot.h"
#include "gpio.h"
#include "loader.h"

static uint32_t check_image(uint32_t readpos)
{
	uint8_t buffer[BUFFER_SIZE];
	uint8_t section_count;
	uint8_t chksum = CHKSUM_INIT;
	uint32_t remaining;
	uint32_t ram_header_addr;

	rom_header_t *header = (rom_header_t*)buffer;
	section_header_t *section = (section_header_t*)buffer;

	if (readpos == 0 || readpos == 0xFFFFFFFF)
		return 0;

	ets_printf("  Reading ROM header at [0x%08X]...\r\n", readpos);

	if (SPIRead(readpos, header, sizeof(rom_header_t)))
		return 0;

	ets_printf("    ROM header magic [%02X] count [%d]!\r\n", header->magic, header->sect_count);

	if (header->magic == ROM_MAGIC_OLD)
	{
		ram_header_addr = readpos;
		section_count = header->sect_count;
	}
	else if (header->magic == ROM_MAGIC_NEW && header->sect_count == 0x04)
	{
		ram_header_addr = readpos + sizeof(rom_header_t) + header->irom_header.length;
		section_count = 0xFF;
	}
	else
	{
		ets_printf("      Invalid ROM header magic!\r\n");

		return 0;
	}

	readpos += sizeof(ram_header_t);

	// test each section
	for (uint8_t current_section = 0; current_section < section_count; current_section++)
	{
		ets_printf("      Reading section [%d] header at [0x%08X]...\r\n", current_section, readpos);

		if (SPIRead(readpos, section, sizeof(section_header_t)))
			return 0;

		ets_printf("        Section [%d] start address [0x%08X] length [%d]!\r\n", current_section, section->address, section->length);

		readpos += sizeof(section_header_t);

		remaining = section->length;

		while (remaining)
		{
			uint32_t readlen = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;

			if (SPIRead(readpos, buffer, readlen))
			{
				ets_printf("          Error reading flash data at [0x%08X]!\r\n", readpos);

				return 0;
			}

			readpos += readlen;
			remaining -= readlen;

			for (uint32_t i = 0; i < readlen; i++)
				chksum ^= buffer[i];
		}

		if (section_count == 0xFF)
		{
			ets_printf("  Reading RAM header at [0x%08X]...\r\n", readpos);

			if (SPIRead(readpos, header, sizeof(ram_header_t)))
				return 0;

			ets_printf("    RAM header magic [%02X] count [%d]!\r\n", header->magic, header->sect_count);

			if (header->magic != ROM_MAGIC_OLD)
			{
				ets_printf("      Invalid RAM header magic!\r\n");

				return 0;
			}

			section_count = header->sect_count + 1;
			readpos += sizeof(ram_header_t);
		}
	}

	// round up to next 16 and get checksum
	readpos = readpos | 0x0F;

	ets_printf("          Reading checksum at [0x%08X]!\r\n", readpos);

	if (SPIRead(readpos, buffer, 1))
		return 0;

	// compare calculated and stored checksums
	ets_printf("            Checksum calc [%02X] stored [%02X]!\r\n", chksum, buffer[0]);

	if (buffer[0] != chksum)
	{
		ets_printf("              Checksum does not match!\r\n");

		return 0;
	}

	return ram_header_addr;
}

static uint32_t perform_gpio_boot(boot_config_t *romconf)
{
	if (!(romconf->mode & MODE_GPIO_ROM))
		return 0;

	return get_gpio(BOOT_GPIO_NUM) == 0;
}

static uint32_t system_rtc_mem(uint32_t addr, void *buff, uint32_t length, uint32_t mode)
{
    // validate reading a user block
    if (addr < 64)
		return 0;

    if (!buff || ((uint32_t)buff & 0x3))
		return 0;

    if (length & 0x3)
		return 0;

    // check valid length from specified starting point
    if (length > (0x300 - (addr * 4)))
		return 0;

    // copy the data
    for (uint32_t blocks = (length >> 2); blocks > 0; blocks--)
	{
        volatile uint32_t *ram = ((uint32_t*)buff) + blocks - 1;
        volatile uint32_t *rtc = PREG(REG_RTCS_BASE(0)) + addr + blocks - 1;

		if (mode == BOOT_RTC_WRITE)
			*rtc = *ram;
		else
			*ram = *rtc;
    }

    return 1;
}

static enum rst_reason get_reset_reason()
{
	return REG(REG_RTCS_BASE(0));
}

static void copy_loader()
{
	ets_memcpy((void*)_text_addr, _text_data, _text_len);
}

static uint8_t calc_chksum(uint8_t *start, uint8_t *end)
{
	uint8_t chksum = CHKSUM_INIT;

	while(start < end)
	{
		chksum ^= *start;
		start++;
	}

	return chksum;
}

uint32_t __attribute__ ((noinline)) find_image()
{
	uint32_t flash_size;
	uint8_t buffer[SECTOR_SIZE];
	uint8_t boot_rom;
	uint8_t gpio_boot = 0;
	uint8_t temp_boot = 0;

	boot_config_t *rom_conf = (boot_config_t*)buffer;
	ram_header_t *header = (ram_header_t*)buffer;

	if (get_reset_reason() != REASON_SOFT_RESTART)
		REG(UART0_CLKDIV) = (UART_CLK_FREQ / BOOT_BAUD); // use register macros pls no autism

	ets_delay_us(1000);

	ets_printf("\r\nBoot v2.3.0\r\n");

	// read rom header
	if(SPIRead(0, header, sizeof(ram_header_t)))
	{
		ets_printf("Error reading flash settings header!\r\n");

		return 0;
	}

	ets_printf("Flash Size: ");
	switch(header->flash_size_freq >> 4)
	{
		case 1:
		{
			ets_printf("2 Mbit\r\n");

			flash_size = 2 * 128 * 1024;
		}
		break;
		case 0:
		{
			ets_printf("4 Mbit\r\n");

			flash_size = 4 * 128 * 1024;
		}
		break;
		case 2:
		{
			ets_printf("8 Mbit\r\n");

			flash_size = 8 * 128 * 1024;
		}
		break;
		case 3:
		{
			ets_printf("16 Mbit\r\n");
			//ets_printf("16 Mbit (512 KB)\r\n");

			flash_size = 16 * 128 * 1024;
		}
		break;
		case 4:
		{
			ets_printf("32 Mbit\r\n");
			//ets_printf("32 Mbit (512 KB)\r\n");

			flash_size = 32 * 128 * 1024;
		}
		break;
		case 5:
		{
			ets_printf("16 Mbit\r\n");

			flash_size = 16 * 128 * 1024;
		}
		break;
		case 6:
		{
			ets_printf("32 Mbit\r\n");

			flash_size = 32 * 128 * 1024;
		}
		break;
		case 7:
		{
			ets_printf("64 Mbit\r\n");

			flash_size = 64 * 128 * 1024;
		}
		break;
		case 8:
		{
			ets_printf("128 Mbit\r\n");

			flash_size = 128 * 128 * 1024;
		}
		break;
		default:
		{
			ets_printf("Unknown\r\n");
		}
		break;
	}

	ets_printf("Flash Mode: ");
	switch(header->flash_mode)
	{
		case 0:
			ets_printf("QIO\r\n");
			break;
		case 1:
			ets_printf("QOUT\r\n");
			break;
		case 2:
			ets_printf("DIO\r\n");
			break;
		case 3:
			ets_printf("DOUT\r\n");
			break;
		default:
			ets_printf("Unknown\r\n");
			break;
	}

	ets_printf("Flash Speed: ");
	switch(header->flash_size_freq & 0x0F)
	{
		case 0:
			ets_printf("40 MHz\r\n");
			break;
		case 1:
			ets_printf("26.7 MHz\r\n");
			break;
		case 2:
			ets_printf("20 MHz\r\n");
			break;
		case 15:
			ets_printf("80 MHz\r\n");
			break;
		default:
			ets_printf("Unknown\r\n");
			break;
	}

	ets_printf("\r\n");

	// read boot config
	if(SPIRead(BOOT_CONFIG_SECTOR * SECTOR_SIZE, rom_conf, SECTOR_SIZE))
	{
		ets_printf("Error reading boot config!\r\n\r\n");

		return 0;
	}

	if (rom_conf->magic != BOOT_CONFIG_MAGIC || rom_conf->chksum != calc_chksum((uint8_t*)rom_conf, (uint8_t*)&rom_conf->chksum))
	{
		ets_printf("Invalid boot config! Creating new...\r\n");

		ets_memset(rom_conf, 0, sizeof(boot_config_t));

		rom_conf->magic = BOOT_CONFIG_MAGIC;
		rom_conf->current_rom = 0;
		rom_conf->gpio_rom = 0;
		rom_conf->roms[0] = 0x00002000;
		rom_conf->roms[1] = 0x00082000;
		rom_conf->rom_count = 2;
		rom_conf->chksum = calc_chksum((uint8_t*)rom_conf, (uint8_t*)&rom_conf->chksum);

		SPIEraseSector(BOOT_CONFIG_SECTOR);
		SPIWrite(BOOT_CONFIG_SECTOR * SECTOR_SIZE, rom_conf, SECTOR_SIZE);
	}

	// try rom selected in the config, unless overriden by gpio/temp boot
	boot_rom = rom_conf->current_rom;

	// if rtc data enabled, check for valid data
	boot_rtc_config_t rtc_conf;

	if (system_rtc_mem(BOOT_RTC_ADDR, &rtc_conf, sizeof(boot_rtc_config_t), BOOT_RTC_READ) && (rtc_conf.chksum == calc_chksum((uint8_t*)&rtc_conf, (uint8_t*)&rtc_conf.chksum)))
	{
		if (rtc_conf.next_mode & MODE_TEMP_ROM)
		{
			if (rtc_conf.temp_rom >= rom_conf->rom_count)
			{
				ets_printf("Invalid RTC temp ROM!\r\n\r\n");

				return 0;
			}

			ets_printf("Selecting RTC temp ROM [%d]...\r\n", rtc_conf.temp_rom);

			temp_boot = 1;
			boot_rom = rtc_conf.temp_rom;
		}
	}

	if (perform_gpio_boot(rom_conf))
	{
		if (rom_conf->gpio_rom >= rom_conf->rom_count)
		{
			ets_printf("Invalid GPIO ROM!\r\n");

			return 0;
		}

		ets_printf("Selecting GPIO ROM [%d]...\r\n", rom_conf->gpio_rom);

		boot_rom = rom_conf->gpio_rom;
		gpio_boot = 1;
	}

	if (rom_conf->mode & MODE_ERASE_SDK_DATA)
	{
		ets_printf("Erasing SDK data sectors...\r\n");

		SPIEraseSector((flash_size / SECTOR_SIZE) - 3); // Erase last 3 sectors
		SPIEraseSector((flash_size / SECTOR_SIZE) - 2);
		SPIEraseSector((flash_size / SECTOR_SIZE) - 1);
	}

	if (rom_conf->current_rom >= rom_conf->rom_count && !gpio_boot && !temp_boot)
	{
		ets_printf("Invalid current ROM! Defaulting to 0.\r\n");

		boot_rom = 0;

		rom_conf->current_rom = 0;
		rom_conf->chksum = calc_chksum((uint8_t*)rom_conf, (uint8_t*)&rom_conf->chksum);

		SPIEraseSector(BOOT_CONFIG_SECTOR);
		SPIWrite(BOOT_CONFIG_SECTOR * SECTOR_SIZE, rom_conf, SECTOR_SIZE);
	}

	ets_printf("Checking selected ROM [%d]...\r\n", boot_rom);

	uint32_t ram_header_addr = check_image(rom_conf->roms[boot_rom]);

	if (gpio_boot && !ram_header_addr)
	{
		ets_printf("GPIO ROM [%d] NOT OK!\r\n\r\n", boot_rom);

		return 0;
	}

	if (temp_boot && !ram_header_addr)
	{
		ets_printf("RTC temp ROM [%d] NOT OK!\r\n\r\n", boot_rom);

		rtc_conf.next_mode = MODE_STANDARD;
		rtc_conf.chksum = calc_chksum((uint8_t*)&rtc_conf, (uint8_t*)&rtc_conf.chksum);

		system_rtc_mem(BOOT_RTC_ADDR, &rtc_conf, sizeof(boot_rtc_config_t), BOOT_RTC_WRITE);

		return 0;
	}

	while (!ram_header_addr)
	{
		ets_printf("Current ROM [%d] NOT OK!\r\n", boot_rom);

		if (boot_rom-- == 0)
			boot_rom = rom_conf->rom_count - 1;

		if (boot_rom == rom_conf->current_rom)
		{
			ets_printf("No good ROM available!\r\n\r\n");

			return 0;
		}

		ets_printf("Checking ROM [%d]...\r\n", boot_rom);

		ram_header_addr = check_image(rom_conf->roms[boot_rom]);
	}

	ets_printf("ROM [%d] OK!\r\n", boot_rom);

	if (!gpio_boot && !temp_boot && boot_rom != rom_conf->current_rom)
	{
		rom_conf->current_rom = boot_rom;

		rom_conf->chksum = calc_chksum((uint8_t*)rom_conf, (uint8_t*)&rom_conf->chksum);

		SPIEraseSector(BOOT_CONFIG_SECTOR);
		SPIWrite(BOOT_CONFIG_SECTOR * SECTOR_SIZE, buffer, SECTOR_SIZE);
	}

	ets_printf("Updating RTC config...\r\n", boot_rom);

	rtc_conf.magic = BOOT_RTC_MAGIC;
	rtc_conf.next_mode = MODE_STANDARD;
	rtc_conf.last_mode = MODE_STANDARD;
	if (temp_boot)
		rtc_conf.last_mode |= MODE_TEMP_ROM;
	if (gpio_boot)
		rtc_conf.last_mode |= MODE_GPIO_ROM;
	rtc_conf.last_rom = boot_rom;
	rtc_conf.temp_rom = 0;
	rtc_conf.chksum = calc_chksum((uint8_t*)&rtc_conf, (uint8_t*)&rtc_conf.chksum);

	system_rtc_mem(BOOT_RTC_ADDR, &rtc_conf, sizeof(boot_rtc_config_t), BOOT_RTC_WRITE);

	ets_printf("Running loader on ROM [%d]...\r\n\r\n", boot_rom);

	copy_loader();

	return ram_header_addr;
}

void call_user_start()
{
#ifdef BOOT_NO_ASM
	uint32_t ram_header_addr = find_image();

	if (ram_header_addr)
		((loader_fn_t)entry_addr)(ram_header_addr); // Call the loader
#else
	__asm volatile (
		"mov a15, a0\n"          // store return addr, hope nobody wanted a15!
		"call0 find_image\n"     // find a good rom to boot
		"mov a0, a15\n"          // restore return addr
		"bnez a2, 1f\n"          // ? success
		"ret\n"                  // no, return
		"1:\n"                   // yes...
		"movi a3, entry_addr\n"  // get pointer to entry_addr
		"l32i a3, a3, 0\n"       // get value of entry_addr
		"jx a3\n"                // now jump to it
	);
#endif
}
