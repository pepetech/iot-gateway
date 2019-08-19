#include "qspi.h"

void qspi_init()
{
    CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_QSPI0;

    while(!(QSPI0->CONFIG & QSPI_CONFIG_IDLE));

    QSPI0->CONFIG = QSPI_CONFIG_ENBAHBADDRREMAP | (0 << _QSPI_CONFIG_PERIPHCSLINES_SHIFT) | QSPI_CONFIG_ENBDIRACCCTLR | QSPI_CONFIG_PHYMODEENABLE;
    QSPI0->DEVINSTRRDCONFIG = (4 << _QSPI_DEVINSTRRDCONFIG_DUMMYRDCLKCYCLES_SHIFT) | QSPI_DEVINSTRRDCONFIG_MODEBITENABLE | (2 << _QSPI_DEVINSTRRDCONFIG_DATAXFERTYPEEXTMODE_SHIFT) | (2 << _QSPI_DEVINSTRRDCONFIG_ADDRXFERTYPESTDMODE_SHIFT) | (0 << _QSPI_DEVINSTRRDCONFIG_INSTRTYPE_SHIFT) | ((uint32_t)QSPI_FLASH_CMD_READ_FAST_QIO << _QSPI_DEVINSTRRDCONFIG_RDOPCODENONXIP_SHIFT);
    QSPI0->DEVINSTRWRCONFIG = (0 << _QSPI_DEVINSTRWRCONFIG_DUMMYWRCLKCYCLES_SHIFT) | (2 << _QSPI_DEVINSTRWRCONFIG_DATAXFERTYPEEXTMODE_SHIFT) | (2 << _QSPI_DEVINSTRWRCONFIG_ADDRXFERTYPESTDMODE_SHIFT) | ((uint32_t)QSPI_FLASH_CMD_WRITE_QIO << _QSPI_DEVINSTRWRCONFIG_WROPCODE_SHIFT);
    QSPI0->DEVDELAY = (2 << _QSPI_DEVDELAY_DNSS_SHIFT) | (2 << _QSPI_DEVDELAY_DBTWN_SHIFT) | (0 << _QSPI_DEVDELAY_DAFTER_SHIFT) | (0 << _QSPI_DEVDELAY_DINIT_SHIFT);
    QSPI0->DEVSIZECONFIG = (0 << _QSPI_DEVSIZECONFIG_MEMSIZEONCS0_SHIFT) | (12 << _QSPI_DEVSIZECONFIG_BYTESPERSUBSECTOR_SHIFT) | (256 << _QSPI_DEVSIZECONFIG_BYTESPERDEVICEPAGE_SHIFT) | (2 << _QSPI_DEVSIZECONFIG_NUMADDRBYTES_SHIFT);
    QSPI0->REMAPADDR = 0x00000000;
    QSPI0->MODEBITCONFIG = (0x00 << _QSPI_MODEBITCONFIG_MODE_SHIFT);
    QSPI0->WRITECOMPLETIONCTRL = (5 << _QSPI_WRITECOMPLETIONCTRL_POLLREPDELAY_SHIFT) | (1 << _QSPI_WRITECOMPLETIONCTRL_POLLCOUNT_SHIFT) | (0 << _QSPI_WRITECOMPLETIONCTRL_POLLINGPOLARITY_SHIFT) | (0 << _QSPI_WRITECOMPLETIONCTRL_POLLINGBITINDEX_SHIFT) | (QSPI_FLASH_CMD_READ_STATUS << _QSPI_WRITECOMPLETIONCTRL_OPCODE_SHIFT);
    QSPI0->POLLINGFLASHSTATUS = (0 << _QSPI_POLLINGFLASHSTATUS_DEVICESTATUSNBDUMMY_SHIFT);
    QSPI0->PHYCONFIGURATION = QSPI_PHYCONFIGURATION_PHYCONFIGRESYNC | (25 << _QSPI_PHYCONFIGURATION_PHYCONFIGTXDLLDELAY_SHIFT) | (43 << _QSPI_PHYCONFIGURATION_PHYCONFIGRXDLLDELAY_SHIFT);
    QSPI0->OPCODEEXTUPPER = (QSPI_FLASH_CMD_WRITE_ENABLE << _QSPI_OPCODEEXTUPPER_WELOPCODE_SHIFT);
    QSPI0->ROUTELOC0 = QSPI_ROUTELOC0_QSPILOC_LOC0;
    QSPI0->ROUTEPEN = QSPI_ROUTEPEN_DQ0PEN | QSPI_ROUTEPEN_DQ1PEN | QSPI_ROUTEPEN_DQ2PEN | QSPI_ROUTEPEN_DQ3PEN | QSPI_ROUTEPEN_CS0PEN | QSPI_ROUTEPEN_SCLKPEN;

    CMU->QSPICTRL = CMU_QSPICTRL_QSPI0CLKSEL_USHFRCO;

    cmu_update_clocks();

    QSPI0->CONFIG |= QSPI_CONFIG_ENBSPI;

    while(!(QSPI0->CONFIG & QSPI_CONFIG_IDLE));

    qspi_flash_init();
}
void qspi_enter_xip()
{
    while(!(QSPI0->CONFIG & QSPI_CONFIG_IDLE));

    QSPI0->MODEBITCONFIG = (QSPI0->MODEBITCONFIG & ~_QSPI_MODEBITCONFIG_MODE_MASK) | (0xA0 << _QSPI_MODEBITCONFIG_MODE_SHIFT);
    QSPI0->CONFIG |= QSPI_CONFIG_ENTERXIPMODE;
}
void qspi_exit_xip()
{
    while(!(QSPI0->CONFIG & QSPI_CONFIG_IDLE));

    QSPI0->CONFIG &= ~(QSPI_CONFIG_ENTERXIPMODE | QSPI_CONFIG_ENTERXIPMODEIMM);
    QSPI0->MODEBITCONFIG = (QSPI0->MODEBITCONFIG & ~_QSPI_MODEBITCONFIG_MODE_MASK) | (0x00 << _QSPI_MODEBITCONFIG_MODE_SHIFT);
    QSPI0->DEVINSTRRDCONFIG &= ~_QSPI_DEVINSTRRDCONFIG_INSTRTYPE_MASK;

    REG_DISCARD(QSPI0_MEM_BASE);

    while(!(QSPI0->CONFIG & QSPI_CONFIG_IDLE));
}


