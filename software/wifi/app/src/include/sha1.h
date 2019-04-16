#ifndef __SHA1_H__
#define __SHA1_H__

#include <c_types.h>

#define SHA1STRU "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"
#define SHA1STRL "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define SHA12STR(hash) hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9], hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19]

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} sha1_context_t;

extern void SHA1Transform(uint32_t [5], uint8_t [64]);
extern void SHA1Init(sha1_context_t *);
extern void SHA1Update(sha1_context_t *, const uint8_t *, const uint32_t);
extern void SHA1Final(uint8_t [20], sha1_context_t *);

#endif
