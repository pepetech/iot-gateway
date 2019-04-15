#include "pn532.h"

#define PN532_SPI_STATUS_READ     2
#define PN532_SPI_DATA_WRITE      1
#define PN532_SPI_DATA_READ       3

// pn532 interface funcions
uint8_t pn532_init()
{
    PN532_RESET();
    delay_ms(1);
    PN532_UNRESET();

    return 1;
}

void pn532_wake()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        PN532_SELECT();
        usart0_spi_transfer_byte(0x55);
        PN532_UNSELECT();
    }

    uint64_t ullTimeoutStart = g_ullSystemTick;

    while(PN532_READY() && (g_ullSystemTick < (ullTimeoutStart + PN532_WAKETIMEOUT)));
}

uint8_t pn532_write_command(uint8_t ubCommand, uint8_t *pubParameters, uint8_t ubNParameters)
{
    uint8_t *pubCommandBuf = (uint8_t*)malloc(1 + ubNParameters);

    if(!pubCommandBuf)
    {
        return 0;
    }


    pubCommandBuf[0] = ubCommand;
    for(uint8_t i = 0; i < ubNParameters; i++)
    {
        pubCommandBuf[1 + i] = pubParameters[i];
    }

    pn532_write_frame(pubCommandBuf, 1 + ubNParameters);

    uint64_t ullTimeoutStart = g_ullSystemTick;
    while(!pn532_ready())
    {
        if (g_ullSystemTick > (ullTimeoutStart + PN532_ACKTIMEOUT))
        {
            free(pubCommandBuf);
            return PN532_TIMEOUT;
        }
    }

    if(pn532_read_ack() == PN532_INVALID_ACK)
    {
        free(pubCommandBuf);
        return  PN532_INVALID_ACK;
    }

    free(pubCommandBuf);
    return 1;
}
uint8_t pn532_read_response(uint8_t ubCommand, uint8_t *pubBuf, uint8_t ubLength)
{
    uint64_t ullTimeoutStart = g_ullSystemTick;
    while(!pn532_ready())
    {
        if (g_ullSystemTick > (ullTimeoutStart + PN532_RESPONSETIMEOUT))
        {
            DBGPRINTLN_CTX("Timeout");
            return PN532_TIMEOUT;
        }
    }

    uint8_t *pubFrameBuf = (uint8_t*)malloc(ubLength + 2);


    if(!pubFrameBuf)
    {
        DBGPRINTLN_CTX("Memory allocation failure");
        return 0;
    }

    pn532_read_frame(pubFrameBuf, ubLength + 2);

    if(pubFrameBuf[0] != PN532_PN532HOST)
    {
        DBGPRINTLN_CTX("Invalid frame, not PN532_PN532HOST");
        free(pubFrameBuf);
        // pubBuf[0] = pubFrameBuf[0]; contains error code
        return PN532_INVALID_FRAME;
    }
    if (pubFrameBuf[1] != (ubCommand + 1))
    {
        DBGPRINTLN_CTX("Invalid frame, not the correct command");

        free(pubFrameBuf);
        return PN532_INVALID_FRAME;
    }

    for(uint8_t i = 0; i < ubLength; i++)
    {
        pubBuf[i] = pubFrameBuf[2 + i];
    }

    free(pubFrameBuf);
    return 1;
}