void qspi_flash_cmd(uint8_t ubOpCode, uint32_t ulAddress, uint8_t ubAddressSize, uint8_t ubModeBits, uint8_t ubDummyCycles, uint8_t *pubSrc, uint8_t ubSrcSize, uint8_t *pubDst, uint8_t ubDstSize)
{
    if(ubAddressSize > 4)
        return;

    if(ubDummyCycles > 31)
        return;

    if(ubSrcSize > 8)
        return;

    if(ubSrcSize && !pubSrc)
        return;

    if(ubDstSize > 16)
        return;

    if(ubDstSize && !pubDst)
        return;

    while(!(QSPI0->CONFIG & QSPI_CONFIG_IDLE));

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        QSPI0->CONFIG &= ~(QSPI_CONFIG_ENBDIRACCCTLR | QSPI_CONFIG_ENBSPI);
        QSPI0->CONFIG |= QSPI_CONFIG_ENBSPI;

        QSPI0->FLASHCMDCTRL = ((uint32_t)ubOpCode << _QSPI_FLASHCMDCTRL_CMDOPCODE_SHIFT) | ((uint32_t)ubDummyCycles << _QSPI_FLASHCMDCTRL_NUMDUMMYCYCLES_SHIFT) | (!!ubModeBits << _QSPI_FLASHCMDCTRL_ENBMODEBIT_SHIFT);

        if(ubAddressSize)
        {
            QSPI0->FLASHCMDCTRL |= QSPI_FLASHCMDCTRL_ENBCOMDADDR | ((uint32_t)(ubAddressSize - 1) << _QSPI_FLASHCMDCTRL_NUMADDRBYTES_SHIFT);
            QSPI0->FLASHCMDADDR = ulAddress;
        }

        if(ubSrcSize)
        {
            uint32_t pulBuf[2] = { 0, 0 };
            uint8_t *pubBuf = (uint8_t *)pulBuf;

            QSPI0->FLASHCMDCTRL |= QSPI_FLASHCMDCTRL_ENBWRITEDATA | ((uint32_t)(ubSrcSize - 1) << _QSPI_FLASHCMDCTRL_NUMWRDATABYTES_SHIFT);

            for (uint8_t i = 0; i < ubSrcSize; i++)
                pubBuf[i] = pubSrc[i];

            QSPI0->FLASHWRDATALOWER = pulBuf[0];
            QSPI0->FLASHWRDATAUPPER = pulBuf[1];
        }

        if(ubDstSize)
        {
            QSPI0->FLASHCMDCTRL |= QSPI_FLASHCMDCTRL_ENBREADDATA;

            if(ubDstSize > 8)
                QSPI0->FLASHCMDCTRL |= QSPI_FLASHCMDCTRL_STIGMEMBANKEN | ((uint32_t)(ubDstSize - 9) << _QSPI_FLASHCMDCTRL_NUMRDDATABYTES_SHIFT);
            else
                QSPI0->FLASHCMDCTRL |= ((uint32_t)(ubDstSize - 1) << _QSPI_FLASHCMDCTRL_NUMRDDATABYTES_SHIFT);
        }

        QSPI0->FLASHCMDCTRL |= QSPI_FLASHCMDCTRL_CMDEXEC;

        while(QSPI0->FLASHCMDCTRL & QSPI_FLASHCMDCTRL_CMDEXECSTATUS);

        if(ubDstSize)
        {
            uint8_t *pubDstTemp = pubDst;
            uint8_t ubDataLeft = ubDstSize;

            while(ubDstSize > 8 && ubDstSize - ubDataLeft < 8)
            {
                QSPI0->FLASHCOMMANDCTRLMEM = (uint32_t)(ubDstSize - ubDataLeft) << _QSPI_FLASHCOMMANDCTRLMEM_MEMBANKADDR_SHIFT;
                QSPI0->FLASHCOMMANDCTRLMEM |= QSPI_FLASHCOMMANDCTRLMEM_TRIGGERMEMBANKREQ;

                while(QSPI0->FLASHCOMMANDCTRLMEM & QSPI_FLASHCOMMANDCTRLMEM_MEMBANKREQINPROGRESS);

                *pubDstTemp = (QSPI0->FLASHCOMMANDCTRLMEM & _QSPI_FLASHCOMMANDCTRLMEM_MEMBANKREADDATA_MASK) >> _QSPI_FLASHCOMMANDCTRLMEM_MEMBANKREADDATA_SHIFT;

                ubDataLeft--;
                pubDstTemp++;
            }

            uint32_t pulBuf[2] = { 0, 0 };
            uint8_t *pubBuf = (uint8_t *)pulBuf;

            pulBuf[0] = QSPI0->FLASHRDDATALOWER;
            pulBuf[1] = QSPI0->FLASHRDDATAUPPER;

            for (uint8_t i = 0; i < ubDataLeft; i++)
                pubDstTemp[i] = pubBuf[i];
        }

        while(!(QSPI0->CONFIG & QSPI_CONFIG_IDLE));

        QSPI0->CONFIG &= ~QSPI_CONFIG_ENBSPI;
        QSPI0->CONFIG |= QSPI_CONFIG_ENBDIRACCCTLR | QSPI_CONFIG_ENBSPI;
    }
}

