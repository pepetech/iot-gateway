#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#define VAL2FIELD(fieldname, value) (((value) & fieldname##_M) << fieldname##_S)

// REG PTR
#define PREG(addr) ((volatile uint32_t *)(addr))
#define REG(addr) (*PREG(addr))

// DPORT
#define REG_DPORT_BASE(i)   (0x3FF00000 + (i) * 4)

#define DPORT_DPORT0                REG_DPORT_BASE(0)
#define DPORT_DPORT0_FIELD0_M           0x0000001F
#define DPORT_DPORT0_FIELD0_S           0

#define DPORT_INT_ENABLE            REG_DPORT_BASE(1)
#define DPORT_INT_ENABLE_WDT            BIT(0)
#define DPORT_INT_ENABLE_TIMER0         BIT(1)
#define DPORT_INT_ENABLE_TIMER1         BIT(2)

#define DPORT_SPI_READY             REG_DPORT_BASE(3)
#define DPORT_SPI_READY_CACHE_SIZE_32K  BIT(26) // 0 = 16K, 1 = 32K
#define DPORT_SPI_READY_MMAP_MB_LOW_M   0x00000001
#define DPORT_SPI_READY_MMAP_MB_LOW_S   25
#define DPORT_SPI_READY_UNK24           BIT(24)
#define DPORT_SPI_READY_MMAP_MB_HIGH_M  0x0000000E
#define DPORT_SPI_READY_MMAP_MB_HIGH_S  15
#define DPORT_SPI_READY_READY_IDLE      BIT(9)
#define DPORT_SPI_READY_CACHE_EMPTY     BIT(1)
#define DPORT_SPI_READY_CACHE_FLUSH     BIT(0)

// WARNING: Macro evaluates argument twice
#define DPORT_SPI_READY_MMAP_MB(val) (VAL2FIELD(DPORT_SPI_READY_MMAP_MB_LOW, val) | VAL2FIELD(DPORT_SPI_READY_MMAP_MB_HIGH, val))

#define DPORT_CPU_CLOCK             REG_DPORT_BASE(5)
#define DPORT_CPU_CLOCK_X2              BIT(0)

#define DPORT_CLOCKGATE_WATCHDOG    REG_DPORT_BASE(6)
#define DPORT_CLOCKGATE_WATCHDOG_UNKNOWN_8  BIT(8)
#define DPORT_CLOCKGATE_WATCHDOG_DISABLE    BIT(3)

#define DPORT_SPI_INT_STATUS        REG_DPORT_BASE(8)
#define DPORT_SPI_INT_STATUS_SPI0       BIT(4)
#define DPORT_SPI_INT_STATUS_SPI1       BIT(7)
#define DPORT_SPI_INT_STATUS_I2S        BIT(9)

#define DPORT_SPI_CACHE_RAM         REG_DPORT_BASE(9)
#define DPORT_SPI_CACHE_RAM_BANK1       BIT(3) // 0x4010C000
#define DPORT_SPI_CACHE_RAM_BANK0       BIT(4) // 0x40108000

#define DPORT_PERI_IO               REG_DPORT_BASE(10)
#define DPORT_PERI_IO_SWAP_UARTS        BIT(0)
#define DPORT_PERI_IO_SWAP_SPIS         BIT(1)
#define DPORT_PERI_IO_SWAP_UART0_PINS   BIT(2)
#define DPORT_PERI_IO_SWAP_UART1_PINS   BIT(3)
#define DPORT_PERI_IO_SPI1_PRIORITY     BIT(5)
#define DPORT_PERI_IO_SPI1_SHARED       BIT(6)
#define DPORT_PERI_IO_SPI0_SHARED       BIT(7)

#define DPORT_SLC_TX_DESC_DEBUG     REG_DPORT_BASE(11)
#define DPORT_SLC_TX_DESC_DEBUG_VALUE_M      0x0000FFFF
#define DPORT_SLC_TX_DESC_DEBUG_VALUE_S      0
#define DPORT_SLC_TX_DESC_DEBUG_VALUE_MAGIC  0xCCCC

#define DPORT_OTP_MAC0              REG_DPORT_BASE(20)
#define DPORT_OTP_MAC1              REG_DPORT_BASE(21)
#define DPORT_OTP_CHIPID            REG_DPORT_BASE(22)
#define DPORT_OTP_MAC2              REG_DPORT_BASE(23)

// WDEV
#define REG_WDEV_BASE(i)   (0x3FF20000 + (i) * 4)

#define WDEV_UART_INT_STATUS        REG_WDEV_BASE(8)
#define WDEV_UART_INT_STATUS_UART0      BIT(0)
#define WDEV_UART_INT_STATUS_UART1      BIT(2)

#define WDEV_SYS_TIME               REG_WDEV_BASE(768)

#define WDEV_BT_COEXIST             REG_WDEV_BASE(888)

#define WDEV_HW_RAND                REG_WDEV_BASE(913)

// MMIO
#define REG_MMIO_BASE       0x60000000