void pn532_write_frame(uint8_t *pubPayload, uint8_t ubLength)
{
    uint8_t *pubBuf = (uint8_t*)malloc(9 + ubLength);

    if(!pubBuf)
    {
        return;
    }


    pubBuf[0] = PN532_SPI_DATA_WRITE;        // data writing (this byte is used for spi frames only)
    pubBuf[1] = PN532_PREAMBLE;              // Preamble
    pubBuf[2] = PN532_STARTCODE1;            // Start of Packet Code
    pubBuf[3] = PN532_STARTCODE2;            // Start of Packet Code byte 2
    pubBuf[4] = ubLength + 1;                // Packet Length: TFI + DATA
    pubBuf[5] = ~pubBuf[4] + 1;               // Packet Length Checksum
    pubBuf[6] = PN532_HOSTPN532;             // TFI, Specific PN532 Frame Identifier

    uint8_t ubCheckSum = PN532_HOSTPN532;   // sum of TFI + DATA

    for(uint8_t i = 0; i < ubLength; i++)
    {
        pubBuf[7 + i] = pubPayload[i];        // Packet Data
        ubCheckSum += pubPayload[i];
    }

    ubCheckSum = ~ubCheckSum + 1;

    pubBuf[7+ubLength] = ubCheckSum;         // Packet Data Checksum
    pubBuf[8+ubLength] = PN532_POSTAMBLE;    // Postamble

    delay_ms(1);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        PN532_SELECT(); // wake up PN532
        usart0_spi_write(pubBuf, 9 + ubLength);
        PN532_UNSELECT();
    }

    free(pubBuf);
}
uint8_t pn532_read_frame(uint8_t *pubPayload, uint8_t ubMaxLength)
{
    uint8_t ubPreamble[4];

    ubPreamble[0] = PN532_SPI_DATA_READ;        // data writing (this byte is used for spi frames only)
    ubPreamble[1] = PN532_PREAMBLE;              // Preamble
    ubPreamble[2] = PN532_STARTCODE1;            // Start of Packet Code
    ubPreamble[3] = PN532_STARTCODE2;            // Start of Packet Code byte 2

    uint8_t ubPostamble[2];

    uint8_t ubLength;

    delay_ms(1);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        PN532_SELECT(); // Wake up PN532

        usart0_spi_write(ubPreamble, 4);    // write "header"

        ubLength = usart0_spi_transfer_byte(0x00);  // read length byte

        if(((usart0_spi_transfer_byte(0x00) + ubLength) & 0xFF) != 0)    // read length byte checksum and verify ubLength
        {
            PN532_UNSELECT();

            DBGPRINTLN_CTX("INVALID FRAME");

            return PN532_INVALID_FRAME;
        }
        if(ubLength > ubMaxLength)  // verify that payload does not exceed buffer size
        {
            PN532_UNSELECT();

            DBGPRINTLN_CTX("Frame too long");

            return PN532_NO_SPACE;
        }

        usart0_spi_read(pubPayload, ubLength);   // read payload
        usart0_spi_read(ubPostamble, 2);    // read payload checksum and postamble

        PN532_UNSELECT();
    }

    uint8_t ubChecksum = 0;

    for(uint8_t i = 0; i < ubLength; i++)   // calculate checksum
    {
        ubChecksum += pubPayload[i];
    }
    ubChecksum = ~ubChecksum + 1;

    if (ubPostamble[0] != ubChecksum)   // verify checksum \ data integrity
    {
        return PN532_INVALID_FRAME;
    }

    return ubLength;
}

uint8_t pn532_read_ack()
{
    uint8_t ubAck[6] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ubAckBuf[6] = {0, 0, 0, 0, 0, 0};

    delay_ms(1);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        PN532_SELECT();

        usart0_spi_transfer_byte(PN532_SPI_DATA_READ);
        usart0_spi_read(ubAckBuf, 6);
        PN532_UNSELECT();
    }

    for(uint8_t i = 0; i < 6; i++)
    {
        if(ubAckBuf[i] != ubAck[i])
        {
            return PN532_INVALID_ACK;
        }
    }

    return 1;
}
void pn532_write_ack(uint8_t ubNack)
{
    if(ubNack)
    {
        uint8_t ubNack[6] = {0, 0, 0xFF, 0, 0xFF, 0};

        delay_ms(1);

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            PN532_SELECT();
            usart0_spi_write(ubNack, 6);
            PN532_UNSELECT();
        }
    }
    else
    {
        uint8_t ubAck[6] = {0, 0, 0xFF, 0, 0xFF, 0};

        delay_ms(1);

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            PN532_SELECT();
            usart0_spi_write(ubAck, 6);
            PN532_UNSELECT();
        }
    }
}

uint8_t pn532_ready()
{
#ifdef  PN532_READY_HW
    return !(PN532_READY());
#else   // PN532_READY_HW
    uint8_t ubStatus;

    delay_ms(1);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        PN532_SELECT();
        usart0_spi_transfer_byte(PN532_SPI_STATUS_READ);
        ubStatus = usart0_spi_transfer_byte(0x00) & 1;
        PN532_UNSELECT();
    }
    return ubStatus;
#endif  // PN532_READY_HW
}

// pn532 RF communication commands
uint32_t pn532_get_version()
{
    uint8_t ubBuf[4];

    if(pn532_write_command(PN532_COMMAND_GETFIRMWAREVERSION, NULL, 0) != 1)
    {
        return 0;
    }
    if(pn532_read_response(PN532_COMMAND_GETFIRMWAREVERSION, ubBuf, 4) != 1)
    {
        return 0;
    }

    return (uint32_t)(ubBuf[0] << 24) | (uint32_t)(ubBuf[1] << 16) | (uint32_t)(ubBuf[2] << 8) | (uint32_t)ubBuf[3];
}

void pn532_Get_General_Status()
{
    return;
}