void qspi_flash_init()
{
    qspi_flash_power_down(0);
    qspi_flash_reset();

    if(qspi_flash_read_jedec_id() != QSPI_FLASH_JEDEC_ID)
        return;

    qspi_flash_unprotect_all_blocks(); // Unprotect all blocks to be able to read/write
    qspi_flash_write_status_config(qspi_flash_read_status(), qspi_flash_read_config() | BIT(1)); // Enable SIO2 and SIO3
}

uint32_t qspi_flash_read_jedec_id()
{
    uint8_t ubBuf[] = { 0x00, 0x00, 0x00 };

    qspi_flash_cmd(QSPI_FLASH_CMD_JEDEC_READ_ID, 0x00000000, 0, 0, 0, NULL, 0, ubBuf, 3);

    return ((uint32_t)ubBuf[0] << 16) | ((uint32_t)ubBuf[1] << 8) | (uint32_t)ubBuf[2];
}
void qspi_flash_busy_wait()
{
    while(qspi_flash_read_status() & BIT(0))
        delay_ms(1);
}
void qspi_flash_write_enable()
{
    qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_ENABLE, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);

    while(!(qspi_flash_read_status() & BIT(1)))
        delay_ms(1);
}
void qspi_flash_write_disable()
{
    qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_DISABLE, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);

    while(qspi_flash_read_status() & BIT(1))
        delay_ms(1);
}
void qspi_flash_sector_erase(uint32_t ulAddress)
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_SECTOR_ERASE, ulAddress, 3, 0, 0, NULL, 0, NULL, 0);
}
void qspi_flash_chip_erase()
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_CHIP_ERASE, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
}
uint8_t qspi_flash_read_status()
{
    uint8_t ubStatus;

    qspi_flash_cmd(QSPI_FLASH_CMD_READ_STATUS, 0x00000000, 0, 0, 0, NULL, 0, &ubStatus, 1);

    return ubStatus;
}

