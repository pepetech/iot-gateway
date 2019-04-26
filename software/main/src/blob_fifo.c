#include "blob_fifo.h"

static inline void blob_fifo_commit(blob_fifo_t *pFIFO)
{
	pFIFO->pubRead = pFIFO->pubTempRead;
	pFIFO->pubWrite = pFIFO->pubTempWrite;
	pFIFO->ulUsedSize = pFIFO->ulTempUsedSize;
}
static inline void blob_fifo_rollback(blob_fifo_t *pFIFO)
{
	pFIFO->pubTempRead = pFIFO->pubRead;
	pFIFO->pubTempWrite = pFIFO->pubWrite;
	pFIFO->ulTempUsedSize = pFIFO->ulUsedSize;
}
static inline void blob_fifo_reset(blob_fifo_t *pFIFO)
{
	pFIFO->ulUsedSize = pFIFO->ulTempUsedSize = 0;
	pFIFO->pubRead = pFIFO->pubTempRead = pFIFO->pubWrite = pFIFO->pubTempWrite = pFIFO->pubBuffer;
}
static uint8_t blob_fifo_write_byte(blob_fifo_t *pFIFO, uint8_t ubData)
{
	if(pFIFO->ulTempUsedSize >= pFIFO->ulBufferSize)
		return 0;

	pFIFO->ulTempUsedSize++;
	*pFIFO->pubTempWrite++ = ubData;

	if(pFIFO->pubTempWrite >= pFIFO->pubBuffer + pFIFO->ulBufferSize)
		pFIFO->pubTempWrite = pFIFO->pubBuffer;

	return 1;
}
static uint8_t blob_fifo_read_byte(blob_fifo_t *pFIFO, uint8_t *pubData)
{
	if(!pFIFO->ulTempUsedSize)
		return 0;

	pFIFO->ulTempUsedSize--;
	*pubData = *pFIFO->pubTempRead++;

	if(pFIFO->pubTempRead >= pFIFO->pubBuffer + pFIFO->ulBufferSize)
		pFIFO->pubTempRead = pFIFO->pubBuffer;

	return 1;
}

blob_fifo_t* blob_fifo_init(uint8_t *pubBuffer, uint32_t ulSize)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(!ulSize)
			return NULL;

		blob_fifo_t *pNewFIFO = (blob_fifo_t *)malloc(sizeof(blob_fifo_t));

		if(!pNewFIFO)
			return NULL;

		memset(pNewFIFO, 0, sizeof(blob_fifo_t));

		pNewFIFO->pubBuffer = pubBuffer ? pubBuffer : (uint8_t *)malloc(ulSize);

		if(!pNewFIFO->pubBuffer)
		{
			free(pNewFIFO);

			return NULL;
		}

		memset(pNewFIFO->pubBuffer, 0, ulSize);

		if(!pubBuffer)
			pNewFIFO->ubBufferAllocated = 1;

		pNewFIFO->ulBufferSize = ulSize;

		blob_fifo_reset(pNewFIFO);

		return pNewFIFO;
	}
}
void blob_fifo_delete(blob_fifo_t *pFIFO)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(!pFIFO)
			return;

		if(pFIFO->ubBufferAllocated)
			free(pFIFO->pubBuffer);

		free(pFIFO);
	}
}
uint8_t blob_fifo_write(blob_fifo_t *pFIFO, const uint8_t *pubData, uint32_t ulSize)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(!pFIFO)
			return 0;

		if(!pubData || !ulSize)
			return 0;

		if(!blob_fifo_write_byte(pFIFO, 0x7E))
			return 0;

		while (ulSize--)
		{
			switch (*pubData)
			{
				case 0x7D:
				case 0x7E:
				case 0x7F:
				{
					if(!blob_fifo_write_byte(pFIFO, 0x7D))
					{
						blob_fifo_rollback(pFIFO);

						return 0;
					}

					if(!blob_fifo_write_byte(pFIFO, (*pubData++) ^ 0x20))
					{
						blob_fifo_rollback(pFIFO);

						return 0;
					}
				}
				break;
				default:
				{
					if(!blob_fifo_write_byte(pFIFO, *pubData++))
					{
						blob_fifo_rollback(pFIFO);

						return 0;
					}
				}
				break;
			}
		}

		if(!blob_fifo_write_byte(pFIFO, 0x7F))
		{
			blob_fifo_rollback(pFIFO);

			return 0;
		}

		blob_fifo_commit(pFIFO);

		return 1;
	}
}
uint8_t blob_fifo_read(blob_fifo_t *pFIFO, uint8_t *pubData, uint32_t *pulSize, uint32_t ulMaxSize)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(!pFIFO)
			return 0;

		if(!pubData || !pulSize || !ulMaxSize)
			return 0;

		uint8_t ubTempData;

		if(!blob_fifo_read_byte(pFIFO, &ubTempData))
			return 0;

		if(ubTempData != 0x7E) // Critical error, first byte should be 0x7E, reset FIFO
		{
			blob_fifo_reset(pFIFO);

			return 0;
		}

		uint8_t ubEscape = 0;

		*pulSize = 0;

		while(1)
		{
			if(!blob_fifo_read_byte(pFIFO, &ubTempData)) // Critical error, fifo empty before last byte, 0x7F, reset FIFO
			{
				blob_fifo_reset(pFIFO);

				return 0;
			}

			switch(ubTempData)
			{
				case 0x7D: // Critical error, unexpected escape byte, reset FIFO
				{
				    if(ubEscape)
				    {
    					blob_fifo_reset(pFIFO);

    					return 0;
				    }

					ubEscape = 1;
				}
				break;
				case 0x7E: // Critical error, unexpected start byte, reset FIFO
				{
					blob_fifo_reset(pFIFO);

					return 0;
				}
				break;
				case 0x7F:
				{
					blob_fifo_commit(pFIFO);

					return 1;
				}
				break;
				default:
				{
				    if(!ulMaxSize)
				    {
				        blob_fifo_rollback(pFIFO);

                		return 0;
				    }

					*pubData++ = ubEscape ? (ubTempData ^ 0x20) : ubTempData;
					(*pulSize)++;
					ulMaxSize--;
					ubEscape = 0;
				}
				break;
			}
		}

		blob_fifo_rollback(pFIFO);

		return 0;
	}
}