#define REG_UART0_BASE(i)   (REG_MMIO_BASE + 0x0000 + (i) * 4) // UART
#define REG_SPI1_BASE(i)    (REG_MMIO_BASE + 0x0100 + (i) * 4) // SPI
#define REG_SPI0_BASE(i)    (REG_MMIO_BASE + 0x0200 + (i) * 4) // SPI
#define REG_GPIO_BASE(i)    (REG_MMIO_BASE + 0x0300 + (i) * 4) // GPIO
//#define REG_UNK_BASE(i)       (REG_MMIO_BASE + 0x0400 + (i) * 4)
#define REG_PHY_BASE(i)     (REG_MMIO_BASE + 0x0500 + (i) * 4)
#define REG_TIMER_BASE(i)   (REG_MMIO_BASE + 0x0600 + (i) * 4)
#define REG_RTC_BASE(i)     (REG_MMIO_BASE + 0x0700 + (i) * 4) // RTC
#define REG_IOMUX_BASE(i)   (REG_MMIO_BASE + 0x0800 + (i) * 4) // IOMUX
#define REG_WDT_BASE(i)     (REG_MMIO_BASE + 0x0900 + (i) * 4)
#define REG_SDIO_BASE(i)    (REG_MMIO_BASE + 0x0A00 + (i) * 4)
#define REG_SLC_BASE(i)     (REG_MMIO_BASE + 0x0B00 + (i) * 4)
//#define REG_UNK_BASE(i)     (REG_MMIO_BASE + 0x0C00 + (i) * 4)
#define REG_SAR_BASE(i)     (REG_MMIO_BASE + 0x0D00 + (i) * 4)
#define REG_I2S_BASE(i)     (REG_MMIO_BASE + 0x0E00 + (i) * 4)
#define REG_UART1_BASE(i)   (REG_MMIO_BASE + 0x0F00 + (i) * 4) // UART
#define REG_RTCB_BASE(i)    (REG_MMIO_BASE + 0x1000 + (i) * 4) // RTC BACKUP MEM
#define REG_RTCS_BASE(i)    (REG_MMIO_BASE + 0x1100 + (i) * 4) // RTC SYSTEM MEM
#define REG_RTCU_BASE(i)    (REG_MMIO_BASE + 0x1200 + (i) * 4) // RTC USER MEM

// UART
#define UART0_FIFO                       REG_UART0_BASE(0)
#define UART1_FIFO                       REG_UART1_BASE(0)
#define UART_RXFIFO_RD_BYTE_M                0x000000FF
#define UART_RXFIFO_RD_BYTE_S                0

#define UART0_INT_RAW                    REG_UART0_BASE(1)
#define UART1_INT_RAW                    REG_UART1_BASE(1)
#define UART_RXFIFO_TOUT_INT_RAW             BIT(8)
#define UART_BRK_DET_INT_RAW                 BIT(7)
#define UART_CTS_CHG_INT_RAW                 BIT(6)
#define UART_DSR_CHG_INT_RAW                 BIT(5)
#define UART_RXFIFO_OVF_INT_RAW              BIT(4)
#define UART_FRM_ERR_INT_RAW                 BIT(3)
#define UART_PARITY_ERR_INT_RAW              BIT(2)
#define UART_TXFIFO_EMPTY_INT_RAW            BIT(1)
#define UART_RXFIFO_FULL_INT_RAW             BIT(0)

#define UART0_INT_ST                     REG_UART0_BASE(2)
#define UART1_INT_ST                     REG_UART1_BASE(2)
#define UART_RXFIFO_TOUT_INT_ST              BIT(8)
#define UART_BRK_DET_INT_ST                  BIT(7)
#define UART_CTS_CHG_INT_ST                  BIT(6)
#define UART_DSR_CHG_INT_ST                  BIT(5)
#define UART_RXFIFO_OVF_INT_ST               BIT(4)
#define UART_FRM_ERR_INT_ST                  BIT(3)
#define UART_PARITY_ERR_INT_ST               BIT(2)
#define UART_TXFIFO_EMPTY_INT_ST             BIT(1)
#define UART_RXFIFO_FULL_INT_ST              BIT(0)

#define UART0_INT_ENA                    REG_UART0_BASE(3)
#define UART1_INT_ENA                    REG_UART1_BASE(3)
#define UART_RXFIFO_TOUT_INT_ENA             BIT(8)
#define UART_BRK_DET_INT_ENA                 BIT(7)
#define UART_CTS_CHG_INT_ENA                 BIT(6)
#define UART_DSR_CHG_INT_ENA                 BIT(5)
#define UART_RXFIFO_OVF_INT_ENA              BIT(4)
#define UART_FRM_ERR_INT_ENA                 BIT(3)
#define UART_PARITY_ERR_INT_ENA              BIT(2)
#define UART_TXFIFO_EMPTY_INT_ENA            BIT(1)
#define UART_RXFIFO_FULL_INT_ENA             BIT(0)

#define UART0_INT_CLR                    REG_UART0_BASE(4)
#define UART1_INT_CLR                    REG_UART1_BASE(4)
#define UART_RXFIFO_TOUT_INT_CLR             BIT(8)
#define UART_BRK_DET_INT_CLR                 BIT(7)
#define UART_CTS_CHG_INT_CLR                 BIT(6)
#define UART_DSR_CHG_INT_CLR                 BIT(5)
#define UART_RXFIFO_OVF_INT_CLR              BIT(4)
#define UART_FRM_ERR_INT_CLR                 BIT(3)
#define UART_PARITY_ERR_INT_CLR              BIT(2)
#define UART_TXFIFO_EMPTY_INT_CLR            BIT(1)
#define UART_RXFIFO_FULL_INT_CLR             BIT(0)

