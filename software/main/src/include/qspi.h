#ifndef __QSPI_H__
#define __QSPI_H__

#include <em_device.h>
#include <stdlib.h>
#include "systick.h"
#include "atomic.h"
#include "utils.h"

// Configuration commands
#define QSPI_FLASH_CMD_NOP                  0x00
#define QSPI_FLASH_CMD_RESET_ENABLE         0x66
#define QSPI_FLASH_CMD_RESET                0x99
#define QSPI_FLASH_CMD_ENABLE_QIO           0x38
#define QSPI_FLASH_CMD_RESET_QIO            0xFF
#define QSPI_FLASH_CMD_READ_STATUS          0x05
#define QSPI_FLASH_CMD_WRITE_STATUS         0x01
//#define QSPI_FLASH_CMD_READ_STATUS_2      0x35
//#define QSPI_FLASH_CMD_WRITE_STATUS_2     0x31
//#define QSPI_FLASH_CMD_READ_STATUS_3      0x15
//#define QSPI_FLASH_CMD_WRITE_STATUS_3     0x11
#define QSPI_FLASH_CMD_READ_CFG             0x35

// Read commands
#define QSPI_FLASH_CMD_READ                 0x03
#define QSPI_FLASH_CMD_READ_FAST            0x0B
#define QSPI_FLASH_CMD_READ_FAST_QOUT       0x6B
#define QSPI_FLASH_CMD_READ_FAST_QIO        0xEB
#define QSPI_FLASH_CMD_READ_FAST_DOUT       0x3B
#define QSPI_FLASH_CMD_READ_FAST_DIO        0xBB
#define QSPI_FLASH_CMD_SET_BURST            0xC0
#define QSPI_FLASH_CMD_READ_BURST_SQI       0x0C
#define QSPI_FLASH_CMD_READ_BURST           0xEC

// Identification commands
//#define QSPI_FLASH_CMD_READ_ID            0xAB
//#define QSPI_FLASH_CMD_READ_UID           0x4B
#define QSPI_FLASH_CMD_JEDEC_READ_ID        0x9F
#define QSPI_FLASH_CMD_JEDEC_READ_ID_QIO    0xAF
#define QSPI_FLASH_CMD_SFDP                 0x5A

// Write commands
#define QSPI_FLASH_CMD_WRITE_ENABLE         0x06
#define QSPI_FLASH_CMD_WRITE_DISABLE        0x04
#define QSPI_FLASH_CMD_SECTOR_ERASE         0x20
#define QSPI_FLASH_CMD_BLOCK_ERASE          0xD8
//#define QSPI_FLASH_CMD_BLOCK_ERASE_32K    0x52
//#define QSPI_FLASH_CMD_BLOCK_ERASE_64K    0xD8
#define QSPI_FLASH_CMD_CHIP_ERASE           0xC7
#define QSPI_FLASH_CMD_WRITE                0x02
#define QSPI_FLASH_CMD_WRITE_QIO            0x32
#define QSPI_FLASH_CMD_SUSPEND              0xB0
#define QSPI_FLASH_CMD_RESUME               0x30

// Protection commands
#define QSPI_FLASH_CMD_READ_PROTECTION      0x72
#define QSPI_FLASH_CMD_WRITE_PROTECTION     0x42
#define QSPI_FLASH_CMD_LOCK_PROTECTION      0x8D
#define QSPI_FLASH_CMD_nVWRITE_PROTECTION   0xE8
#define QSPI_FLASH_CMD_GLOBAL_UNPROTECT     0x98
//#define QSPI_FLASH_CMD_GLOBAL_PROTECT     0x7E
//#define QSPI_FLASH_CMD_QUERY_PROTECT      0x3D
//#define QSPI_FLASH_CMD_UNPROTECT          0x39
//#define QSPI_FLASH_CMD_PROTECT            0x36

// Security area commands
#define QSPI_FLASH_CMD_READ_SECURITY        0x88
#define QSPI_FLASH_CMD_WRITE_SECURITY       0xA5
#define QSPI_FLASH_CMD_LOCK_SECURITY        0x85
//#define QSPI_FLASH_CMD_ERASE_SECURITY_REG 0x44
//#define QSPI_FLASH_CMD_READ_SECURITY_REG  0x48
//#define QSPI_FLASH_CMD_WRITE_SECURITY_REG 0x42

// Power saving commands
#define QSPI_FLASH_CMD_POWER_DOWN           0xB9
#define QSPI_FLASH_CMD_RELEASE_POWER_DOWN   0xAB


// Known IDs
//#define QSPI_FLASH_JEDEC_ID   0x00EF4016 // W25Q32JV-IQ/JQ
//#define QSPI_FLASH_JEDEC_ID   0x00EF7016 // W25Q32JV-IM/JM
//#define QSPI_FLASH_JEDEC_ID   0x00BF2641 // SST26VF016B
#define QSPI_FLASH_JEDEC_ID     0x00BF2643 // SST26VF064B


#define QSPI_FLASH_SIZE             ((uint32_t)0x800000) // 8 MB (64 Mbit)
#define QSPI_FLASH_SECTOR_SIZE      ((uint32_t)0x001000) // 4 KB
#define QSPI_FLASH_PAGE_SIZE        ((uint32_t)0x000100) // 256 B

#define QSPI_FLASH_SECTOR(i)        (QSPI_FLASH_SECTOR_SIZE * (i))
#define QSPI_FLASH_PAGE(i)          (QSPI_FLASH_PAGE_SIZE * (i))

#define QSPI_FLASH_SECTOR_MASK      (QSPI_FLASH_MAX_ADDRESS & ~(QSPI_FLASH_SECTOR_SIZE - 1))
#define QSPI_FLASH_PAGE_MASK        (QSPI_FLASH_MAX_ADDRESS & ~(QSPI_FLASH_PAGE_SIZE - 1))

