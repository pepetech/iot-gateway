#include "blob_fifo.h"

static inline void ICACHE_FLASH_ATTR blob_fifo_commit(blob_fifo_t *fifo)
{
	fifo->read_ptr = fifo->tmp_read_ptr;
	fifo->write_ptr = fifo->tmp_write_ptr;
	fifo->used_size = fifo->tmp_used_size;
}
static inline void ICACHE_FLASH_ATTR blob_fifo_rollback(blob_fifo_t *fifo)
{
	fifo->tmp_read_ptr = fifo->read_ptr;
	fifo->tmp_write_ptr = fifo->write_ptr;
	fifo->tmp_used_size = fifo->used_size;
}
static inline void ICACHE_FLASH_ATTR blob_fifo_reset(blob_fifo_t *fifo)
{
	fifo->used_size = fifo->tmp_used_size = 0;
	fifo->read_ptr = fifo->tmp_read_ptr = fifo->write_ptr = fifo->tmp_write_ptr = fifo->buffer;
}
static uint8_t ICACHE_FLASH_ATTR blob_fifo_write_byte(blob_fifo_t *fifo, uint8_t data)
{
	if(fifo->tmp_used_size >= fifo->buffer_size)
		return 0;

	fifo->tmp_used_size++;
	*fifo->tmp_write_ptr++ = data;

	if(fifo->tmp_write_ptr >= fifo->buffer + fifo->buffer_size)
		fifo->tmp_write_ptr = fifo->buffer;

	return 1;
}
static uint8_t ICACHE_FLASH_ATTR blob_fifo_read_byte(blob_fifo_t *fifo, uint8_t *data)
{
	if(!fifo->tmp_used_size)
		return 0;

	fifo->tmp_used_size--;
	*data = *fifo->tmp_read_ptr++;

	if(fifo->tmp_read_ptr >= fifo->buffer + fifo->buffer_size)
		fifo->tmp_read_ptr = fifo->buffer;

	return 1;
}

blob_fifo_t* blob_fifo_init(uint8_t *buffer, uint32_t size)
{
	if(!size)
		return NULL;

	blob_fifo_t *fifo = (blob_fifo_t *)ets_zalloc(sizeof(blob_fifo_t));

	if(!fifo)
		return NULL;

	fifo->buffer = buffer ? buffer : ets_zalloc(size);

	if(!fifo->buffer)
	{
		ets_free(fifo);

		return NULL;
	}

	if(!buffer)
		fifo->buffer_allocated = 1;

	fifo->buffer_size = size;

	blob_fifo_reset(fifo);

	return fifo;
}
void blob_fifo_delete(blob_fifo_t *fifo)
{
	if(!fifo)
		return;

	if(fifo->buffer_allocated)
		ets_free(fifo->buffer);

	ets_free(fifo);
}
uint8_t blob_fifo_write(blob_fifo_t *fifo, uint8_t *data, uint32_t size)
{
	if(!fifo)
		return 0;

	if(!data || !size)
		return 0;

	if(!blob_fifo_write_byte(fifo, 0x7E))
		return 0;

    while (size--)
	{
        switch (*data)
		{
	        case 0x7D:
	        case 0x7E:
	        case 0x7F:
			{
	        	if(!blob_fifo_write_byte(fifo, 0x7D))
				{
					blob_fifo_rollback(fifo);

					return 0;
				}

	        	if(!blob_fifo_write_byte(fifo, *data++ ^ 0x20))
				{
					blob_fifo_rollback(fifo);

					return 0;
				}
			}
	        break;
	        default:
			{
				if(!blob_fifo_write_byte(fifo, *data++))
				{
					blob_fifo_rollback(fifo);

					return 0;
				}
			}
	        break;
        }
    }

	if(!blob_fifo_write_byte(fifo, 0x7F))
	{
		blob_fifo_rollback(fifo);

		return 0;
	}

	blob_fifo_commit(fifo);

	return 1;
}
uint8_t blob_fifo_read(blob_fifo_t *fifo, uint8_t *data, uint32_t *size, uint32_t max_size)
{
	if(!fifo)
		return 0;

	if(!data || !size || !max_size)
		return 0;

	uint8_t temp;

	if(!blob_fifo_read_byte(fifo, &temp))
		return 0;

	if(temp != 0x7E) // Critical error, first byte should be 0x7E, reset FIFO
	{
		blob_fifo_reset(fifo);

		return 0;
	}

	uint8_t escape = 0;

	*size = 0;

	while(max_size--)
	{
		if(!blob_fifo_read_byte(fifo, &temp)) // Critical error, fifo empty before last byte, 0x7F, reset FIFO
		{
			blob_fifo_reset(fifo);

			return 0;
		}

		switch(temp)
		{
			case 0x7D:
			{
				escape = 1;
			}
			break;
			case 0x7E: // Critical error, unexpected start byte, reset FIFO
			{
				blob_fifo_reset(fifo);

				return 0;
			}
			break;
			case 0x7F:
			{
				blob_fifo_commit(fifo);

				return 1;
			}
			break;
			default:
			{
				*data++ = escape ? (temp ^ 0x20) : temp;
				(*size)++;
				escape = 0;
			}
			break;
		}
	}

	blob_fifo_rollback(fifo);

	return 0;
}