#define UART0_CLKDIV                     REG_UART0_BASE(5)
#define UART1_CLKDIV                     REG_UART1_BASE(5)
#define UART_CLKDIV_M                        0x000FFFFF
#define UART_CLKDIV_S                        0

#define UART0_AUTOBAUD                   REG_UART0_BASE(6)
#define UART1_AUTOBAUD                   REG_UART1_BASE(6)
#define UART_GLITCH_FILT_M                   0x000000FF
#define UART_GLITCH_FILT_S                   8
#define UART_AUTOBAUD_EN                     BIT(0)

#define UART0_STATUS                     REG_UART0_BASE(7)
#define UART1_STATUS                     REG_UART1_BASE(7)
#define UART_TXD                             BIT(31)
#define UART_RTSN                            BIT(30)
#define UART_DTRN                            BIT(29)
#define UART_TXFIFO_CNT_M                    0x000000FF
#define UART_TXFIFO_CNT_S                    16
#define UART_RXD                             BIT(15)
#define UART_CTSN                            BIT(14)
#define UART_DSRN                            BIT(13)
#define UART_RXFIFO_CNT_M                    0x000000FF
#define UART_RXFIFO_CNT_S                    0

#define UART0_CONF0                      REG_UART0_BASE(8)
#define UART1_CONF0                      REG_UART1_BASE(8)
#define UART_DTR_INV                         BIT(24)
#define UART_RTS_INV                         BIT(23)
#define UART_TXD_INV                         BIT(22)
#define UART_DSR_INV                         BIT(21)
#define UART_CTS_INV                         BIT(20)
#define UART_RXD_INV                         BIT(19)
#define UART_TXFIFO_RST                      BIT(18)
#define UART_RXFIFO_RST                      BIT(17)
#define UART_IRDA_EN                         BIT(16)
#define UART_TX_FLOW_EN                      BIT(15)
#define UART_LOOPBACK                        BIT(14)
#define UART_IRDA_RX_INV                     BIT(13)
#define UART_IRDA_TX_INV                     BIT(12)
#define UART_IRDA_WCTL                       BIT(11)
#define UART_IRDA_TX_EN                      BIT(10)
#define UART_IRDA_DPLX                       BIT(9)
#define UART_TXD_BRK                         BIT(8)
#define UART_SW_DTR                          BIT(7)
#define UART_SW_RTS                          BIT(6)
#define UART_STOP_BIT_NUM_M                  0x00000003
#define UART_STOP_BIT_NUM_S                  4
#define UART_BIT_NUM_M                       0x00000003
#define UART_BIT_NUM_S                       2
#define UART_PARITY_EN                       BIT(1)
#define UART_PARITY                          BIT(0) // 1 = ODD, 0 = EVEN

#define UART0_CONF1                      REG_UART0_BASE(9)
#define UART1_CONF1                      REG_UART1_BASE(9)
#define UART_RX_TOUT_EN                      BIT(31)
#define UART_RX_TOUT_THRHD_M                 0x0000007F
#define UART_RX_TOUT_THRHD_S                 24
#define UART_RX_FLOW_EN                      BIT(23)
#define UART_RX_FLOW_THRHD_M                 0x0000007F
#define UART_RX_FLOW_THRHD_S                 16
#define UART_TXFIFO_EMPTY_THRHD_M            0x0000007F
#define UART_TXFIFO_EMPTY_THRHD_S            8
#define UART_RXFIFO_FULL_THRHD_M             0x0000007F
#define UART_RXFIFO_FULL_THRHD_S             0

#define UART0_LOWPULSE                   REG_UART0_BASE(10)
#define UART1_LOWPULSE                   REG_UART1_BASE(10)
#define UART_LOWPULSE_MIN_CNT_M              0x000FFFFF
#define UART_LOWPULSE_MIN_CNT_S              0

#define UART0_HIGHPULSE                  REG_UART0_BASE(11)
#define UART1_HIGHPULSE                  REG_UART1_BASE(11)
#define UART_HIGHPULSE_MIN_CNT_M             0x000FFFFF
#define UART_HIGHPULSE_MIN_CNT_S             0

#define UART0_PULSE_NUM                  REG_UART0_BASE(12)
#define UART1_PULSE_NUM                  REG_UART1_BASE(12)
#define UART_PULSE_NUM_CNT_M                 0x0003FF
#define UART_PULSE_NUM_CNT_S                 0

#define UART0_DATE                       REG_UART0_BASE(30)
#define UART1_DATE                       REG_UART1_BASE(30)

#define UART0_ID                         REG_UART0_BASE(31)
#define UART1_ID                         REG_UART1_BASE(31)

// SPI
#define SPI0_CMD                         REG_SPI0_BASE(0)
#define SPI1_CMD                         REG_SPI1_BASE(0)
#define SPI_CMD_READ                         BIT(31)
#define SPI_CMD_WRITE_ENABLE                 BIT(30)
#define SPI_CMD_WRITE_DISABLE                BIT(29)
#define SPI_CMD_READ_ID                      BIT(28)
#define SPI_CMD_READ_SR                      BIT(27)
#define SPI_CMD_WRITE_SR                     BIT(26)
#define SPI_CMD_PP                           BIT(25)
#define SPI_CMD_SE                           BIT(24)
#define SPI_CMD_BE                           BIT(23)
#define SPI_CMD_CE                           BIT(22)
#define SPI_CMD_DP                           BIT(21)
#define SPI_CMD_RES                          BIT(20)
#define SPI_CMD_HPM                          BIT(19)
#define SPI_CMD_USR                          BIT(18)