#define QSPI_FLASH_MAX_ADDRESS      (QSPI_FLASH_SIZE - 1)


void qspi_init();
void qspi_enter_xip();
void qspi_exit_xip();

void qspi_flash_cmd(uint8_t ubOpCode, uint32_t ulAddress, uint8_t ubAddressSize, uint8_t ubModeBits, uint8_t ubDummyCycles, uint8_t *pubSrc, uint8_t ubSrcSize, uint8_t *pubDst, uint8_t ubDstSize);

void qspi_flash_init();
uint32_t qspi_flash_read_jedec_id();
void qspi_flash_busy_wait();
void qspi_flash_write_enable();
void qspi_flash_write_disable();
void qspi_flash_sector_erase(uint32_t ulAddress);
void qspi_flash_chip_erase();
uint8_t qspi_flash_read_status();

#if defined(QSPI_FLASH_CMD_READ_CFG)
uint8_t qspi_flash_read_config();
void qspi_flash_write_status_config(uint8_t ubStatus, uint8_t ubConfig);
#else
void qspi_flash_write_status(uint8_t ubStatus);
#if defined(QSPI_FLASH_CMD_READ_STATUS_2) && defined(QSPI_FLASH_CMD_WRITE_STATUS_2)
uint8_t qspi_flash_read_status_2();
void qspi_flash_write_status_2(uint8_t ubStatus);
#endif
#if defined(QSPI_FLASH_CMD_READ_STATUS_3) && defined(QSPI_FLASH_CMD_WRITE_STATUS_3)
uint8_t qspi_flash_read_status_3();
void qspi_flash_write_status_3(uint8_t ubStatus);
#endif
#endif

#if defined(QSPI_FLASH_CMD_RESET_ENABLE) && defined(QSPI_FLASH_CMD_RESET)
void qspi_flash_reset();
#endif

#if defined(QSPI_FLASH_CMD_SUSPEND) && defined(QSPI_FLASH_CMD_RESUME)
void qspi_flash_suspend();
void qspi_flash_resume();
#endif

#if defined(QSPI_FLASH_CMD_BLOCK_ERASE)
void qspi_flash_block_erase(uint32_t ulAddress);
#else
#if defined(QSPI_FLASH_CMD_BLOCK_ERASE_32K)
void qspi_flash_32k_block_erase(uint32_t ulAddress);
#endif
#if defined(QSPI_FLASH_CMD_BLOCK_ERASE_64K)
void qspi_flash_64k_block_erase(uint32_t ulAddress);
#endif
#endif

#if defined(QSPI_FLASH_CMD_READ_ID)
uint8_t qspi_flash_read_device_id();
#endif

#if defined(QSPI_FLASH_CMD_READ_UID)
uint64_t qspi_flash_read_unique_id();
#endif

#if defined(QSPI_FLASH_CMD_READ_SECURITY) && defined(QSPI_FLASH_CMD_WRITE_SECURITY)
void qspi_flash_read_security(uint16_t usAddress, uint8_t *pubDst, uint16_t usCount);
void qspi_flash_write_security(uint16_t usAddress, uint8_t *pubSrc, uint16_t usCount);
#endif

#if defined(QSPI_FLASH_CMD_LOCK_SECURITY)
void qspi_flash_lock_security();
#endif

#if defined(QSPI_FLASH_CMD_READ_SECURITY_REG) && defined(QSPI_FLASH_CMD_WRITE_SECURITY_REG)
void qspi_flash_read_security_register(uint8_t ubIndex, uint8_t ubAddress, uint8_t *pubDst, uint16_t usCount);
void qspi_flash_write_security_register(uint8_t ubIndex, uint8_t ubAddress, uint8_t *pubSrc, uint16_t usCount);
#endif

#if defined(QSPI_FLASH_CMD_ERASE_SECURITY_REG)
void qspi_flash_erase_security_register(uint8_t ubIndex);
#endif

#if defined(QSPI_FLASH_CMD_POWER_DOWN) && defined(QSPI_FLASH_CMD_RELEASE_POWER_DOWN)
void qspi_flash_power_down(uint8_t ubPower);
#endif

#if defined(QSPI_FLASH_CMD_READ_PROTECTION) && defined(QSPI_FLASH_CMD_WRITE_PROTECTION)
void qspi_flash_protect_blocks(uint64_t ullMask);
void qspi_flash_unprotect_blocks(uint64_t ullMask);
uint64_t qspi_flash_read_protection();
#endif

#if defined(QSPI_FLASH_CMD_LOCK_PROTECTION)
void qspi_flash_lock_protection();
#endif

#if defined(QSPI_FLASH_CMD_nVWRITE_PROTECTION)
void qspi_flash_write_nv_protection(uint64_t ullProtection);
#endif

#if defined(QSPI_FLASH_CMD_GLOBAL_UNPROTECT)
void qspi_flash_unprotect_all_blocks();
#endif

#if defined(QSPI_FLASH_CMD_GLOBAL_PROTECT)
void qspi_flash_protect_all_blocks();
#endif

#if defined(QSPI_FLASH_CMD_QUERY_PROTECT)
uint8_t qspi_flash_is_block_protected(uint32_t ulAddress);
#endif

#if defined(QSPI_FLASH_CMD_UNPROTECT)
void qspi_flash_unprotect_block(uint32_t ulAddress);
#endif

#if defined(QSPI_FLASH_CMD_PROTECT)
void qspi_flash_protect_block(uint32_t ulAddress);
#endif

#endif  // __QSPI_H__
