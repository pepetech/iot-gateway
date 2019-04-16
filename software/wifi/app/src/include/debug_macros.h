#ifndef __DEBUG_MACROS_H__
#define __DEBUG_MACROS_H__

#define DBGPRINTLN_CTX(FORMAT, ...)  ets_roprintf(PSTR("[%s] - " FORMAT "\r\n"), __FUNCTION__, ##__VA_ARGS__)
#define DBGPRINT_CTX(FORMAT, ...)  ets_roprintf(PSTR("[%s] - " FORMAT), __FUNCTION__, ##__VA_ARGS__)
#define DBGPRINTLN(FORMAT, ...) ets_roprintf(PSTR(FORMAT "\r\n"), ##__VA_ARGS__)
#define DBGPRINT(FORMAT, ...) ets_roprintf(PSTR(FORMAT), ##__VA_ARGS__)

#endif