#define SPI0_ADDR                        REG_SPI0_BASE(1)
#define SPI1_ADDR                        REG_SPI1_BASE(1)

#define SPI0_CTRL0                       REG_SPI0_BASE(2)
#define SPI1_CTRL0                       REG_SPI1_BASE(2)
#define SPI_CTRL0_WR_BIT_ORDER               BIT(26)
#define SPI_CTRL0_RD_BIT_ORDER               BIT(25)
#define SPI_CTRL0_QIO_MODE                   BIT(24)
#define SPI_CTRL0_DIO_MODE                   BIT(23)
#define SPI_CTRL0_QOUT_MODE                  BIT(20)
#define SPI_CTRL0_DOUT_MODE                  BIT(14)
#define SPI_CTRL0_FASTRD_MODE                BIT(13)
#define SPI_CTRL0_CLOCK_EQU_SYS_CLOCK        BIT(12)
#define SPI_CTRL0_CLOCK_NUM_M                0x0000000F
#define SPI_CTRL0_CLOCK_NUM_S                8
#define SPI_CTRL0_CLOCK_HIGH_M               0x0000000F
#define SPI_CTRL0_CLOCK_HIGH_S               4
#define SPI_CTRL0_CLOCK_LOW_M                0x0000000F
#define SPI_CTRL0_CLOCK_LOW_S                0
#define SPI_CTRL0_CLOCK_M                    0x00000FFF
#define SPI_CTRL0_CLOCK_S                    0

#define SPI0_CTRL1                       REG_SPI0_BASE(3)
#define SPI1_CTRL1                       REG_SPI1_BASE(3)
#define SPI_CTRL1_CS_HOLD_DELAY_M           0x0000000F
#define SPI_CTRL1_CS_HOLD_DELAY_S           28
#define SPI_CTRL1_CS_HOLD_DELAY_RES_M       0x00000FFF
#define SPI_CTRL1_CS_HOLD_DELAY_RES_S       16

#define SPI0_RSTATUS                     REG_SPI0_BASE(4)
#define SPI1_RSTATUS                     REG_SPI1_BASE(4)

#define SPI0_CTRL2                       REG_SPI0_BASE(5)
#define SPI1_CTRL2                       REG_SPI1_BASE(5)
#define SPI_CTRL2_CS_DELAY_NUM_M             0x0000000F
#define SPI_CTRL2_CS_DELAY_NUM_S             28
#define SPI_CTRL2_CS_DELAY_MODE_M            0x00000003
#define SPI_CTRL2_CS_DELAY_MODE_S            26
#define SPI_CTRL2_MOSI_DELAY_NUM_M           0x00000007
#define SPI_CTRL2_MOSI_DELAY_NUM_S           23
#define SPI_CTRL2_MOSI_DELAY_MODE_M          0x00000003
#define SPI_CTRL2_MOSI_DELAY_MODE_S          21
#define SPI_CTRL2_MISO_DELAY_NUM_M           0x00000007
#define SPI_CTRL2_MISO_DELAY_NUM_S           18
#define SPI_CTRL2_MISO_DELAY_MODE_M          0x00000003
#define SPI_CTRL2_MISO_DELAY_MODE_S          16

#define SPI0_CLOCK                       REG_SPI0_BASE(6)
#define SPI1_CLOCK                       REG_SPI1_BASE(6)
#define SPI_CLOCK_EQU_SYS_CLOCK              BIT(31)
#define SPI_CLOCK_DIV_PRE_M                  0x00001FFF
#define SPI_CLOCK_DIV_PRE_S                  18
#define SPI_CLOCK_COUNT_NUM_M                0x0000003F
#define SPI_CLOCK_COUNT_NUM_S                12
#define SPI_CLOCK_COUNT_HIGH_M               0x0000003F
#define SPI_CLOCK_COUNT_HIGH_S               6
#define SPI_CLOCK_COUNT_LOW_M                0x0000003F
#define SPI_CLOCK_COUNT_LOW_S                0
#define SPI_CLOCK_COUNT_M                    0x0003FFFF
#define SPI_CLOCK_COUNT_S                    0

#define SPI0_USER0                       REG_SPI0_BASE(7)
#define SPI1_USER0                       REG_SPI1_BASE(7)
#define SPI_USER0_COMMAND                    BIT(31)
#define SPI_USER0_ADDR                       BIT(30)
#define SPI_USER0_DUMMY                      BIT(29)
#define SPI_USER0_MISO                       BIT(28)
#define SPI_USER0_MOSI                       BIT(27)
#define SPI_USER0_MOSI_HIGHPART              BIT(25)
#define SPI_USER0_MISO_HIGHPART              BIT(24)
#define SPI_USER0_SIO                        BIT(16)
#define SPI_USER0_FWRITE_QIO                 BIT(15)
#define SPI_USER0_FWRITE_DIO                 BIT(14)
#define SPI_USER0_FWRITE_QUAD                BIT(13)
#define SPI_USER0_FWRITE_DUAL                BIT(12)
#define SPI_USER0_WR_BYTE_ORDER              BIT(11)
#define SPI_USER0_RD_BYTE_ORDER              BIT(10)
#define SPI_USER0_CLOCK_OUT_EDGE             BIT(7)
#define SPI_USER0_CLOCK_IN_EDGE              BIT(6)
#define SPI_USER0_CS_SETUP                   BIT(5)
#define SPI_USER0_CS_HOLD                    BIT(4)
#define SPI_USER0_FLASH_MODE                 BIT(2)
#define SPI_USER0_DUPLEX                     BIT(0)

