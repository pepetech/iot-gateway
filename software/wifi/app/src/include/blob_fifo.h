#ifndef __BLOB_FIFO_H__
#define __BLOB_FIFO_H__

#include <c_types.h>
#include "ets_func.h"

typedef struct
{
	uint8_t *buffer;
	uint8_t buffer_allocated : 1;
	uint32_t buffer_size;
	uint8_t *read_ptr;
	uint8_t *tmp_read_ptr;
	uint8_t *write_ptr;
	uint8_t *tmp_write_ptr;
	uint32_t used_size;
	uint32_t tmp_used_size;
} blob_fifo_t;

blob_fifo_t* ICACHE_FLASH_ATTR blob_fifo_init(uint8_t *buffer, uint32_t size);
void ICACHE_FLASH_ATTR blob_fifo_delete(blob_fifo_t *fifo);
uint8_t ICACHE_FLASH_ATTR blob_fifo_write(blob_fifo_t *fifo, uint8_t *data, uint32_t size);
uint8_t ICACHE_FLASH_ATTR blob_fifo_read(blob_fifo_t *fifo, uint8_t *data, uint32_t *size, uint32_t max_size);
static inline uint8_t ICACHE_FLASH_ATTR blob_fifo_is_empty(blob_fifo_t *fifo)
{
    if(!fifo)
        return 1;

	return !fifo->used_size;
}
static inline uint8_t ICACHE_FLASH_ATTR blob_fifo_is_full(blob_fifo_t *fifo)
{
    if(!fifo)
        return 1;

	return fifo->used_size >= fifo->buffer_size;
}

#endif
