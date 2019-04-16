#ifndef __MD5_H__
#define __MD5_H__

#include <c_types.h>

#define MD5STRU "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"
#define MD5STRL "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define MD52STR(hash) hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9], hash[10], hash[11], hash[12], hash[13], hash[14], hash[15]

typedef struct {
    uint32_t state[4];
    uint32_t count[2];
    uint8_t buffer[64];
} md5_context_t;

extern void MD5Init(md5_context_t *);
extern void MD5Update(md5_context_t *, const uint8_t *, const uint16_t);
extern void MD5Final(uint8_t [16], md5_context_t *);

#endif
