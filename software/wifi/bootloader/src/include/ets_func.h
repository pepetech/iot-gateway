#ifndef __ETS_FUNC_H__
#define __ETS_FUNC_H__

#include <c_types.h>

extern uint32_t SPIRead(uint32_t, void *, uint32_t);
extern uint32_t SPIEraseSector(int);
extern uint32_t SPIWrite(uint32_t, void *, uint32_t);

extern void ets_printf(const char*, ...);
extern void ets_delay_us(int);
extern void ets_memset(void*, uint8_t, uint32_t);
extern void ets_memcpy(void*, const void*, uint32_t);

#endif
