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
    QSPI0->DEVINSTRRDCONFIG &= _QSPI_DEVINSTRRDCONFIG_INSTRTYPE_MASK;

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

        if (ubSrcSize)
        {
            uint32_t pulBuf[2] = { 0, 0 };
            uint8_t *pubBuf = (uint8_t *)pulBuf;

            QSPI0->FLASHCMDCTRL |= QSPI_FLASHCMDCTRL_ENBWRITEDATA | ((uint32_t)(ubSrcSize - 1) << _QSPI_FLASHCMDCTRL_NUMWRDATABYTES_SHIFT);

            for (uint8_t i = 0; i < ubSrcSize; i++)
                pubBuf[i] = pubSrc[i];

            QSPI0->FLASHWRDATALOWER = pulBuf[0];
            QSPI0->FLASHWRDATAUPPER = pulBuf[1];
        }

        if (ubDstSize)
        {
            QSPI0->FLASHCMDCTRL |= QSPI_FLASHCMDCTRL_ENBREADDATA;

            if(ubDstSize > 8)
                QSPI0->FLASHCMDCTRL |= QSPI_FLASHCMDCTRL_STIGMEMBANKEN | ((uint32_t)(ubDstSize - 9) << _QSPI_FLASHCMDCTRL_NUMRDDATABYTES_SHIFT);
            else
                QSPI0->FLASHCMDCTRL |= ((uint32_t)(ubDstSize - 1) << _QSPI_FLASHCMDCTRL_NUMRDDATABYTES_SHIFT);
        }

        QSPI0->FLASHCMDCTRL |= QSPI_FLASHCMDCTRL_CMDEXEC;

        while(QSPI0->FLASHCMDCTRL & QSPI_FLASHCMDCTRL_CMDEXECSTATUS);

        if (ubDstSize)
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
    qspi_flash_reset();

    if(qspi_flash_read_jedec_id() != 0xBF2643)
        return;

    qspi_flash_unprotect_all_blocks(); // Unprotect all blocks to be able to read/write
    qspi_flash_write_status_config(qspi_flash_read_status(), qspi_flash_read_config() | BIT(1)); // Enable SIO2 and SIO3
}
void qspi_flash_reset()
{
    qspi_flash_cmd(QSPI_FLASH_CMD_RESET_ENABLE, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
    qspi_flash_cmd(QSPI_FLASH_CMD_RESET, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
}
uint8_t qspi_flash_read_status()
{
    uint8_t ubStatus;

    qspi_flash_cmd(QSPI_FLASH_CMD_READ_STATUS, 0x00000000, 0, 0, 0, NULL, 0, &ubStatus, 1);

    return ubStatus;
}
uint8_t qspi_flash_read_config()
{
    uint8_t ubConfig;

    qspi_flash_cmd(QSPI_FLASH_CMD_READ_CFG, 0x00000000, 0, 0, 0, NULL, 0, &ubConfig, 1);

    return ubConfig;
}
void qspi_flash_write_status_config(uint8_t ubStatus, uint8_t ubConfig)
{
    uint8_t ubBuf[] = { ubStatus, ubConfig };

    qspi_flash_write_enable();
    qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_STATUS, 0x00000000, 0, 0, 0, ubBuf, 2, NULL, 0);
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
void qspi_flash_block_erase(uint32_t ulAddress)
{
    qspi_flash_write_enable();
    qspi_flash_cmd(QSPI_FLASH_CMD_BLOCK_ERASE, ulAddress, 3, 0, 0, NULL, 0, NULL, 0);

    qspi_flash_busy_wait();
}
void qspi_flash_sector_erase(uint32_t ulAddress)
{
    qspi_flash_write_enable();
    qspi_flash_cmd(QSPI_FLASH_CMD_SECTOR_ERASE, ulAddress, 3, 0, 0, NULL, 0, NULL, 0);

    qspi_flash_busy_wait();
}
void qspi_flash_chip_erase()
{
    qspi_flash_write_enable();
    qspi_flash_cmd(QSPI_FLASH_CMD_CHIP_ERASE, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);

    qspi_flash_busy_wait();
}
uint32_t qspi_flash_read_jedec_id()
{
	uint8_t ubBuf[] = { 0x00, 0x00, 0x00 };

    qspi_flash_cmd(QSPI_FLASH_CMD_JEDEC_READ_ID, 0x00000000, 0, 0, 0, NULL, 0, ubBuf, 3);

	return ((uint32_t)ubBuf[0] << 16) | ((uint32_t)ubBuf[1] << 8) | (uint32_t)ubBuf[2];
}
void qspi_flash_read_security(uint16_t usAddress, uint8_t *pubDst, uint8_t ubCount)
{
    qspi_flash_cmd(QSPI_FLASH_CMD_READ_SECURITY, usAddress, 2, 0, 8, NULL, 0, pubDst, ubCount);
}
void qspi_flash_write_security(uint16_t usAddress, uint8_t *pubSrc, uint8_t ubCount)
{
    qspi_flash_write_enable();
    qspi_flash_cmd(QSPI_FLASH_CMD_WRITE_SECURITY, usAddress, 2, 0, 0, pubSrc, ubCount, NULL, 0);
}
void qspi_flash_unprotect_all_blocks()
{
	qspi_flash_write_enable();
    qspi_flash_cmd(QSPI_FLASH_CMD_UNLOCK_PROTECTION, 0x00000000, 0, 0, 0, NULL, 0, NULL, 0);
}