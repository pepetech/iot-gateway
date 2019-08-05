#ifndef __BLOB_FIFO_H__
#define __BLOB_FIFO_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "atomic.h"

typedef struct
{
    uint8_t *pubBuffer;
    uint8_t ubBufferAllocated : 1;
    uint32_t ulBufferSize;
    volatile uint8_t *pubRead;
    volatile uint8_t *pubTempRead;
    volatile uint8_t *pubWrite;
    volatile uint8_t *pubTempWrite;
    volatile uint32_t ulUsedSize;
    volatile uint32_t ulTempUsedSize;
} blob_fifo_t;

blob_fifo_t* blob_fifo_init(uint8_t *pubBuffer, uint32_t ulSize);
void blob_fifo_delete(blob_fifo_t *pFIFO);
uint8_t blob_fifo_write(blob_fifo_t *pFIFO, const uint8_t *pubData, uint32_t ulSize);
uint8_t blob_fifo_read(blob_fifo_t *pFIFO, uint8_t *pubData, uint32_t *pulSize, uint32_t ulMaxSize);
static inline uint8_t blob_fifo_is_empty(blob_fifo_t *pFIFO)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if(!pFIFO)
            return 1;

        return !pFIFO->ulUsedSize;
    }
}
static inline uint8_t blob_fifo_is_full(blob_fifo_t *pFIFO)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if(!pFIFO)
            return 1;

        return pFIFO->ulUsedSize >= pFIFO->ulBufferSize;
    }
}

#endif