#if defined(QSPI_FLASH_CMD_READ_CFG)
uint8_t qspi_flash_read_config()
{
    uint8_t ubConfig;

    qspi_flash_busy_wait();

    qspi_flash_cmd(QSPI_FLASH_CMD_READ_CFG, 0x00000000, 0, 0, 0, NULL, 0, &ubConfig, 1);

    return ubConfig;
}
void qspi_flash_write_status_config(uint8_t ubStatus, uint8_t ubConfig)
{
    uint8_t ubBuf[] = { ubStatus, ubConfig };

    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_STATUS, 0x00000000, 0, 0, 0, ubBuf, 2, NULL, 0);
}
#else
void qspi_flash_write_status(uint8_t ubStatus)
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_STATUS, 0x00000000, 0, 0, 0, &ubStatus, 1, NULL, 0);
}
#if defined(QSPI_FLASH_CMD_READ_STATUS_2) && defined(QSPI_FLASH_CMD_WRITE_STATUS_2)
uint8_t qspi_flash_read_status_2()
{
    uint8_t ubStatus;

    qspi_flash_busy_wait();

    qspi_flash_cmd(QSPI_FLASH_CMD_READ_STATUS_2, 0x00000000, 0, 0, 0, NULL, 0, &ubStatus, 1);

    return ubStatus;
}
void qspi_flash_write_status_2(uint8_t ubStatus)
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_STATUS_2, 0x00000000, 0, 0, 0, &ubStatus, 1, NULL, 0);
}
#endif
#if defined(QSPI_FLASH_CMD_READ_STATUS_3) && defined(QSPI_FLASH_CMD_WRITE_STATUS_3)
uint8_t qspi_flash_read_status_3()
{
    uint8_t ubStatus;

    qspi_flash_busy_wait();

    qspi_flash_cmd(QSPI_FLASH_CMD_READ_STATUS_3, 0x00000000, 0, 0, 0, NULL, 0, &ubStatus, 1);

    return ubStatus;
}
void qspi_flash_write_status_3(uint8_t ubStatus)
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_STATUS_3, 0x00000000, 0, 0, 0, &ubStatus, 1, NULL, 0);
}
#endif
#endif