#define SPI0_USER1                       REG_SPI0_BASE(8)
#define SPI1_USER1                       REG_SPI1_BASE(8)
#define SPI_USER1_ADDR_BITLEN_M              0x0000003F
#define SPI_USER1_ADDR_BITLEN_S              26
#define SPI_USER1_MOSI_BITLEN_M              0x000001FF
#define SPI_USER1_MOSI_BITLEN_S              17
#define SPI_USER1_MISO_BITLEN_M              0x000001FF
#define SPI_USER1_MISO_BITLEN_S              8
#define SPI_USER1_DUMMY_CYCLELEN_M           0x000000FF
#define SPI_USER1_DUMMY_CYCLELEN_S           0

#define SPI0_USER2                       REG_SPI0_BASE(9)
#define SPI1_USER2                       REG_SPI1_BASE(9)
#define SPI_USER2_COMMAND_BITLEN_M           0x0000000F
#define SPI_USER2_COMMAND_BITLEN_S           28
#define SPI_USER2_COMMAND_VALUE_M            0x0000FFFF
#define SPI_USER2_COMMAND_VALUE_S            0

#define SPI0_WSTATUS                     REG_SPI0_BASE(10)
#define SPI1_WSTATUS                     REG_SPI1_BASE(10)

#define SPI0_PIN                         REG_SPI0_BASE(11)
#define SPI1_PIN                         REG_SPI1_BASE(11)
#define SPI_PIN_IDLE_EDGE                    BIT(29)  ///< CPOL
#define SPI_PIN_CS2_DISABLE                  BIT(2)
#define SPI_PIN_CS1_DISABLE                  BIT(1)
#define SPI_PIN_CS0_DISABLE                  BIT(0)

#define SPI0_SLAVE0                      REG_SPI0_BASE(12)
#define SPI1_SLAVE0                      REG_SPI1_BASE(12)
#define SPI_SLAVE0_SYNC_RESET                BIT(31)
#define SPI_SLAVE0_MODE                      BIT(30)
#define SPI_SLAVE0_WR_RD_BUF_EN              BIT(29)
#define SPI_SLAVE0_WR_RD_STA_EN              BIT(28)
#define SPI_SLAVE0_CMD_DEFINE                BIT(27)
#define SPI_SLAVE0_TRANS_COUNT_M             0x0000000F
#define SPI_SLAVE0_TRANS_COUNT_S             23
#define SPI_SLAVE0_TRANS_DONE_EN             BIT(9)
#define SPI_SLAVE0_WR_STA_DONE_EN            BIT(8)
#define SPI_SLAVE0_RD_STA_DONE_EN            BIT(7)
#define SPI_SLAVE0_WR_BUF_DONE_EN            BIT(6)
#define SPI_SLAVE0_RD_BUF_DONE_EN            BIT(5)
#define SPI_SLAVE0_TRANS_DONE                BIT(4)
#define SPI_SLAVE0_WR_STA_DONE               BIT(3)
#define SPI_SLAVE0_RD_STA_DONE               BIT(2)
#define SPI_SLAVE0_WR_BUF_DONE               BIT(1)
#define SPI_SLAVE0_RD_BUF_DONE               BIT(0)

#define SPI0_SLAVE1                      REG_SPI0_BASE(13)
#define SPI1_SLAVE1                      REG_SPI1_BASE(13)
#define SPI_SLAVE1_STATUS_BITLEN_M           0x0000001F
#define SPI_SLAVE1_STATUS_BITLEN_S           27
#define SPI_SLAVE1_BUF_BITLEN_M              0x000001FF
#define SPI_SLAVE1_BUF_BITLEN_S              16
#define SPI_SLAVE1_RD_ADDR_BITLEN_M          0x0000003F
#define SPI_SLAVE1_RD_ADDR_BITLEN_S          10
#define SPI_SLAVE1_WR_ADDR_BITLEN_M          0x0000003F
#define SPI_SLAVE1_WR_ADDR_BITLEN_S          4
#define SPI_SLAVE1_WRSTA_DUMMY_ENABLE        BIT(3)
#define SPI_SLAVE1_RDSTA_DUMMY_ENABLE        BIT(2)
#define SPI_SLAVE1_WRBUF_DUMMY_ENABLE        BIT(1)
#define SPI_SLAVE1_RDBUF_DUMMY_ENABLE        BIT(0)

#define SPI0_SLAVE2                      REG_SPI0_BASE(14)
#define SPI1_SLAVE2                      REG_SPI1_BASE(14)
#define SPI_SLAVE2_WRBUF_DUMMY_CYCLELEN_M    0x000000FF
#define SPI_SLAVE2_WRBUF_DUMMY_CYCLELEN_S    24
#define SPI_SLAVE2_RDBUF_DUMMY_CYCLELEN_M    0x000000FF
#define SPI_SLAVE2_RDBUF_DUMMY_CYCLELEN_S    16
#define SPI_SLAVE2_WRSTR_DUMMY_CYCLELEN_M    0x000000FF
#define SPI_SLAVE2_WRSTR_DUMMY_CYCLELEN_S    8
#define SPI_SLAVE2_RDSTR_DUMMY_CYCLELEN_M    0x000000FF
#define SPI_SLAVE2_RDSTR_DUMMY_CYCLELEN_S    0

