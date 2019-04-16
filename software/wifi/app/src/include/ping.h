#ifndef __PING_H__
#define __PING_H__

#include <c_types.h>

typedef void (* ping_recv_fn_t)(void *, void *);
typedef void (* ping_sent_fn_t)(void *, void *);

typedef struct
{
	uint32_t count;
	uint32_t ip;
	uint32_t coarse_time;
	ping_recv_fn_t recv_function;
	ping_sent_fn_t sent_function;
	void *reverse;
} ping_option_t;

typedef struct
{
	uint32_t total_count;
	uint32_t resp_time;
	uint32_t seqno;
	uint32_t timeout_count;
	uint32_t bytes;
	uint32_t total_bytes;
	uint32_t total_time;
	uint8_t ping_err;
} ping_resp_t;

extern bool ping_start(ping_option_t *ping_opt);
extern bool ping_regist_recv(ping_option_t *ping_opt, ping_recv_fn_t ping_recv);
extern bool ping_regist_sent(ping_option_t *ping_opt, ping_sent_fn_t ping_sent);

#endif