uint8_t pn532_read_register(uint16_t usAddr)
{
    uint8_t ubParam[2] = {usAddr >> 8, usAddr & 0xFF};
    uint8_t ubValue;

    if(pn532_write_command(PN532_COMMAND_READREGISTER, ubParam, 2) != 1)
        return 0;
    if(pn532_read_response(PN532_COMMAND_READREGISTER, &ubValue, 1) != 1)
        return 0;

    return ubValue;
}
uint8_t pn532_write_register(uint16_t usAddr, uint8_t ubValue)
{
    uint8_t ubParam[3] = {usAddr >> 8, usAddr & 0xFF, ubValue};

    if(pn532_write_command(PN532_COMMAND_WRITEREGISTER, ubParam, 3) != 1)
        return 0;
    if(pn532_read_response(PN532_COMMAND_WRITEREGISTER, NULL, 0) != 1)
        return 0;

    return 1;
}

uint8_t pn532_read_gpio(uint8_t ubPort)
{
    uint8_t ubValues[3];

    if(pn532_write_command(PN532_COMMAND_READGPIO, NULL, 0) != 1)
        return 0;
    if(pn532_read_response(PN532_COMMAND_READGPIO, ubValues, 3) != 1)
        return 0;

    return ubValues[ubPort & 0x03];
}
uint8_t pn532_write_gpio(uint8_t ubPort, uint8_t ubPins)
{
    uint8_t ubParam[2];

    if(ubPort == PN532_PORT_P3)
    {
        ubParam[0] = 0x80;
        ubParam[0] |= ubPins;
        ubParam[1] = 0;
    }
    if(ubPort == PN532_PORT_P7)
    {
        ubParam[1] = 0x80;
        ubParam[1] |= ubPins;
        ubParam[0] = 0;
    }

    if(pn532_write_command(PN532_COMMAND_WRITEGPIO, ubParam, 2) != 1)
        return 0;
    if(pn532_read_response(PN532_COMMAND_WRITEGPIO, NULL, 0) != 1)
        return 0;

    return 1;
}

uint8_t pn532_set_serial_baud(uint8_t ubBaud)
{
    if(pn532_write_command(PN532_COMMAND_SETSERIALBAUDRATE, &ubBaud, 1) != 1)
        return 0;
    if(pn532_read_response(PN532_COMMAND_SETSERIALBAUDRATE, NULL, 0) != 1)
        return 0;

    pn532_write_ack(0);

    return 1;
}

uint8_t pn532_set_parameters(uint8_t ubFlags)
{
    if(pn532_write_command(PN532_COMMAND_SETPARAMETERS, &ubFlags, 1) != 1)
        return 0;
    if(pn532_read_response(PN532_COMMAND_SETPARAMETERS, NULL, 0) != 1)
        return 0;

    return 1;
}

uint8_t pn532_sam_configuration(uint8_t ubMode, uint8_t ubTimeout, uint8_t ubIrq)
{
    uint8_t ubParam[3] = {ubMode, ubTimeout, ubIrq};

    if(pn532_write_command(PN532_COMMAND_SAMCONFIGURATION, ubParam, 3) != 1)
        return 0;
    if(pn532_read_response(PN532_COMMAND_SAMCONFIGURATION, NULL, 0) != 1)
        return 0;

    return 1;
}

uint8_t pn532_power_down(uint8_t ubWakeSrcs, uint8_t ubGenIrq)
{
    uint8_t ubParam[2] = {ubWakeSrcs, ubGenIrq};
    uint8_t ubStatus;

    if(pn532_write_command(PN532_COMMAND_POWERDOWN, ubParam, 2) != 1)
        return 0;
    if(pn532_read_response(PN532_COMMAND_POWERDOWN, &ubStatus, 1) != 1)
        return 0;
    if(ubStatus & 0x3F != 0x00)
        return 0;

    delay_ms(1);

    return 1;
}

// RF communication
uint8_t pn532_rf_configuration(uint8_t ubCfgItem, uint8_t *pubCfgData, uint8_t ubCfgDataLen)
{
    uint8_t *pubCfgBuf = malloc(1 + ubCfgDataLen);

    pubCfgBuf[0] = ubCfgItem;

    for(uint8_t i = 0; i < ubCfgDataLen; i++)
    {
        pubCfgBuf[1+i] = pubCfgData[i];
    }

    if(pn532_write_command(PN532_COMMAND_RFCONFIGURATION, pubCfgBuf, 1 + ubCfgDataLen) != 1)
    {
        free(pubCfgBuf);

        return 0;
    }
    if(pn532_read_response(PN532_COMMAND_RFCONFIGURATION, NULL, 0) != 1)
    {
        free(pubCfgBuf);

        return 0;
    }

    free(pubCfgBuf);

    return 1;
}