#define SPI0_SLAVE3                      REG_SPI0_BASE(15)
#define SPI1_SLAVE3                      REG_SPI1_BASE(15)
#define SPI_SLAVE3_WRSTA_CMD_VALUE_M         0x000000FF
#define SPI_SLAVE3_WRSTA_CMD_VALUE_S         24
#define SPI_SLAVE3_RDSTA_CMD_VALUE_M         0x000000FF
#define SPI_SLAVE3_RDSTA_CMD_VALUE_S         16
#define SPI_SLAVE3_WRBUF_CMD_VALUE_M         0x000000FF
#define SPI_SLAVE3_WRBUF_CMD_VALUE_S         8
#define SPI_SLAVE3_RDBUF_CMD_VALUE_M         0x000000FF
#define SPI_SLAVE3_RDBUF_CMD_VALUE_S         0

#define SPI0_W(i)                        REG_SPI0_BASE(16 + (i))
#define SPI1_W(i)                        REG_SPI1_BASE(16 + (i))

#define SPI0_EXT0                        REG_SPI0_BASE(60)
#define SPI1_EXT0                        REG_SPI1_BASE(60)

#define SPI0_EXT1                        REG_SPI0_BASE(61)
#define SPI1_EXT1                        REG_SPI1_BASE(61)

#define SPI0_EXT2                        REG_SPI0_BASE(62)
#define SPI1_EXT2                        REG_SPI1_BASE(62)

#define SPI0_EXT3                        REG_SPI0_BASE(63)
#define SPI1_EXT3                        REG_SPI1_BASE(63)
#define SPI_EXT3_INT_HOLD_ENABLE_M           0x00000003
#define SPI_EXT3_INT_HOLD_ENABLE_S           0

// GPIO
#define GPIO_OUT                    REG_GPIO_BASE(0)
#define GPIO_OUT_SET                REG_GPIO_BASE(1)
#define GPIO_OUT_CLEAR              REG_GPIO_BASE(2)
#define GPIO_ENABLE_OUT             REG_GPIO_BASE(3)
#define GPIO_ENABLE_OUT_SET         REG_GPIO_BASE(4)
#define GPIO_ENABLE_OUT_CLEAR       REG_GPIO_BASE(5)
#define GPIO_IN                     REG_GPIO_BASE(6)
#define GPIO_STATUS                 REG_GPIO_BASE(7)
#define GPIO_STATUS_SET             REG_GPIO_BASE(8)
#define GPIO_STATUS_CLEAR           REG_GPIO_BASE(9)

#define GPIO_CONF_INTTYPE(val) VAL2FIELD(GPIO_CONF_INTTYPE, val)

#define GPIO_CONF(i)                REG_GPIO_BASE(10 + (i))
#define GPIO_CONF_CONFIG_M              0x00000003
#define GPIO_CONF_CONFIG_S              11
#define GPIO_CONF_WAKEUP_ENABLE         BIT(10)
#define GPIO_CONF_INTTYPE_M             0x00000007
#define GPIO_CONF_INTTYPE_S             7
#define GPIO_CONF_INTTYPE_NONE          GPIO_CONF_INTTYPE(0)
#define GPIO_CONF_INTTYPE_EDGE_POS      GPIO_CONF_INTTYPE(1)
#define GPIO_CONF_INTTYPE_EDGE_NEG      GPIO_CONF_INTTYPE(2)
#define GPIO_CONF_INTTYPE_EDGE_ANY      GPIO_CONF_INTTYPE(3)
#define GPIO_CONF_INTTYPE_LEVEL_LOW     GPIO_CONF_INTTYPE(4)
#define GPIO_CONF_INTTYPE_LEVEL_HIGH    GPIO_CONF_INTTYPE(5)
#define GPIO_CONF_OPEN_DRAIN            BIT(2)
#define GPIO_CONF_SOURCE_PWM            BIT(0)

#define GPIO_PWM                    REG_GPIO_BASE(26)
#define GPIO_PWM_ENABLE                  BIT(16)
#define GPIO_PWM_PRESCALER_M            0x000000FF
#define GPIO_PWM_PRESCALER_S            8
#define GPIO_PWM_TARGET_M               0x000000FF
#define GPIO_PWM_TARGET_S               0

#define GPIO_RTC_CALIB              REG_GPIO_BASE(27)
#define GPIO_RTC_CALIB_START            BIT(31)
#define GPIO_RTC_CALIB_PERIOD_M         0x000003FF
#define GPIO_RTC_CALIB_PERIOD_S         0

#define GPIO_RTC_CALIB_RESULT       REG_GPIO_BASE(28)
#define GPIO_RTC_CALIB_RESULT_READY       BIT(31)
#define GPIO_RTC_CALIB_RESULT_READY_REAL  BIT(30)
#define GPIO_RTC_CALIB_RESULT_VALUE_M     0x000FFFFF
#define GPIO_RTC_CALIB_RESULT_VALUE_S     0

// RTC
#define RTC_CTRL0                   REG_RTC_BASE(0)
#define RTC_COUNTER_ALARM           REG_RTC_BASE(1)
#define RTC_RESET_REASON0           REG_RTC_BASE(2)

#define RTC_RESET_REASON1           REG_RTC_BASE(5)
#define RTC_RESET_REASON1_CODE_M        0x0000000F
#define RTC_RESET_REASON1_CODE_S        0