#if defined(QSPI_FLASH_CMD_RESET_ENABLE) && defined(QSPI_FLASH_CMD_RESET)
void qspi_flash_reset()
{
    qspi_flash_cmd(QSPI_FLASH_CMD_RESET_ENABLE, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
    qspi_flash_cmd(QSPI_FLASH_CMD_RESET, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
}
#endif

#if defined(QSPI_FLASH_CMD_SUSPEND) && defined(QSPI_FLASH_CMD_RESUME)
void qspi_flash_suspend()
{
    qspi_flash_cmd(QSPI_FLASH_CMD_SUSPEND, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
}
void qspi_flash_resume()
{
    qspi_flash_cmd(QSPI_FLASH_CMD_RESUME, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
}
#endif

#if defined(QSPI_FLASH_CMD_BLOCK_ERASE)
void qspi_flash_block_erase(uint32_t ulAddress)
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_BLOCK_ERASE, ulAddress, 3, 0, 0, NULL, 0, NULL, 0);
}
#else
#if defined(QSPI_FLASH_CMD_BLOCK_ERASE_32K)
void qspi_flash_32k_block_erase(uint32_t ulAddress)
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_BLOCK_ERASE_32K, ulAddress, 3, 0, 0, NULL, 0, NULL, 0);
}
#endif
#if defined(QSPI_FLASH_CMD_BLOCK_ERASE_64K)
void qspi_flash_64k_block_erase(uint32_t ulAddress)
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_BLOCK_ERASE_64K, ulAddress, 3, 0, 0, NULL, 0, NULL, 0);
}
#endif
#endif

#if defined(QSPI_FLASH_CMD_READ_ID)
uint8_t qspi_flash_read_device_id()
{
    uint8_t ubID;

    qspi_flash_busy_wait();

    qspi_flash_cmd(QSPI_FLASH_CMD_READ_ID, 0x00000000, 0, 0, 24, NULL, 0, &ubID, 1);

    return ubID;
}
#endif

#if defined(QSPI_FLASH_CMD_READ_UID)
uint64_t qspi_flash_read_unique_id()
{
    uint8_t pubBuf[8];

    qspi_flash_busy_wait();

    qspi_flash_cmd(QSPI_FLASH_CMD_READ_UID, 0x00000000, 4, 0, 0, NULL, 0, pubBuf, 8);

    return ((uint64_t)pubBuf[0] << 56) | ((uint64_t)pubBuf[1] << 48) | ((uint64_t)pubBuf[2] << 40) | ((uint64_t)pubBuf[3] << 32) | ((uint64_t)pubBuf[4] << 24) | ((uint64_t)pubBuf[5] << 16) | ((uint64_t)pubBuf[6] << 8) | (uint64_t)pubBuf[7];
}
#endif

#if defined(QSPI_FLASH_CMD_READ_SECURITY) && defined(QSPI_FLASH_CMD_WRITE_SECURITY)
void qspi_flash_read_security(uint16_t usAddress, uint8_t *pubDst, uint16_t usCount)
{
    if(!usCount)
        return;

    if(!pubDst)
        return;

    if(usAddress + usCount > 2048)
        return;

    for(uint16_t i = 0; i < usCount; i += 16)
    {
        uint8_t ubChunkSize = 16;

        if(usCount - i < 16)
            ubChunkSize = usCount - i;

        qspi_flash_busy_wait();

        qspi_flash_cmd(QSPI_FLASH_CMD_READ_SECURITY, usAddress, 2, 0, 8, NULL, 0, pubDst + i, ubChunkSize);
    }
}
void qspi_flash_write_security(uint16_t usAddress, uint8_t *pubSrc, uint16_t usCount)
{
    if(!usCount)
        return;

    if(!pubSrc)
        return;

    if(usAddress + usCount > 2048)
        return;

    for(uint16_t i = 0; i < usCount; i += 8)
    {
        uint8_t ubChunkSize = 8;

        if(usCount - i < 8)
            ubChunkSize = usCount - i;

        qspi_flash_busy_wait();
        qspi_flash_write_enable();

        qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_SECURITY, usAddress, 2, 0, 0, pubSrc + i, ubChunkSize, NULL, 0);
    }
}
#endif