uint8_t pn532_set_passive_activation_retries(uint8_t ubRetries)
{
    uint8_t ubBuf[3] = {0xFF, 0x01, ubRetries};

    if(pn532_rf_configuration(PN532_RF_ITEM_MAXRETRIES, ubBuf, 3) != 1)
        return 0;

    return 1;
}

uint8_t pn532_set_rf_field(uint8_t ubRfFieldFlags)
{
    if(pn532_rf_configuration(PN532_RF_ITEM_RF_FIELD, &ubRfFieldFlags, 1) != 1)
        return 0;

    return 1;
}

uint8_t pn532_rf_regulation_test(uint8_t ubSpeedAndFramming)
{
    if(pn532_write_command(PN532_COMMAND_RFREGULATIONTEST, &ubSpeedAndFramming, 1) != 1)
        return 0;

    return 1;
}

// Initiator
uint8_t pn532_inlist_passive_target(uint8_t ubMaxTg, uint8_t ubBrTg, uint8_t *pubInitData, uint8_t ubInitDataLen, uint8_t *pubTg1, uint8_t *pubTg2, uint8_t ubTgDataLen)
{
    uint8_t *pubTgTBuf = (uint8_t *)malloc(2 + ubInitDataLen);
    uint8_t *pubTgRBuf = (uint8_t *)malloc(ubMaxTg * ubTgDataLen);

    pubTgTBuf[0] = ubMaxTg;
    pubTgTBuf[1] = ubBrTg;

    for(uint8_t i = 0; i < ubInitDataLen; i++)
    {
        pubTgTBuf[2 + i] = pubInitData[i];
    }

    if(pn532_write_command(PN532_COMMAND_INLISTPASSIVETARGET, pubTgTBuf, 2 + ubInitDataLen) != 1)
    {
        DBGPRINTLN_CTX("INVALID COMMAND");

        free(pubTgTBuf);
        free(pubTgRBuf);

        return 0;
    }
    if(pn532_read_response(PN532_COMMAND_INLISTPASSIVETARGET, pubTgRBuf, (ubMaxTg * ubTgDataLen) + 1) != 1)
    {
        DBGPRINTLN_CTX("INVALID RESPONSE");

        free(pubTgTBuf);
        free(pubTgRBuf);

        return 0;
    }
    

    memset(pubTg1, 0x00, ubTgDataLen);
    memset(pubTg2, 0x00, ubTgDataLen);

    uint8_t ubNTg = 0;

    switch(ubBrTg)
    {
        case PN532_MIFARE_ISO14443A:

            ubNTg = pubTgRBuf[0];
            uint8_t ubBufPtr = 1;
            uint8_t ubNTg1Len = 0;
            uint8_t ubNTg2Len = 0;

            //DBGPRINTLN_CTX("N tags: %d", ubNTg);

            if (ubNTg > 0) 
            {
                ubNTg1Len += 6 + pubTgRBuf[ubBufPtr + 4] + pubTgRBuf[ubBufPtr + 4 + 1 + pubTgRBuf[ubBufPtr + 4]];

                //DBGPRINTLN_CTX("tag1 Len: %d", ubNTg1Len);

                for(uint8_t i = 0; i < ubNTg1Len; i++)
                {
                    //DBGPRINTLN_CTX("tag1: 0x%02X", pubTgRBuf[ubBufPtr + i]);
                    pubTg1[i] = pubTgRBuf[ubBufPtr + i];
                }

                if (ubNTg > 1) 
                {
                    ubBufPtr += ubNTg1Len;
                    ubNTg2Len += 6 + pubTgRBuf[ubBufPtr + 4] + pubTgRBuf[ubBufPtr + 4 + 1 + pubTgRBuf[ubBufPtr + 4]];

                    for(uint8_t i = 0; i < ubNTg1Len; i++)
                    {
                        pubTg2[i] = pubTgRBuf[ubBufPtr + i];
                    }
                }
            }
                
            break;
    }

    free(pubTgTBuf);
    free(pubTgRBuf);

    return ubNTg;
}
uint8_t pn532_read_passive_target_id(uint8_t *pubUid, uint8_t *pubUidLen)
{
    memset(pubUid, 0x00, 7);
   *pubUidLen = 0;

    uint8_t ubTgBuf[24];

    memset(ubTgBuf, 0x00, 24);

    if(pn532_inlist_passive_target(1, PN532_MIFARE_ISO14443A, NULL, 0, ubTgBuf, NULL, 24) != 1)
        return 0;

    *pubUidLen = ubTgBuf[4];

    for (uint8_t i = 0; i < *pubUidLen; i++) 
    {
        pubUid[i] = ubTgBuf[5 + i];
    }

    return 1;
}

// Target