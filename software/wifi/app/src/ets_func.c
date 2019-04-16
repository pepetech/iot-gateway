#include "ets_func.h"

int ets_rostrlen(const char *rostr)
{
    int len = 0;

    while (pgm_read_byte(rostr++) != '\0')
        len++;

    return len;
}
void *ets_romemcpy(void *dst, const void *rosrc, unsigned int count)
{
    const uint8_t* read = (const uint8_t*)rosrc;
    uint8_t* write = (uint8_t*)dst;

    while (count--)
        *write++ = pgm_read_byte(read++);

    return dst;
}
char *ets_rostrcpy(char *dst, const char *rosrc)
{
    char *write = dst;

    while (pgm_read_byte(rosrc) != '\0')
        *write++ = pgm_read_byte(rosrc++);

    *write = '\0';

    return dst;
}
void ets_roprintf(const char *rofmt, ...)
{
    int fmt_len = ets_rostrlen(rofmt);
    char* fmt = ets_zalloc(fmt_len + 1);

    if(!fmt)
        return;

    ets_rostrcpy(fmt, rofmt);

    va_list arg;
    va_start(arg, rofmt);
    ets_vprintf(ets_write_char, fmt, arg);
    va_end(arg);

    ets_free(fmt);
}
void ets_rovprintf(const char *rofmt, va_list arg)
{
    int fmt_len = ets_rostrlen(rofmt);
    char* fmt = ets_zalloc(fmt_len + 1);

    if(!fmt)
        return;

    ets_rostrcpy(fmt, rofmt);

    ets_vprintf(ets_write_char, fmt, arg);

    ets_free(fmt);
}
int ets_rosprintf(char *dst, const char *rofmt, ...)
{
    int fmt_len = ets_rostrlen(rofmt);
    char* fmt = ets_zalloc(fmt_len + 1);

    if(!fmt)
        return 0;

    ets_rostrcpy(fmt, rofmt);

    int ret;

    va_list arg;
    va_start(arg, rofmt);
    ret = ets_vsprintf(dst, fmt, arg);
    va_end(arg);

    ets_free(fmt);

    return ret;
}
int ets_rovsprintf(char *dst, const char *rofmt, va_list arg)
{
    int fmt_len = ets_rostrlen(rofmt);
    char* fmt = ets_zalloc(fmt_len + 1);

    if(!fmt)
        return 0;

    ets_rostrcpy(fmt, rofmt);

    int ret = ets_vsprintf(dst, fmt, arg);

    ets_free(fmt);

    return ret;
}
int ets_rosnprintf(char *dst, size_t dst_size, const char *rofmt, ...)
{
    int fmt_len = ets_rostrlen(rofmt);
    char* fmt = ets_zalloc(fmt_len + 1);

    if(!fmt)
        return 0;

    ets_rostrcpy(fmt, rofmt);

    int ret;

    va_list arg;
    va_start(arg, rofmt);
    ret = ets_vsnprintf(dst, dst_size, fmt, arg);
    va_end(arg);

    ets_free(fmt);

    return ret;
}
int ets_rovsnprintf(char *dst, size_t dst_size, const char *rofmt, va_list arg)
{
    int fmt_len = ets_rostrlen(rofmt);
    char* fmt = ets_zalloc(fmt_len + 1);

    if(!fmt)
        return 0;

    ets_rostrcpy(fmt, rofmt);

    int ret = ets_vsnprintf(dst, dst_size, fmt, arg);

    ets_free(fmt);

    return ret;
}