#if defined(QSPI_FLASH_CMD_LOCK_SECURITY)
void qspi_flash_lock_security()
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_LOCK_SECURITY, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
}
#endif

#if defined(QSPI_FLASH_CMD_READ_SECURITY_REG) && defined(QSPI_FLASH_CMD_WRITE_SECURITY_REG)
void qspi_flash_read_security_register(uint8_t ubIndex, uint8_t ubAddress, uint8_t *pubDst, uint16_t usCount)
{
    if(!ubIndex)
        return;

    if(ubIndex > 3)
        return;

    if(!usCount)
        return;

    if(!pubDst)
        return;

    if(ubAddress + usCount > 256)
        return;

    for(uint16_t i = 0; i < usCount; i += 16)
    {
        uint8_t ubChunkSize = 16;

        if(usCount - i < 16)
            ubChunkSize = usCount - i;

        qspi_flash_busy_wait();

        qspi_flash_cmd(QSPI_FLASH_CMD_READ_SECURITY_REG, (ubIndex << 12) | (ubAddress + i), 3, 0, 8, NULL, 0, pubDst + i, ubChunkSize);
    }
}
void qspi_flash_write_security_register(uint8_t ubIndex, uint8_t ubAddress, uint8_t *pubSrc, uint16_t usCount)
{
    if(!ubIndex)
        return;

    if(ubIndex > 3)
        return;

    if(!usCount)
        return;

    if(!pubSrc)
        return;

    if(ubAddress + usCount > 256)
        return;

    for(uint16_t i = 0; i < usCount; i += 8)
    {
        uint8_t ubChunkSize = 8;

        if(usCount - i < 8)
            ubChunkSize = usCount - i;

        qspi_flash_busy_wait();
        qspi_flash_write_enable();

        qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_SECURITY_REG, (ubIndex << 12) | (ubAddress + i), 3, 0, 0, pubSrc + i, ubChunkSize, NULL, 0);
    }
}
#endif

#if defined(QSPI_FLASH_CMD_ERASE_SECURITY_REG)
void qspi_flash_erase_security_register(uint8_t ubIndex)
{
    if(!ubIndex)
        return;

    if(ubIndex > 3)
        return;

    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_ERASE_SECURITY_REG, ubIndex << 12, 3, 0, 0, NULL, 0, NULL, 0);
}
#endif

#if defined(QSPI_FLASH_CMD_POWER_DOWN) && defined(QSPI_FLASH_CMD_RELEASE_POWER_DOWN)
void qspi_flash_power_down(uint8_t ubPower)
{
    if(ubPower)
        qspi_flash_busy_wait();

    qspi_flash_cmd(ubPower ? QSPI_FLASH_CMD_POWER_DOWN : QSPI_FLASH_CMD_RELEASE_POWER_DOWN, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);

    delay_ms(1);
}
#endif