#define RTC_RESET_REASON2           REG_RTC_BASE(6)
#define RTC_RESET_REASON2_CODE_M        0x0000003F
#define RTC_RESET_REASON2_CODE_S        8

#define RTC_COUNTER                 REG_RTC_BASE(7)
#define RTC_INT_SET                 REG_RTC_BASE(8)
#define RTC_INT_CLEAR               REG_RTC_BASE(9)
#define RTC_INT_ENABLE              REG_RTC_BASE(10)
#define RTC_SCRATCH(i)              REG_RTC_BASE(12 + (i))
#define RTC_GPIO_OUT                REG_RTC_BASE(26)
#define RTC_GPIO_ENABLE             REG_RTC_BASE(29)
#define RTC_GPIO_IN                 REG_RTC_BASE(35)

#define RTC_GPIO_CONF               REG_RTC_BASE(36)
#define RTC_GPIO_CONF_OUT_ENABLE        BIT(0)

#define RTC_GPIO_CFG(i)             REG_RTC_BASE(37 + (i))
#define RTC_GPIO_CFG3_PIN_PULLDOWN_SLEEP  BIT(5)
#define RTC_GPIO_CFG3_PIN_PULLUP_SLEEP    BIT(4)
#define RTC_GPIO_CFG3_PIN_PULLDOWN        BIT(3)
#define RTC_GPIO_CFG3_PIN_PULLUP          BIT(2)
#define RTC_GPIO_CFG3_PIN_FUNC_RTC_GPIO0  BIT(0)
#define RTC_GPIO_CFG3_PIN_FUNC_M          0x00000043
#define RTC_GPIO_CFG3_PIN_FUNC_S          0

// IOMUX
#define IOMUX_PIN_OUTPUT_ENABLE        BIT(0)
#define IOMUX_PIN_OUTPUT_ENABLE_SLEEP  BIT(1)
#define IOMUX_PIN_PULLDOWN_SLEEP       BIT(2)
#define IOMUX_PIN_PULLUP_SLEEP         BIT(3)
#define IOMUX_PIN_FUNC_LOW_M           0x00000003
#define IOMUX_PIN_FUNC_LOW_S           4
#define IOMUX_PIN_PULLDOWN             BIT(6)
#define IOMUX_PIN_PULLUP               BIT(7)
#define IOMUX_PIN_FUNC_HIGH_M          0x00000004
#define IOMUX_PIN_FUNC_HIGH_S          6
#define IOMUX_PIN_FUNC_MASK            0x00000130

// WARNING: Macro evaluates argument twice
#define IOMUX_FUNC(val) (VAL2FIELD(IOMUX_PIN_FUNC_LOW, val) | VAL2FIELD(IOMUX_PIN_FUNC_HIGH, val))

#define IOMUX_CONF                          REG_IOMUX_BASE(0)
#define IOMUX_CONF_SPI0_CLOCK_EQU_SYS_CLOCK     BIT(8)
#define IOMUX_CONF_SPI1_CLOCK_EQU_SYS_CLOCK     BIT(9)

#define IOMUX_GPIO12                        REG_IOMUX_BASE(1)
#define IOMUX_GPIO12_FUNC_MTDI                  IOMUX_FUNC(0)
#define IOMUX_GPIO12_FUNC_I2SI_DATA             IOMUX_FUNC(1)
#define IOMUX_GPIO12_FUNC_SPI1_MISO_DATA1       IOMUX_FUNC(2)
#define IOMUX_GPIO12_FUNC_GPIO                  IOMUX_FUNC(3)
#define IOMUX_GPIO12_FUNC_UART0_DTR             IOMUX_FUNC(4)

#define IOMUX_GPIO13                        REG_IOMUX_BASE(2)
#define IOMUX_GPIO13_FUNC_MTCK                  IOMUX_FUNC(0)
#define IOMUX_GPIO13_FUNC_I2SI_BCK              IOMUX_FUNC(1)
#define IOMUX_GPIO13_FUNC_SPI1_MOSI_DATA0       IOMUX_FUNC(2)
#define IOMUX_GPIO13_FUNC_GPIO                  IOMUX_FUNC(3)
#define IOMUX_GPIO13_FUNC_UART0_CTS             IOMUX_FUNC(4)

#define IOMUX_GPIO14                        REG_IOMUX_BASE(3)
#define IOMUX_GPIO14_FUNC_MTMS                  IOMUX_FUNC(0)
#define IOMUX_GPIO14_FUNC_I2SI_WS               IOMUX_FUNC(1)
#define IOMUX_GPIO14_FUNC_SPI1_CLK              IOMUX_FUNC(2)
#define IOMUX_GPIO14_FUNC_GPIO                  IOMUX_FUNC(3)
#define IOMUX_GPIO14_FUNC_UART0_DSR             IOMUX_FUNC(4)

#define IOMUX_GPIO15                        REG_IOMUX_BASE(4)
#define IOMUX_GPIO15_FUNC_MTDO                  IOMUX_FUNC(0)
#define IOMUX_GPIO15_FUNC_I2SO_BCK              IOMUX_FUNC(1)
#define IOMUX_GPIO15_FUNC_SPI1_CS0              IOMUX_FUNC(2)
#define IOMUX_GPIO15_FUNC_GPIO                  IOMUX_FUNC(3)
#define IOMUX_GPIO15_FUNC_UART0_RTS             IOMUX_FUNC(4)

