#include "ets_func.h"
#include "boot.h"

rom_entry_fn_t __attribute__ ((noinline)) load_rom(uint32_t readpos)
{
	uint8_t *writepos;
	uint32_t remaining;
	rom_entry_fn_t entry;

	ram_header_t header;
	section_header_t section;

	SPIRead(readpos, &header, sizeof(ram_header_t));
	readpos += sizeof(ram_header_t);

	entry = header.entry;

	// copy all the sections
	for (uint8_t section_count = header.sect_count; section_count > 0; section_count--)
	{
		// read section header
		SPIRead(readpos, &section, sizeof(section_header_t));
		readpos += sizeof(section_header_t);

		// get section address and length
		writepos = section.address;
		remaining = section.length;

		while (remaining)
		{
			uint32_t readlen = (remaining < READ_SIZE) ? remaining : READ_SIZE;
			// read the block
			SPIRead(readpos, writepos, readlen);
			readpos += readlen;
			// increment next write position
			writepos += readlen;
			// decrement remaining count
			remaining -= readlen;
		}
	}

	return entry;
}

void call_user_start(uint32_t readpos)
{
#ifdef BOOT_NO_ASM
	rom_entry_fn_t entry = load_rom(readpos);

	entry();
#else
	__asm volatile (
		"mov a15, a0\n"     // store return addr, we already splatted a15!
		"call0 load_rom\n"  // load the rom
		"mov a0, a15\n"     // restore return addr
		"jx a2\n"           // now jump to the rom code
	);
#endif
}
