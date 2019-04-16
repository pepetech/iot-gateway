#ifndef __ETS_FUNC_H__
#define __ETS_FUNC_H__

#include <c_types.h>
#include <stdarg.h>

// Clock
#define APB_CLK_FREQ     80 * 1000000
#define UART_CLK_FREQ    APB_CLK_FREQ
#define TIMER_CLK_FREQ   (APB_CLK_FREQ >> 8)

// Heap management
extern void *pvPortMalloc(size_t, const char *, unsigned);
extern void vPortFree(void *, const char *, unsigned);
extern void *pvPortZalloc(size_t, const char *, unsigned);
extern void *pvPortCalloc(size_t, size_t, const char *, unsigned);
extern void *pvPortRealloc(void *, size_t, const char *, unsigned);

#define ets_free(s)        vPortFree(s, "", 0)
#define ets_malloc(s)      pvPortMalloc(s, "", 0)
#define ets_realloc(p, s)  pvPortRealloc(p, s, "", 0)
#define ets_calloc(l, s)   pvPortCalloc(l, s, "", 0)
#define ets_zalloc(s)      pvPortZalloc(s, "", 0)

extern void ets_bzero(void *, size_t);

// Memory management
extern int ets_memcmp(const void *, const void *, unsigned int);
extern void *ets_memcpy(void *, const void *, unsigned int);
extern void *ets_memmove(void *, const void *, unsigned int);
extern void *ets_memset(void *, int, unsigned int);
extern int ets_strcmp(const char *, const char *);
extern char *ets_strcpy(char *, const char *);
extern int ets_strncmp(const char *, const char *, unsigned int);
extern int ets_strlen(const char *);
extern char *ets_strncpy(char *, const char *, unsigned int);
extern char *ets_strstr(const char *, const char *);

// Timers
typedef struct ets_timer_t ets_timer_t;
typedef void (* ets_timer_callback_fn_t)(void *);

struct ets_timer_t
{
    ets_timer_t *timer_next;
    uint32_t timer_expire;
    uint32_t timer_period;
    ets_timer_callback_fn_t timer_func;
    void *timer_arg;
};

extern void ets_timer_arm_new(ets_timer_t *, uint32_t, uint8_t, uint8_t);
extern void ets_timer_disarm(ets_timer_t *);
extern void ets_timer_setfn(ets_timer_t *, ets_timer_callback_fn_t, void *);

// Delay
extern void ets_delay_us(uint32_t);

// Tasks
typedef struct ets_event_t ets_event_t;
typedef void (* ets_task_t)(ets_event_t *);

struct ets_event_t
{
    uint32_t sig;
    void *par;
};

extern bool ets_task(ets_task_t, uint8_t, ets_event_t *, uint8_t);
extern bool ets_post(uint8_t, uint32_t, void *);

// CPU
extern void ets_update_cpu_frequency(uint32_t);
extern uint32_t ets_get_cpu_frequency();

// Interrupts
typedef void (* ets_isr_t)(void *);

extern void ets_isr_mask(int);
extern void ets_isr_unmask(int);
extern void ets_isr_attach(int, ets_isr_t, void *);
extern void ets_intr_lock();
extern void ets_intr_unlock();

#define ETS_INUM_WDEV_FIQ   0
#define ETS_INUM_SLC        1
#define ETS_INUM_SPI        2
#define ETS_INUM_RTC        3
#define ETS_INUM_GPIO       4
#define ETS_INUM_UART       5
#define ETS_INUM_TICK       6
#define ETS_INUM_SOFT       7
#define ETS_INUM_TIMER_FRC1 9
#define ETS_INUM_WDT        8
#define ETS_INUM_TIMER_FRC2 10

// xxxPrintf
typedef void (* ets_putc_fn_t)(char);

extern void ets_install_putc1(ets_putc_fn_t);
extern void ets_install_putc2(ets_putc_fn_t);
extern void ets_write_char(char);
extern void ets_putc(char);
extern void ets_getc(char *);

extern void ets_vprintf(ets_putc_fn_t, const char*, va_list);
extern void ets_printf(const char*, ...);

int ets_vsprintf(char *, const char *, va_list); // Defined in libmain.a
int ets_sprintf(char *, const char *, ...); // Defined in libmain.a
int ets_vsnprintf(char *, size_t, const char *, va_list); // Defined in libmain.a
int ets_snprintf(char *, size_t, const char *, ...);  // Defined in libmain.a

#define pgm_read_with_offset(addr, res) \
    __asm__("extui    %0, %1, 0, 2\n"     /* Extract offset within word (in bytes) */ \
            "sub      %1, %1, %0\n"       /* Subtract offset from addr, yielding an aligned address */ \
            "l32i.n   %1, %1, 0x0\n"      /* Load word from aligned address */ \
            "slli     %0, %0, 3\n"        /* Mulitiply offset by 8, yielding an offset in bits */ \
            "ssr      %0\n"               /* Prepare to shift by offset (in bits) */ \
            "srl      %0, %1\n"           /* Shift right; now the requested byte is the first one */ \
            :"=r"(res), "=r"(addr) \
            :"1"(addr) \
            :);

static inline uint8_t pgm_read_byte_inlined(const void *addr)
{
    register uint32_t res;
    pgm_read_with_offset(addr, res);
    return (uint8_t) res;     /* This masks the lower byte from the returned word */
}
static inline uint16_t pgm_read_word_inlined(const void *addr)
{
    register uint32_t res;
    pgm_read_with_offset(addr, res);
    return (uint16_t) res;    /* This masks the lower half-word from the returned word */
}

#define pgm_read_byte(addr)     pgm_read_byte_inlined((const void*)(addr))
#define pgm_read_word(addr)     pgm_read_word_inlined((const void*)(addr))
#define pgm_read_dword(addr) 	(*(const uint32_t*)(addr))
#define pgm_read_float(addr) 	(*(const float*)(addr))
#define pgm_read_ptr(addr) 		(*(const void* const *)(addr))

#define PSTR(s) (__extension__({static const char __c[] ICACHE_RODATA_ATTR = (s); &__c[0];}))

int ets_rostrlen(const char *);
void *ets_romemcpy(void *, const void *, unsigned int);
char *ets_rostrcpy(char *, const char *);
void ets_roprintf(const char *, ...);
void ets_rovprintf(const char *, va_list);
int ets_rosprintf(char *, const char *, ...);
int ets_rovsprintf(char *, const char *, va_list);
int ets_rosnprintf(char *, size_t, const char *rofmt, ...);
int ets_rovsnprintf(char *, size_t, const char *rofmt, va_list);

#endif