#define IOMUX_GPIO3                         REG_IOMUX_BASE(5)
#define IOMUX_GPIO3_FUNC_UART0_RXD              IOMUX_FUNC(0)
#define IOMUX_GPIO3_FUNC_I2SO_DATA              IOMUX_FUNC(1)
#define IOMUX_GPIO3_FUNC_GPIO                   IOMUX_FUNC(3)
#define IOMUX_GPIO3_FUNC_CLOCK_XTAL_BLINK       IOMUX_FUNC(4)

#define IOMUX_GPIO1                         REG_IOMUX_BASE(6)
#define IOMUX_GPIO1_FUNC_UART0_TXD              IOMUX_FUNC(0)
#define IOMUX_GPIO1_FUNC_SPI0_CS1               IOMUX_FUNC(1)
#define IOMUX_GPIO1_FUNC_GPIO                   IOMUX_FUNC(3)
#define IOMUX_GPIO1_FUNC_CLOCK_RTC_BLINK        IOMUX_FUNC(4)

#define IOMUX_GPIO6                         REG_IOMUX_BASE(7)
#define IOMUX_GPIO6_FUNC_SD_CLK                 IOMUX_FUNC(0)
#define IOMUX_GPIO6_FUNC_SPI0_CLK               IOMUX_FUNC(1)
#define IOMUX_GPIO6_FUNC_GPIO                   IOMUX_FUNC(3)
#define IOMUX_GPIO6_FUNC_UART1_CTS              IOMUX_FUNC(4)

#define IOMUX_GPIO7                         REG_IOMUX_BASE(8)
#define IOMUX_GPIO7_FUNC_SD_DATA0               IOMUX_FUNC(0)
#define IOMUX_GPIO7_FUNC_SPI0_MISO_DATA1        IOMUX_FUNC(1)
#define IOMUX_GPIO7_FUNC_GPIO                   IOMUX_FUNC(3)
#define IOMUX_GPIO7_FUNC_UART1_TXD              IOMUX_FUNC(4)

#define IOMUX_GPIO8                         REG_IOMUX_BASE(9)
#define IOMUX_GPIO8_FUNC_SD_DATA1               IOMUX_FUNC(0)
#define IOMUX_GPIO8_FUNC_SPI0_MOSI_DATA0        IOMUX_FUNC(1)
#define IOMUX_GPIO8_FUNC_GPIO                   IOMUX_FUNC(3)
#define IOMUX_GPIO8_FUNC_UART1_RXD              IOMUX_FUNC(4)

#define IOMUX_GPIO9                         REG_IOMUX_BASE(10)
#define IOMUX_GPIO9_FUNC_SD_DATA2               IOMUX_FUNC(0)
#define IOMUX_GPIO9_FUNC_SPI0_DATA3             IOMUX_FUNC(1)
#define IOMUX_GPIO9_FUNC_GPIO                   IOMUX_FUNC(3)
#define IOMUX_GPIO9_FUNC_SPI1_DATA3             IOMUX_FUNC(4)

#define IOMUX_GPIO10                        REG_IOMUX_BASE(11)
#define IOMUX_GPIO10_FUNC_SD_DATA3              IOMUX_FUNC(0)
#define IOMUX_GPIO10_FUNC_SPI0_DATA2            IOMUX_FUNC(1)
#define IOMUX_GPIO10_FUNC_GPIO                  IOMUX_FUNC(3)
#define IOMUX_GPIO10_FUNC_SPI1_DATA2            IOMUX_FUNC(4)

#define IOMUX_GPIO11                        REG_IOMUX_BASE(12)
#define IOMUX_GPIO11_FUNC_SD_CMD                IOMUX_FUNC(0)
#define IOMUX_GPIO11_FUNC_SPI0_CS0              IOMUX_FUNC(1)
#define IOMUX_GPIO11_FUNC_GPIO                  IOMUX_FUNC(3)
#define IOMUX_GPIO11_FUNC_UART1_RTS             IOMUX_FUNC(4)

#define IOMUX_GPIO0                         REG_IOMUX_BASE(13)
#define IOMUX_GPIO0_FUNC_GPIO                   IOMUX_FUNC(0)
#define IOMUX_GPIO0_FUNC_SPI0_CS2               IOMUX_FUNC(1)
#define IOMUX_GPIO0_FUNC_CLOCK_OUT              IOMUX_FUNC(4)

#define IOMUX_GPIO2                         REG_IOMUX_BASE(14)
#define IOMUX_GPIO2_FUNC_GPIO                   IOMUX_FUNC(0)
#define IOMUX_GPIO2_FUNC_I2SO_WS                IOMUX_FUNC(1)
#define IOMUX_GPIO2_FUNC_UART1_TXD              IOMUX_FUNC(2)
#define IOMUX_GPIO2_FUNC_UART0_TXD              IOMUX_FUNC(4)

#define IOMUX_GPIO4                         REG_IOMUX_BASE(15)
#define IOMUX_GPIO4_FUNC_GPIO                   IOMUX_FUNC(0)
#define IOMUX_GPIO4_FUNC_CLOCK_XTAL             IOMUX_FUNC(1)

#define IOMUX_GPIO5                         REG_IOMUX_BASE(16)
#define IOMUX_GPIO5_FUNC_GPIO                   IOMUX_FUNC(0)
#define IOMUX_GPIO5_FUNC_CLOCK_RTC              IOMUX_FUNC(1)

#endif // REGISTERS_H_