#if defined(QSPI_FLASH_CMD_READ_PROTECTION) && defined(QSPI_FLASH_CMD_WRITE_PROTECTION)
void qspi_flash_protect_blocks(uint64_t ullMask)
{
    uint64_t ullCurrentProtection = qspi_flash_read_protection();

    ullMask &= 0x0000FFFFFFFFFFFF;
    ullCurrentProtection |= ullMask;

    uint8_t pubBuf[6];

    pubBuf[0] = (ullCurrentProtection >> 40) & 0xFF;
    pubBuf[1] = (ullCurrentProtection >> 32) & 0xFF;
    pubBuf[2] = (ullCurrentProtection >> 24) & 0xFF;
    pubBuf[3] = (ullCurrentProtection >> 16) & 0xFF;
    pubBuf[4] = (ullCurrentProtection >> 8) & 0xFF;
    pubBuf[5] = (ullCurrentProtection >> 0) & 0xFF;

    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_PROTECTION, 0x00000000, 0, 0, 0, pubBuf, 6, NULL, 0);
}
void qspi_flash_unprotect_blocks(uint64_t ullMask)
{
    uint64_t ullCurrentProtection = qspi_flash_read_protection();

    ullMask &= 0x0000FFFFFFFFFFFF;
    ullCurrentProtection &= ~ullMask;

    uint8_t pubBuf[6];

    pubBuf[0] = (ullCurrentProtection >> 40) & 0xFF;
    pubBuf[1] = (ullCurrentProtection >> 32) & 0xFF;
    pubBuf[2] = (ullCurrentProtection >> 24) & 0xFF;
    pubBuf[3] = (ullCurrentProtection >> 16) & 0xFF;
    pubBuf[4] = (ullCurrentProtection >> 8) & 0xFF;
    pubBuf[5] = (ullCurrentProtection >> 0) & 0xFF;

    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_PROTECTION, 0x00000000, 0, 0, 0, pubBuf, 6, NULL, 0);
}
uint64_t qspi_flash_read_protection()
{
    uint8_t pubBuf[6];

    qspi_flash_busy_wait();

    qspi_flash_cmd(QSPI_FLASH_CMD_READ_PROTECTION, 0x00000000, 0, 0, 0, NULL, 0, pubBuf, 6);

    return ((uint64_t)pubBuf[0] << 40) | ((uint64_t)pubBuf[1] << 32) | ((uint64_t)pubBuf[2] << 24) | ((uint64_t)pubBuf[3] << 16) | ((uint64_t)pubBuf[4] << 8) | (uint64_t)pubBuf[5];
}
#endif

#if defined(QSPI_FLASH_CMD_LOCK_PROTECTION)
void qspi_flash_lock_protection()
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_LOCK_PROTECTION, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
}
#endif

#if defined(QSPI_FLASH_CMD_nVWRITE_PROTECTION)
void qspi_flash_write_nv_protection(uint64_t ullProtection)
{
    uint8_t pubBuf[6];

    pubBuf[0] = (ullProtection >> 40) & 0xFF;
    pubBuf[1] = (ullProtection >> 32) & 0xFF;
    pubBuf[2] = (ullProtection >> 24) & 0xFF;
    pubBuf[3] = (ullProtection >> 16) & 0xFF;
    pubBuf[4] = (ullProtection >> 8) & 0xFF;
    pubBuf[5] = (ullProtection >> 0) & 0xFF;

    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_nVWRITE_PROTECTION, 0x00000000, 0, 0, 0, pubBuf, 6, NULL, 0);
}
#endif

#if defined(QSPI_FLASH_CMD_GLOBAL_UNPROTECT)
void qspi_flash_unprotect_all_blocks()
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_GLOBAL_UNPROTECT, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
}
#endif

#if defined(QSPI_FLASH_CMD_GLOBAL_PROTECT)
void qspi_flash_protect_all_blocks()
{
    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_GLOBAL_PROTECT, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
}
#endif

#if defined(QSPI_FLASH_CMD_QUERY_PROTECT)
uint8_t qspi_flash_is_block_protected(uint32_t ulAddress)
{
    ulAddress &= QSPI_FLASH_MAX_ADDRESS;

    uint8_t ubProtected = 0x00;

    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_QUERY_PROTECT, ulAddress, 3, 0, 0, NULL, 0, &ubProtected, 1);

    return ubProtected;
}
#endif

#if defined(QSPI_FLASH_CMD_UNPROTECT)
void qspi_flash_unprotect_block(uint32_t ulAddress)
{
    ulAddress &= QSPI_FLASH_MAX_ADDRESS;

    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_UNPROTECT, ulAddress, 3, 0, 0, NULL, 0, NULL, 0);
}
#endif

#if defined(QSPI_FLASH_CMD_PROTECT)
void qspi_flash_protect_block(uint32_t ulAddress)
{
    ulAddress &= QSPI_FLASH_MAX_ADDRESS;

    qspi_flash_busy_wait();
    qspi_flash_write_enable();

    qspi_flash_cmd(QSPI_FLASH_CMD_PROTECT, ulAddress, 3, 0, 0, NULL, 0, NULL, 0);
}
#endif