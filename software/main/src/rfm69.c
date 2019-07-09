#include "rfm69.h"

static uint16_t usRadioPacketID = 0;
static uint8_t ubRadioNodeID = 0;
static volatile uint8_t ubRadioCurrentMode = RFM69_RF_OPMODE_STANDBY;
static int8_t bRadioCurrentPowerLevel = RFM69_MAXIMUM_TX_POWER;
static int8_t *pbRadioATCPowerLevel = NULL;
static int8_t *pbRadioATCTargetRemoteRSSI = NULL;
static int8_t *pbRadioATCRemoteRSSI = NULL;
static int8_t *pbRadioLastRSSI = NULL;
static uint64_t ullLastTX = 0;
static rfm69_pending_packet_t *pRadioACKPending = NULL;
static rfm69_pending_packet_t *pRadioRELPending = NULL;
static rfm69_pending_packet_t *pRadioRELACKPending = NULL;
static blob_fifo_t *pRadioRXPacketFIFO = NULL;
static blob_fifo_t *pRadioTXPacketFIFO = NULL;
static rfm69_timeout_callback_fn_t pfRadioTimeoutCallback = NULL;
static rfm69_tx_callback_fn_t pfRadioTXCallback = NULL;
static rfm69_ack_callback_fn_t pfRadioACKCallback = NULL;
static rfm69_rx_callback_fn_t pfRadioRXCallback = NULL;


static uint8_t rfm69_read_register(uint8_t ubRegister)
{
	uint8_t ubValue;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		RFM69_SELECT();

		usart3_spi_transfer_byte(ubRegister & 0x7F);

		ubValue = usart3_spi_transfer_byte(0);

		RFM69_UNSELECT();
	}

	return ubValue;
}
static void rfm69_write_register(uint8_t ubRegister, uint8_t ubValue)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		RFM69_SELECT();

		usart3_spi_transfer_byte(ubRegister | 0x80);
		usart3_spi_transfer_byte(ubValue);

		RFM69_UNSELECT();
	}
}
static void rfm69_rmw_register(uint8_t ubRegister, uint8_t ubMask, uint8_t ubValue)
{
	rfm69_write_register(ubRegister, (rfm69_read_register(ubRegister) & ubMask) | ubValue);
}

static inline uint16_t rfm69_get_next_packet_id()
{
    while(!++usRadioPacketID);

    return usRadioPacketID;
}

static uint8_t rfm69_add_pending_packet(rfm69_pending_packet_t **ppList, const rfm69_packet_header_t *pHeader, const uint8_t *pubData, uint8_t ubDataSize, uint16_t usRetryDelay, uint16_t usRetriesLeft)
{
    if(!ppList)
        return 0;

    if(!pHeader)
        return 0;

    rfm69_pending_packet_t *pNewPacket = (rfm69_pending_packet_t *)malloc(sizeof(rfm69_pending_packet_t));

    if(!pNewPacket)
        return 0;

    memset(pNewPacket, 0, sizeof(rfm69_pending_packet_t));

	if(pubData && ubDataSize)
	{
		pNewPacket->pubData = (uint8_t *)malloc(ubDataSize);

		if(!pNewPacket->pubData)
		{
			free(pNewPacket);

			return 0;
		}

		memcpy(pNewPacket->pubData, pubData, ubDataSize);

    	pNewPacket->ubDataSize = ubDataSize;
	}

	memcpy(&pNewPacket->sHeader, pHeader, sizeof(rfm69_packet_header_t));

	pNewPacket->usRetryDelay = usRetryDelay;
	pNewPacket->usRetriesLeft = usRetriesLeft;

	// Insert at the head of the list
    pNewPacket->pNext = (*ppList);
    pNewPacket->pPrev = NULL;

    if(*ppList)
        (*ppList)->pPrev = pNewPacket;

    (*ppList) = pNewPacket;

    return 1;
}
static uint8_t rfm69_remove_pending_packet(rfm69_pending_packet_t **ppList, rfm69_pending_packet_t *pPacket)
{
    if(!ppList)
        return 0;

    if(!pPacket)
        return 0;

	free(pPacket->pubData);

    if((*ppList) == pPacket)
        (*ppList) = pPacket->pNext;

    if(pPacket->pPrev)
        pPacket->pPrev->pNext = pPacket->pNext;

    if(pPacket->pNext)
        pPacket->pNext->pPrev = pPacket->pPrev;

    free(pPacket);

    return 1;
}
static rfm69_pending_packet_t* rfm69_find_pending_packet(rfm69_pending_packet_t *pList, uint16_t usID, uint8_t ubNodeID)
{
    if(!usID)
		return NULL;

	for(rfm69_pending_packet_t *pPacket = pList; pPacket; pPacket = pPacket->pNext)
	{
		if(pPacket->sHeader.usID == usID && pPacket->sHeader.ubReceiverNodeID == ubNodeID)
			return pPacket;
	}

	return NULL;
}
static uint8_t rfm69_clear_pending_packets(rfm69_pending_packet_t **ppList)
{
    if(!ppList)
        return 0;

    while(*ppList)
    {
        rfm69_pending_packet_t *pPacket = (*ppList);

        (*ppList) = pPacket->pNext;

		free(pPacket->pubData);
        free(pPacket);
    }

    return 1;
}

static uint8_t rfm69_pack_header(const rfm69_packet_header_t *pHeader, uint8_t *pubBuffer, uint8_t ubBufferSize)
{
    if(!pubBuffer || !ubBufferSize)
		return 0;

	if(ubBufferSize < RFM69_PACKET_HEADER_SIZE)
		return 0;

    if(!pHeader)
        return 0;

	uint8_t ubFlags = ((!!pHeader->ubACKRequested) << RFM69_CTL_ACKR) |
					  ((!!pHeader->ubACKSent) << RFM69_CTL_ACKS) |
					  ((!!pHeader->ubRELRequested) << RFM69_CTL_RELR) |
					  ((!!pHeader->ubRELSent) << RFM69_CTL_RELS) |
					  ((!!pHeader->ubQoSLevel) << RFM69_CTL_QOS);

	memcpy(pubBuffer, &ubFlags, sizeof(uint8_t));

	pubBuffer += sizeof(uint8_t);

	memcpy(pubBuffer, &pHeader->usID, sizeof(uint16_t));

	pubBuffer += sizeof(uint16_t);

	memcpy(pubBuffer, &pHeader->bRemoteRSSI, sizeof(int8_t));

	pubBuffer += sizeof(int8_t);

	memcpy(pubBuffer, &pHeader->ubReceiverNodeID, sizeof(uint8_t));

	pubBuffer += sizeof(uint8_t);

	memcpy(pubBuffer, &pHeader->ubSenderNodeID, sizeof(uint8_t));

	pubBuffer += sizeof(uint8_t);

	return 1;
}
static uint8_t rfm69_unpack_header(rfm69_packet_header_t *pHeader, const uint8_t *pubBuffer, uint8_t ubBufferSize)
{
    if(!pubBuffer || !ubBufferSize)
		return 0;

	if(ubBufferSize < RFM69_PACKET_HEADER_SIZE)
		return 0;

    if(!pHeader)
        return 0;

	uint8_t ubFlags;

	memcpy(&ubFlags, pubBuffer, sizeof(uint8_t));

	pubBuffer += sizeof(uint8_t);

	pHeader->ubACKRequested = !!(ubFlags & BIT(RFM69_CTL_ACKR));
	pHeader->ubACKSent = !!(ubFlags & BIT(RFM69_CTL_ACKS));
	pHeader->ubRELRequested = !!(ubFlags & BIT(RFM69_CTL_RELR));
	pHeader->ubRELSent = !!(ubFlags & BIT(RFM69_CTL_RELS));
	pHeader->ubQoSLevel = !!(ubFlags & BIT(RFM69_CTL_QOS));

	memcpy(&pHeader->usID, pubBuffer, sizeof(uint16_t));

	pubBuffer += sizeof(uint16_t);

	memcpy(&pHeader->bRemoteRSSI, pubBuffer, sizeof(int8_t));

	pubBuffer += sizeof(int8_t);

	memcpy(&pHeader->ubReceiverNodeID, pubBuffer, sizeof(uint8_t));

	pubBuffer += sizeof(uint8_t);

	memcpy(&pHeader->ubSenderNodeID, pubBuffer, sizeof(uint8_t));

	pubBuffer += sizeof(uint8_t);

	return 1;
}
static uint8_t rfm69_build_payload(const rfm69_packet_header_t *pHeader, const uint8_t *pubData, uint8_t ubDataSize, uint8_t *pubBuffer, uint8_t ubBufferSize, uint8_t *pubPayloadSize)
{
    if(!pubBuffer || !ubBufferSize)
		return 0;

	if(ubBufferSize < ubDataSize + RFM69_PACKET_HEADER_SIZE)
		return 0;

    if(!pHeader)
        return 0;

	if(ubDataSize > RFM69_MAX_DATA_SIZE)
		return 0;

	rfm69_pack_header(pHeader, pubBuffer, RFM69_PACKET_HEADER_SIZE);

	pubBuffer += RFM69_PACKET_HEADER_SIZE;

	if(pubData && ubDataSize)
	{
		memcpy(pubBuffer, pubData, ubDataSize);

		pubBuffer += ubDataSize;
	}

	if(pubPayloadSize)
		*pubPayloadSize = ubDataSize + RFM69_PACKET_HEADER_SIZE;

	return 1;
}
static uint8_t rfm69_parse_payload(rfm69_packet_header_t *pHeader, uint8_t **ppubData, uint8_t *pubDataSize, uint8_t *pubBuffer, uint8_t ubBufferSize)
{
    if(!pubBuffer || !ubBufferSize)
		return 0;

	if(ubBufferSize < RFM69_PACKET_HEADER_SIZE)
		return 0;

    if(!pHeader)
		return 0;

	rfm69_unpack_header(pHeader, pubBuffer, RFM69_PACKET_HEADER_SIZE);

	pubBuffer += RFM69_PACKET_HEADER_SIZE;

	uint8_t ubDataSize = ubBufferSize - RFM69_PACKET_HEADER_SIZE;

	if(pubDataSize)
		*pubDataSize = ubDataSize;

	if(ppubData && ubDataSize > 0)
		*ppubData = pubBuffer;

	return 1;
}
static uint8_t rfm69_build_pending_packet_payload(const rfm69_pending_packet_t *pPacket, uint8_t *pubBuffer, uint8_t ubBufferSize, uint8_t *pubPayloadSize)
{
	if(!pPacket)
		return 0;

	return rfm69_build_payload(&pPacket->sHeader, pPacket->pubData, pPacket->ubDataSize, pubBuffer, ubBufferSize, pubPayloadSize);
}

uint8_t rfm69_init(uint8_t ubNodeID, uint8_t ubNetID, const void *pvEncKey)
{
	free(pbRadioATCPowerLevel);
	free(pbRadioATCTargetRemoteRSSI);
	free(pbRadioATCRemoteRSSI);
	free(pbRadioLastRSSI);

	blob_fifo_delete(pRadioTXPacketFIFO);
	blob_fifo_delete(pRadioRXPacketFIFO);

	pbRadioATCPowerLevel = (int8_t *)malloc(256);

	if(!pbRadioATCPowerLevel)
		return 0;

	pbRadioATCTargetRemoteRSSI = (int8_t *)malloc(256);

	if(!pbRadioATCTargetRemoteRSSI)
	{
		free(pbRadioATCPowerLevel);

		return 0;
	}

	pbRadioATCRemoteRSSI = (int8_t *)malloc(256);

	if(!pbRadioATCRemoteRSSI)
	{
		free(pbRadioATCPowerLevel);
		free(pbRadioATCTargetRemoteRSSI);

		return 0;
	}

	pbRadioLastRSSI = (int8_t *)malloc(256);

	if(!pbRadioLastRSSI)
	{
		free(pbRadioATCPowerLevel);
		free(pbRadioATCTargetRemoteRSSI);
		free(pbRadioATCRemoteRSSI);

		return 0;
	}

	pRadioRXPacketFIFO = blob_fifo_init(NULL, RFM69_RX_PACKET_FIFO_SIZE);

	if(!pRadioRXPacketFIFO)
	{
		free(pbRadioATCPowerLevel);
		free(pbRadioATCTargetRemoteRSSI);
		free(pbRadioATCRemoteRSSI);
		free(pbRadioLastRSSI);

		return 0;
	}

	pRadioTXPacketFIFO = blob_fifo_init(NULL, RFM69_TX_PACKET_FIFO_SIZE);

	if(!pRadioTXPacketFIFO)
	{
		free(pbRadioATCPowerLevel);
		free(pbRadioATCTargetRemoteRSSI);
		free(pbRadioATCRemoteRSSI);
		free(pbRadioLastRSSI);

		blob_fifo_delete(pRadioRXPacketFIFO);

		return 0;
	}

	memset(pbRadioATCPowerLevel, RFM69_MAXIMUM_TX_POWER, 256);
	memset(pbRadioATCTargetRemoteRSSI, 0, 256);
	memset(pbRadioATCRemoteRSSI, -128, 256);
	memset(pbRadioLastRSSI, -128, 256);

	RFM69_RESET();
	delay_ms(10);
	RFM69_UNRESET();

	delay_ms(10);

	if(rfm69_read_register(RFM69_REG_VERSION) == 0x24)
	{
		ubRadioNodeID = ubNodeID;

		// ...---------|------------|-----------|-----------|------------|---------...
		//           RxBw          Fdev       Carrier      Fdev          RxBw

		// 1)  0.5 <= (2 * Fdev) / BR <= 10				(modulation index, MI)
		// 2)  BR < 2 * RxBw							(bit rate)
		// 3)  RxBw >= Fdev + (BR / 2)					(receiver bandwidth)
		// 4)  RxBwAfc >=  Fdev + (BR / 2) + LOoffset	(receiver AFC bandwidth)
		// 5)  Fdev + (BR / 2) < 500kHz					(maximum RxBw setting)

		rfm69_write_register(RFM69_REG_OPMODE, RFM69_RF_OPMODE_STANDBY); // RegOpMode: Standby
		rfm69_write_register(RFM69_REG_DATAMODUL, RFM69_RF_DATAMODUL_DATAMODE_PACKET | RFM69_RF_DATAMODUL_MODULATIONTYPE_FSK | RFM69_RF_DATAMODUL_MODULATIONSHAPING_00); // RegDataModul: Packet mode, FSK, no shaping
		rfm69_write_register(RFM69_REG_BITRATEMSB, RFM69_RF_BITRATEMSB_25000); // RegBitrateMsb: 25,000 bps
		rfm69_write_register(RFM69_REG_BITRATELSB, RFM69_RF_BITRATELSB_25000); // RegBitrateLsb
		rfm69_write_register(RFM69_REG_FDEVMSB, RFM69_RF_FDEVMSB_20000); // RegFdevMsb: 20 kHz Single-side TX Deviation
		rfm69_write_register(RFM69_REG_FDEVLSB, RFM69_RF_FDEVLSB_20000); // RegFdevLsb
		rfm69_write_register(RFM69_REG_FRFMSB, RFM69_RF_FRFMSB_868 + 0x00); // RegFrfMsb: 868,2 MHz
		rfm69_write_register(RFM69_REG_FRFMID, RFM69_RF_FRFMID_868 + 0x0C); // RegFrfMid
		rfm69_write_register(RFM69_REG_FRFLSB, RFM69_RF_FRFLSB_868 + 0xCC); // RegFrfLsb
		rfm69_write_register(RFM69_REG_AFCCTRL, RFM69_RF_AFCCTRL_LOWBETA_OFF); // RegAfcCtrl: Disable Low beta AFC
		rfm69_write_register(RFM69_REG_LISTEN1, RFM69_RF_LISTEN1_RESOL_IDLE_4100 | RFM69_RF_LISTEN1_RESOL_RX_64 | RFM69_RF_LISTEN1_CRITERIA_RSSIANDSYNC | RFM69_RF_LISTEN1_END_10); // RegListen1: Idle resolution 4.1ms, RX resolution 64us, RSSI and Sync to receive packet, stay in listen mode after IRQ
		rfm69_write_register(RFM69_REG_LISTEN2, 0xC8); // RegListen2: 820 ms idle (200 * 4,1 ms)
		rfm69_write_register(RFM69_REG_LISTEN3, 0x50); // RegListen3: 5120 us RX (80 * 64 us)
		rfm69_write_register(RFM69_REG_PALEVEL, RFM69_RF_PALEVEL_PA1_ON | RFM69_RF_PALEVEL_OUTPUTPOWER_10000); // RegPaLevel: Enable PA1 with minimum power
		rfm69_write_register(RFM69_REG_PARAMP, RFM69_RF_PARAMP_40); // RegPaRamp: 40us PA Ramp time
		rfm69_write_register(RFM69_REG_OCP, RFM69_RF_OCP_OFF); // RegOcp: OCP off because we only use H (High Power) devices
		rfm69_write_register(RFM69_REG_LNA, RFM69_RF_LNA_ZIN_50 | RFM69_RF_LNA_GAINSELECT_AUTO); // RegLNA: 50 Ohm impedance, gain set by AGC loop
		rfm69_write_register(RFM69_REG_RXBW, RFM69_RF_RXBW_DCCFREQ_010 | RFM69_RF_RXBW_MANT_24 | RFM69_RF_RXBW_EXP_3); // RegRxBw: DCCFreq = 4% RxBw, 41,7 kHz Single-side RX Bandwidth
		rfm69_write_register(RFM69_REG_AFCBW, RFM69_RF_AFCBW_DCCFREQAFC_100 | RFM69_RF_AFCBW_MANTAFC_16 | RFM69_RF_AFCBW_EXPAFC_2); // RegAfcBw: DCCFreqAfc = 1% RxBwAfc, 125 kHz Single-side RX Bandwidth during AFC
		rfm69_write_register(RFM69_REG_AFCFEI, RFM69_RF_AFCFEI_AFCAUTO_OFF | RFM69_RF_AFCFEI_AFCAUTOCLEAR_OFF); // RegAfcFei: Disable auto AFC on receiver startup
		rfm69_write_register(RFM69_REG_DIOMAPPING1, RFM69_RF_DIOMAPPING1_DIO0_01); // RegDioMapping1: Pin DIO0 outputs PayloadReady
		rfm69_write_register(RFM69_REG_DIOMAPPING2, RFM69_RF_DIOMAPPING2_CLKOUT_OFF); // RegDioMapping2: Disable CLKOUT on DIO5 to save power
		rfm69_write_register(RFM69_REG_RSSITHRESH, -(RFM69_NORMAL_RX_SENSITIVITY) << 1); // RegRssiThresh: Min RSSI to start receiving
		rfm69_write_register(RFM69_REG_RXTIMEOUT1, 0x00); // RegRxTimeout1: No timeout if no RSSI detected
		rfm69_write_register(RFM69_REG_RXTIMEOUT2, 0x57); // RegRxTimeout2: Timeout after Rssi interrupt and no PayloadReady interrupt (~55ms)
		rfm69_write_register(RFM69_REG_PREAMBLEMSB, 0x00); // RegPreambleMsb: 5 bytes preamble
		rfm69_write_register(RFM69_REG_PREAMBLELSB, 0x05); // RegPreambleLsb
		rfm69_write_register(RFM69_REG_SYNCCONFIG, RFM69_RF_SYNC_ON | RFM69_RF_SYNC_SIZE_3); // RegSyncConfig: Enable sync word, 3 bytes sync word
		rfm69_write_register(RFM69_REG_SYNCVALUE1, 0x21); // RegSyncValue1: 0x21 (Hardcoded)
		rfm69_write_register(RFM69_REG_SYNCVALUE2, 0x29); // RegSyncValue2: 0x29 (Hardcoded)
		rfm69_write_register(RFM69_REG_SYNCVALUE3, ubNetID); // RegSyncValue3: Network ID
		rfm69_write_register(RFM69_REG_PACKETCONFIG1, RFM69_RF_PACKET1_FORMAT_VARIABLE | RFM69_RF_PACKET1_DCFREE_WHITENING | RFM69_RF_PACKET1_CRC_ON); // RegPacketConfig1: Variable length, CRC on, whitening, Address match off
		rfm69_write_register(RFM69_REG_PAYLOADLENGTH, 0x41); // RegPayloadLength: 65 bytes max payload (length byte)
		rfm69_write_register(RFM69_REG_NODEADRS, 0x00); // RegNodeAdrs: Node Address (Not used)
		rfm69_write_register(RFM69_REG_BROADCASTADRS, 0xFF); // RegBroadcastAdrs: Broadcast Address
		rfm69_write_register(RFM69_REG_FIFOTHRESH, RFM69_RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | 0x0F); // RegFifoThresh: TxStart on FifoNotEmpty, 15 bytes FifoLevel
		rfm69_write_register(RFM69_REG_PACKETCONFIG2, RFM69_RF_PACKET2_RXRESTARTDELAY_2BITS | RFM69_RF_PACKET2_AUTORXRESTART_ON | (pvEncKey == 0 ? RFM69_RF_PACKET2_AES_OFF : RFM69_RF_PACKET2_AES_ON)); // RegPacketConfig2: RX restart delay exp = 2, Auto RX restart, AES on/off
		rfm69_write_register(RFM69_REG_TESTDAGC, RFM69_RF_DAGC_IMPROVED_LOWBETA0); // RegTestDagc: Recommended value
		rfm69_write_register(RFM69_REG_TESTAFC, 0x03); // RegTestAfc: Recommended AFC offset 10% Fdev. Unit: 488 Hz (AfcOffset > DCCFreqAfc)

		if(pvEncKey != 0)
		{
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
				RFM69_SELECT();

				usart3_spi_transfer_byte(RFM69_REG_AESKEY1 | 0x80);
				usart3_spi_write((uint8_t*)pvEncKey, 16, 1);

				RFM69_UNSELECT();
			}
		}

		while(!(rfm69_read_register(RFM69_REG_IRQFLAGS1) & RFM69_RF_IRQFLAGS1_MODEREADY)); // Wait for ModeReady

		rfm69_write_register(RFM69_REG_OSC1, RFM69_RF_OSC1_RCCAL_START); // Start RC calibration

		while(!(rfm69_read_register(RFM69_REG_OSC1) & RFM69_RF_OSC1_RCCAL_DONE)); // Wait for RC calibration

		rfm69_clear_fifo();

		return 1;
	}

	return 0;
}
void rfm69_isr()
{
	uint8_t ubIRQFlags = rfm69_read_register(RFM69_REG_IRQFLAGS2);

	if(ubIRQFlags & RFM69_RF_IRQFLAGS2_PAYLOADREADY) // PayloadReady
	{
		rfm69_set_mode(RFM69_RF_OPMODE_STANDBY); // Standby

		int8_t bRSSI = rfm69_read_rssi();

		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			RFM69_SELECT();

			usart3_spi_transfer_byte(RFM69_REG_FIFO & 0x7F);

			uint8_t ubPayloadLength = usart3_spi_transfer_byte(0);

			if(ubPayloadLength >= RFM69_PACKET_HEADER_SIZE && ubPayloadLength <= RFM69_MAX_PAYLOAD_SIZE)
			{
				uint8_t pubBuffer[RFM69_MAX_PAYLOAD_SIZE + 1];

				pubBuffer[0] = (uint8_t)bRSSI;

				usart3_spi_read(pubBuffer + 1, ubPayloadLength, 0);

				blob_fifo_write(pRadioRXPacketFIFO, pubBuffer, ubPayloadLength + 1);
			}
			else
			{
				while(ubPayloadLength--)
					usart3_spi_transfer_byte(0);
			}

			RFM69_UNSELECT();
		}

		rfm69_set_mode(RFM69_RF_OPMODE_RECEIVER); // RX
	}
}
void rfm69_tick()
{
	uint8_t pubTXBuffer[RFM69_MAX_PAYLOAD_SIZE];
	uint8_t pubRXBuffer[RFM69_MAX_PAYLOAD_SIZE + 1];

	if(ubRadioCurrentMode == RFM69_RF_OPMODE_STANDBY)
		rfm69_set_mode(RFM69_RF_OPMODE_RECEIVER); // RX

	for(rfm69_pending_packet_t *pPacket = pRadioRELPending; pPacket; pPacket = pPacket->pNext)
	{
		if(pPacket->ubInTX)
			continue;

		if(g_ullSystemTick - pPacket->ullLastRetry < pPacket->usRetryDelay)
			continue;

		uint8_t ubPayloadSize;

		if(!rfm69_build_pending_packet_payload(pPacket, pubTXBuffer, RFM69_MAX_PAYLOAD_SIZE, &ubPayloadSize))
			continue;

		if(!blob_fifo_write(pRadioTXPacketFIFO, pubTXBuffer, ubPayloadSize))
			continue;

		pPacket->ubInTX = 1;
	}

	for(rfm69_pending_packet_t *pPacket = pRadioRELACKPending; pPacket; pPacket = pPacket->pNext)
	{
		if(pPacket->ubInTX)
			continue;

		if(g_ullSystemTick - pPacket->ullLastRetry < pPacket->usRetryDelay)
			continue;

		uint8_t ubPayloadSize;

		if(!rfm69_build_pending_packet_payload(pPacket, pubTXBuffer, RFM69_MAX_PAYLOAD_SIZE, &ubPayloadSize))
			continue;

		if(!blob_fifo_write(pRadioTXPacketFIFO, pubTXBuffer, ubPayloadSize))
			continue;

		pPacket->ubInTX = 1;
	}

	for(rfm69_pending_packet_t *pPacket = pRadioACKPending; pPacket; pPacket = pPacket->pNext)
	{
		if(pPacket->ubInTX)
			continue;

		if(g_ullSystemTick - pPacket->ullLastRetry < pPacket->usRetryDelay)
			continue;

		uint8_t ubPayloadSize;

		if(!rfm69_build_pending_packet_payload(pPacket, pubTXBuffer, RFM69_MAX_PAYLOAD_SIZE, &ubPayloadSize))
			continue;

		if(!blob_fifo_write(pRadioTXPacketFIFO, pubTXBuffer, ubPayloadSize))
			continue;

		pPacket->ubInTX = 1;
	}

	if(!blob_fifo_is_empty(pRadioTXPacketFIFO) && g_ullSystemTick - ullLastTX >= 5)
	{
		ullLastTX = g_ullSystemTick;

		if(rfm69_read_rssi() < RFM69_CHANNEL_FREE_RSSI)
		{
			uint32_t ulBufferSize;

			if(blob_fifo_read(pRadioTXPacketFIFO, pubTXBuffer, &ulBufferSize, RFM69_MAX_PAYLOAD_SIZE))
			{
				rfm69_packet_header_t *pHeader = (rfm69_packet_header_t *)malloc(sizeof(rfm69_packet_header_t));

				if(pHeader)
				{
					if(rfm69_unpack_header(pHeader, pubTXBuffer, RFM69_PACKET_HEADER_SIZE))
					{
						rfm69_set_mode(RFM69_RF_OPMODE_STANDBY); // Standby

						while (!(rfm69_read_register(RFM69_REG_IRQFLAGS1) & RFM69_RF_IRQFLAGS1_MODEREADY)); // Wait for ModeReady

						rfm69_clear_fifo();

						ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
						{
							RFM69_SELECT();

							usart3_spi_transfer_byte(RFM69_REG_FIFO | 0x80);
							usart3_spi_transfer_byte((uint8_t)ulBufferSize);
							usart3_spi_write(pubTXBuffer, ulBufferSize, 1);

							RFM69_UNSELECT();
						}

						bRadioCurrentPowerLevel = pbRadioATCPowerLevel[pHeader->ubReceiverNodeID]; // Set the power needed to this target node ID

						rfm69_set_mode(RFM69_RF_OPMODE_TRANSMITTER); // TX (Send the packet)
						while (!(rfm69_read_register(RFM69_REG_IRQFLAGS2) & RFM69_RF_IRQFLAGS2_PACKETSENT)); // Wait for PacketSent
						rfm69_set_mode(RFM69_RF_OPMODE_RECEIVER); // RX

						rfm69_pending_packet_t *pPendingPacket = NULL;
						rfm69_pending_packet_t **ppPendingList = NULL;

						if(pHeader->ubACKRequested)
							ppPendingList = &pRadioACKPending;
						else if(pHeader->ubRELRequested && pHeader->ubQoSLevel == 1)
							ppPendingList = &pRadioRELACKPending;
						else if(pHeader->ubACKSent && pHeader->ubQoSLevel == 1)
							ppPendingList = &pRadioRELPending;

						if(ppPendingList)
							pPendingPacket = rfm69_find_pending_packet(*ppPendingList, pHeader->usID, pHeader->ubReceiverNodeID);

						if(pPendingPacket)
						{
							if(pPendingPacket->ullLastRetry && pbRadioATCPowerLevel[pHeader->ubReceiverNodeID] < RFM69_MAXIMUM_TX_POWER)
								pbRadioATCPowerLevel[pHeader->ubReceiverNodeID]++; // Increase the power if it is not the first try

							if(!pPendingPacket->usRetriesLeft)
							{
								rfm69_remove_pending_packet(ppPendingList, pPendingPacket);

								if(pfRadioTimeoutCallback)
									pfRadioTimeoutCallback(pHeader->usID);
							}
							else
							{
								pPendingPacket->usRetriesLeft--;
								pPendingPacket->ullLastRetry = g_ullSystemTick;
								pPendingPacket->ubInTX = 0;

								if(pfRadioTXCallback)
									pfRadioTXCallback(pHeader->usID, pPendingPacket->usRetriesLeft);
							}
						}
						else
						{
							if(pfRadioTXCallback)
								pfRadioTXCallback(pHeader->usID, 0);
						}
					}

					free(pHeader);
				}
			}
		}
		else
		{
			rfm69_rmw_register(RFM69_REG_PACKETCONFIG2, 0xFB, RFM69_RF_PACKET2_RXRESTART); // Restart RX (WAIT mode to setup new gain through the AGC)
		}
	}
	else
	{
		rfm69_set_mode(RFM69_RF_OPMODE_RECEIVER); // RX
	}

	if(!blob_fifo_is_empty(pRadioRXPacketFIFO))
	{
		uint32_t ulBufferSize = 0;

		if(blob_fifo_read(pRadioRXPacketFIFO, pubRXBuffer, &ulBufferSize, RFM69_MAX_PAYLOAD_SIZE + 1))
		{
			int8_t bRSSI = (int8_t)pubRXBuffer[0];
			rfm69_packet_header_t *pHeader = (rfm69_packet_header_t *)malloc(sizeof(rfm69_packet_header_t));
			uint8_t *pubData = NULL;
			uint8_t ubDataSize = 0;

			if(pHeader)
			{
				if(rfm69_parse_payload(pHeader, &pubData, &ubDataSize, pubRXBuffer + 1, ulBufferSize - 1))
				{
					pbRadioLastRSSI[pHeader->ubSenderNodeID] = bRSSI;

					if(pHeader->ubReceiverNodeID == ubRadioNodeID)
					{
						// ATC - Power Control to target a constant RSSI at the receiver
						if(pHeader->bRemoteRSSI > -128)
						{
							pbRadioATCRemoteRSSI[pHeader->ubSenderNodeID] = pHeader->bRemoteRSSI;

							if(pHeader->bRemoteRSSI > pbRadioATCTargetRemoteRSSI[pHeader->ubSenderNodeID] && pbRadioATCPowerLevel[pHeader->ubSenderNodeID] > RFM69_MINIMUM_TX_POWER)
								pbRadioATCPowerLevel[pHeader->ubSenderNodeID]--;
							else if(pHeader->bRemoteRSSI < pbRadioATCTargetRemoteRSSI[pHeader->ubSenderNodeID] && pbRadioATCPowerLevel[pHeader->ubSenderNodeID] < RFM69_MAXIMUM_TX_POWER)
								pbRadioATCPowerLevel[pHeader->ubSenderNodeID]++;
						}

						if(pfRadioRXCallback && pubData && ubDataSize > 0 && !rfm69_find_pending_packet(pRadioRELPending, pHeader->usID, pHeader->ubSenderNodeID))
							pfRadioRXCallback(pHeader, bRSSI, pubData, ubDataSize);

						if(pHeader->ubACKSent)
						{
							// Sending ACK on QoS level 1 (bit cleared) means the packet is delivered
							// Sending ACK on QoS level 2 (bit set) means the packet is delivered and we should request a release

							// Check if pending and remove
							rfm69_pending_packet_t *pPendingPacket;

							pPendingPacket = rfm69_find_pending_packet(pRadioACKPending, pHeader->usID, pHeader->ubSenderNodeID);

							if(pPendingPacket)
							{
								rfm69_remove_pending_packet(&pRadioACKPending, pPendingPacket);

								if(pHeader->ubQoSLevel == 0 && pfRadioACKCallback)
									pfRadioACKCallback(pHeader->usID);
								else if(pHeader->ubQoSLevel == 1)
								{
									// Build response header
									rfm69_packet_header_t sHeader;

									memcpy(&sHeader, pHeader, sizeof(rfm69_packet_header_t));

									sHeader.ubACKRequested = 0;
									sHeader.ubACKSent = 0;
									sHeader.ubRELRequested = 1;
									sHeader.ubRELSent = 0;
									sHeader.bRemoteRSSI = pbRadioLastRSSI[pHeader->ubSenderNodeID];
									sHeader.ubReceiverNodeID = pHeader->ubSenderNodeID;
									sHeader.ubSenderNodeID = ubRadioNodeID;

									pPendingPacket = rfm69_find_pending_packet(pRadioRELACKPending, pHeader->usID, pHeader->ubSenderNodeID);

									if(!pPendingPacket) // Add to pending if we dont already have it there
										rfm69_add_pending_packet(&pRadioRELACKPending, &sHeader, NULL, 0, 250, 80);
									else // Otherwise just update the header
										memcpy(&pPendingPacket->sHeader, &sHeader, sizeof(rfm69_packet_header_t));
								}
							}
						}
						else if(pHeader->ubRELSent && pHeader->ubQoSLevel == 1)
						{
							// Sending REL on QoS level 2 (bit set) means the packet is released, on QoS level 1 (bit cleared) is forbidden

							// Check if pending and remove
							rfm69_pending_packet_t *pPendingPacket = rfm69_find_pending_packet(pRadioRELACKPending, pHeader->usID, pHeader->ubSenderNodeID);

							if(pPendingPacket)
							{
								rfm69_remove_pending_packet(&pRadioRELACKPending, pPendingPacket);

								if(pfRadioACKCallback)
									pfRadioACKCallback(pHeader->usID);
							}
						}
						else if(pHeader->ubRELRequested && pHeader->ubQoSLevel == 1)
						{
							// Requesting REL on QoS level 2 (bit set) means the packet should released and a REL sent, on QoS level 1 (bit cleared) is forbidden

							// Check if pending and remove
							rfm69_pending_packet_t *pPendingPacket = rfm69_find_pending_packet(pRadioRELPending, pHeader->usID, pHeader->ubSenderNodeID);

							if(pPendingPacket)
								rfm69_remove_pending_packet(&pRadioRELPending, pPendingPacket);

							// Build response header
							rfm69_packet_header_t sHeader;

							memcpy(&sHeader, pHeader, sizeof(rfm69_packet_header_t));

							sHeader.ubACKRequested = 0;
							sHeader.ubACKSent = 0;
							sHeader.ubRELRequested = 0;
							sHeader.ubRELSent = 1;
							sHeader.bRemoteRSSI = pbRadioLastRSSI[pHeader->ubSenderNodeID];
							sHeader.ubReceiverNodeID = pHeader->ubSenderNodeID;
							sHeader.ubSenderNodeID = ubRadioNodeID;

							uint8_t ubPayloadSize;

							if(rfm69_build_payload(&sHeader, NULL, 0, pubTXBuffer, RFM69_MAX_PAYLOAD_SIZE, &ubPayloadSize))
								blob_fifo_write(pRadioTXPacketFIFO, pubTXBuffer, ubPayloadSize);
						}
						else if(pHeader->ubACKRequested)
						{
							// Requesting ACK on QoS level 1 (bit cleared) means the packet is delivered
							// Requesting ACK on QoS level 2 (bit set) means the packet is delivered but we should wait for a release request

							// Build response header
							rfm69_packet_header_t sHeader;

							memcpy(&sHeader, pHeader, sizeof(rfm69_packet_header_t));

							sHeader.ubACKRequested = 0;
							sHeader.ubACKSent = 1;
							sHeader.ubRELRequested = 0;
							sHeader.ubRELSent = 0;
							sHeader.bRemoteRSSI = pbRadioLastRSSI[pHeader->ubSenderNodeID];
							sHeader.ubReceiverNodeID = pHeader->ubSenderNodeID;
							sHeader.ubSenderNodeID = ubRadioNodeID;

							if(pHeader->ubQoSLevel == 1)
							{
								rfm69_pending_packet_t *pPendingPacket = rfm69_find_pending_packet(pRadioRELPending, pHeader->usID, pHeader->ubSenderNodeID);

								if(!pPendingPacket) // Add to pending if we dont already have it there
									rfm69_add_pending_packet(&pRadioRELPending, &sHeader, NULL, 0, 250, 80);
								else // Otherwise just update the header
									memcpy(&pPendingPacket->sHeader, &sHeader, sizeof(rfm69_packet_header_t));
							}
							else
							{
								uint8_t ubPayloadSize;

								if(rfm69_build_payload(&sHeader, NULL, 0, pubTXBuffer, RFM69_MAX_PAYLOAD_SIZE, &ubPayloadSize))
									blob_fifo_write(pRadioTXPacketFIFO, pubTXBuffer, ubPayloadSize);
							}
						}
					}
					else
					{
						if(pfRadioRXCallback && pubData && ubDataSize > 0)
							pfRadioRXCallback(pHeader, bRSSI, pubData, ubDataSize);
					}
				}

				free(pHeader);
			}
		}
	}
}

void rfm69_set_timeout_callback(rfm69_timeout_callback_fn_t pfFunc)
{
	pfRadioTimeoutCallback = pfFunc;
}
void rfm69_set_tx_callback(rfm69_tx_callback_fn_t pfFunc)
{
	pfRadioTXCallback = pfFunc;
}
void rfm69_set_ack_callback(rfm69_ack_callback_fn_t pfFunc)
{
	pfRadioACKCallback = pfFunc;
}
void rfm69_set_rx_callback(rfm69_rx_callback_fn_t pfFunc)
{
	pfRadioRXCallback = pfFunc;
}

uint16_t rfm69_send(uint8_t ubReceiver, const void *pvPayload, uint8_t ubSize, uint8_t ubQoSLevel, uint16_t usRetryDelay, uint16_t usRetries)
{
	if(ubReceiver == ubRadioNodeID)
		return 0;

	if(ubSize > RFM69_MAX_DATA_SIZE)
		return 0;

	if(ubQoSLevel > 2)
		return 0;

	rfm69_packet_header_t sHeader;

	sHeader.ubACKRequested = !!ubQoSLevel;
	sHeader.ubACKSent = 0;
	sHeader.ubRELRequested = 0;
	sHeader.ubRELSent = 0;
	sHeader.ubQoSLevel = ubQoSLevel - 1;
	sHeader.usID = rfm69_get_next_packet_id();
	sHeader.bRemoteRSSI = pbRadioLastRSSI[ubReceiver];
	sHeader.ubReceiverNodeID = ubReceiver;
	sHeader.ubSenderNodeID = ubRadioNodeID;

	if(!ubQoSLevel)
	{
		uint8_t pubBuffer[RFM69_MAX_PAYLOAD_SIZE];
		uint8_t ubPayloadSize;

		if(!rfm69_build_payload(&sHeader, pvPayload, ubSize, pubBuffer, RFM69_MAX_PAYLOAD_SIZE, &ubPayloadSize))
			return 0;

		if(!blob_fifo_write(pRadioTXPacketFIFO, pubBuffer, ubPayloadSize))
			return 0;
	}
	else
	{
		if(!rfm69_add_pending_packet(&pRadioACKPending, &sHeader, pvPayload, ubSize, usRetryDelay, usRetries))
			return 0;
	}

	return sHeader.usID;
}

uint32_t rfm69_get_rx_bandwidth()
{
	uint8_t ubReg = rfm69_read_register(RFM69_REG_RXBW);

	switch(ubReg & 0x18)
	{
		case 0x00:
			return (uint32_t)(32000000.f / (16.f * (1 << ((ubReg & 0x07) + 2))));
		case 0x08:
			return (uint32_t)(32000000.f / (20.f * (1 << ((ubReg & 0x07) + 2))));
		case 0x10:
			return (uint32_t)(32000000.f / (24.f * (1 << ((ubReg & 0x07) + 2))));
	}
}
void rfm69_set_carrier(uint32_t ulCarrier)
{
	rfm69_set_mode(RFM69_RF_OPMODE_STANDBY); // Standby

	ulCarrier /= 61.03515625;

	if(ulCarrier >= (1 << 24))
		return;

	rfm69_write_register(RFM69_REG_FRFMSB, ulCarrier >> 16);
	rfm69_write_register(RFM69_REG_FRFMID, ulCarrier >> 8);
	rfm69_write_register(RFM69_REG_FRFLSB, ulCarrier);

	rfm69_set_mode(RFM69_RF_OPMODE_SYNTHESIZER); // Frequency Synthetizer

	while (!(rfm69_read_register(RFM69_REG_IRQFLAGS1) & RFM69_RF_IRQFLAGS1_PLLLOCK)); // Wait for PllLock

	rfm69_set_mode(RFM69_RF_OPMODE_STANDBY); // Standby
}
uint32_t rfm69_get_carrier()
{
	uint32_t ulCarrier = ((uint32_t)rfm69_read_register(RFM69_REG_FRFMSB) << 16) | ((uint32_t)rfm69_read_register(RFM69_REG_FRFMID) << 8) | (uint32_t)rfm69_read_register(RFM69_REG_FRFLSB);

	ulCarrier *= 61.03515625;

	return ulCarrier;
}
void rfm69_set_deviation(uint32_t ulDeviation)
{
	rfm69_set_mode(RFM69_RF_OPMODE_STANDBY); // Standby

	ulDeviation /= 61.03515625;

	if(ulDeviation >= (1 << 16))
		return;

	rfm69_write_register(RFM69_REG_FDEVMSB, ulDeviation >> 8);
	rfm69_write_register(RFM69_REG_FDEVLSB, ulDeviation);

	rfm69_set_mode(RFM69_RF_OPMODE_SYNTHESIZER); // Frequency Synthetizer

	while (!(rfm69_read_register(RFM69_REG_IRQFLAGS1) & RFM69_RF_IRQFLAGS1_PLLLOCK)); // Wait for PllLock

	rfm69_set_mode(RFM69_RF_OPMODE_STANDBY); // Standby
}
uint32_t rfm69_get_deviation()
{
	uint32_t ulDeviation = ((uint32_t)rfm69_read_register(RFM69_REG_FDEVMSB) << 8) | (uint32_t)rfm69_read_register(RFM69_REG_FDEVLSB);

	ulDeviation *= 61.03515625;

	return ulDeviation;
}
void rfm69_set_bit_rate(uint32_t ulBitRate)
{
	rfm69_set_mode(RFM69_RF_OPMODE_STANDBY); // Standby

	ulBitRate = 32000000.f / ulBitRate;

	rfm69_write_register(RFM69_REG_BITRATEMSB, ulBitRate >> 8);
	rfm69_write_register(RFM69_REG_BITRATELSB, ulBitRate);
}
uint32_t rfm69_get_bit_rate()
{
	uint32_t ulBitRate = ((uint32_t)rfm69_read_register(RFM69_REG_BITRATEMSB) << 8) | (uint32_t)rfm69_read_register(RFM69_REG_BITRATELSB);

	return (uint32_t)(32000000.f / ulBitRate);
}
uint32_t rfm69_get_freq_error()
{
	uint32_t ulError = ((uint32_t)rfm69_read_register(RFM69_REG_FEIMSB) << 8) | (uint32_t)rfm69_read_register(RFM69_REG_FEILSB);

	ulError *= 61.03515625;

	return ulError;
}
uint32_t rfm69_get_freq_correction()
{
	uint32_t ulCorrection = ((uint32_t)rfm69_read_register(RFM69_REG_AFCMSB) << 8) | (uint32_t)rfm69_read_register(RFM69_REG_AFCLSB);

	ulCorrection *= 61.03515625;

	return ulCorrection;
}

void rfm69_set_aes_key(const void *pvEncKey)
{
	if(!pvEncKey)
	{
		rfm69_rmw_register(RFM69_REG_PACKETCONFIG2, 0xFE, RFM69_RF_PACKET2_AES_OFF);
	}
	else
	{
		rfm69_rmw_register(RFM69_REG_PACKETCONFIG2, 0xFE, RFM69_RF_PACKET2_AES_ON);

		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			RFM69_SELECT();

			usart3_spi_transfer_byte(RFM69_REG_AESKEY1 | 0x80);
			usart3_spi_write((uint8_t*)pvEncKey, 16, 1);

			RFM69_UNSELECT();
		}
	}
}
void rfm69_set_network_id(uint8_t ubNetID)
{
	rfm69_write_register(RFM69_REG_SYNCVALUE3, ubNetID);
}

void rfm69_listen_mode()
{
	rfm69_set_mode(RFM69_RF_OPMODE_STANDBY); // Standby

	rfm69_write_register(RFM69_REG_DIOMAPPING1, RFM69_RF_DIOMAPPING1_DIO0_01); // Make sure we will get the PayloadReady interrupt
	rfm69_write_register(RFM69_REG_RSSITHRESH, -(RFM69_LISTEN_RX_SENSITIVITY) << 1); // Change the sensitivity
	rfm69_rmw_register(RFM69_REG_OPMODE, 0xA3, RFM69_RF_OPMODE_LISTEN_ON | RFM69_RF_OPMODE_STANDBY); // Enable listen mode, set standby as next mode
}

void rfm69_set_power_level(int8_t bPowerLevel)
{
	bPowerLevel = (bPowerLevel < RFM69_MINIMUM_TX_POWER) ? RFM69_MINIMUM_TX_POWER : ((bPowerLevel > RFM69_MAXIMUM_TX_POWER) ? RFM69_MAXIMUM_TX_POWER : bPowerLevel);

	if(bPowerLevel <= 13)
	{
		rfm69_write_register(RFM69_REG_PALEVEL, RFM69_RF_PALEVEL_PA1_ON | (bPowerLevel + 18));
		rfm69_write_register(RFM69_REG_TESTPA1, 0x55); // Disable PA1 boost
		rfm69_write_register(RFM69_REG_TESTPA2, 0x70); // Disable PA2 boost
	}
	else if(bPowerLevel <= 17)
	{
		rfm69_write_register(RFM69_REG_PALEVEL, RFM69_RF_PALEVEL_PA1_ON | RFM69_RF_PALEVEL_PA2_ON | (bPowerLevel + 14));
		rfm69_write_register(RFM69_REG_TESTPA1, 0x55); // Disable PA1 boost
		rfm69_write_register(RFM69_REG_TESTPA2, 0x70); // Disable PA2 boost
	}
	else
	{
		rfm69_write_register(RFM69_REG_PALEVEL, RFM69_RF_PALEVEL_PA1_ON | RFM69_RF_PALEVEL_PA2_ON | (bPowerLevel + 11));
		rfm69_write_register(RFM69_REG_TESTPA1, 0x5D); // Enable PA1 boost
		rfm69_write_register(RFM69_REG_TESTPA2, 0x7C); // Enable PA2 boost
	}
}

int8_t rfm69_get_atc_power_level(uint8_t ubNodeID)
{
	return pbRadioATCPowerLevel[ubNodeID];
}
void rfm69_set_atc_target_rssi(uint8_t ubNodeID, int8_t bRSSI)
{
    pbRadioATCTargetRemoteRSSI[ubNodeID] = bRSSI;
}

void rfm69_set_mode(uint8_t ubMode)
{
	uint8_t ubListenOn = rfm69_read_register(RFM69_REG_OPMODE) & RFM69_RF_OPMODE_LISTEN_ON;

	if(ubRadioCurrentMode != ubMode || ubListenOn)
	{
		if(ubListenOn)
		{
			rfm69_rmw_register(RFM69_REG_OPMODE, 0x83, RFM69_RF_OPMODE_LISTEN_OFF | RFM69_RF_OPMODE_LISTENABORT | ubMode); // Disable listen mode, abort listen mode, set desired mode
			rfm69_rmw_register(RFM69_REG_OPMODE, 0x83, RFM69_RF_OPMODE_LISTEN_OFF | ubMode); // Disable listen mode, disable abort listen mode, set desired mode
			rfm69_write_register(RFM69_REG_RSSITHRESH, -(RFM69_NORMAL_RX_SENSITIVITY) << 1); // Change the sensitivity back to normal
		}
		else
		{
			rfm69_rmw_register(RFM69_REG_OPMODE, 0xE3, ubMode);
		}

		if(ubMode == RFM69_RF_OPMODE_TRANSMITTER)
			rfm69_set_power_level(bRadioCurrentPowerLevel);
		else
			rfm69_set_power_level(RFM69_MINIMUM_TX_POWER);

		if(ubRadioCurrentMode == RFM69_RF_OPMODE_SLEEP)
			while(!(rfm69_read_register(RFM69_REG_IRQFLAGS1) & RFM69_RF_IRQFLAGS1_MODEREADY));

		ubRadioCurrentMode = ubMode;
	}
}
int8_t rfm69_read_rssi()
{
	return -rfm69_read_register(RFM69_REG_RSSIVALUE) >> 1;
}
uint8_t rfm69_read_temperature(uint8_t ubCalFactor)
{
	rfm69_set_mode(RFM69_RF_OPMODE_STANDBY); // Standby
	rfm69_write_register(RFM69_REG_TEMP1, RFM69_RF_TEMP1_MEAS_START);

	while ((rfm69_read_register(RFM69_REG_TEMP1) & RFM69_RF_TEMP1_MEAS_RUNNING));

	return ~rfm69_read_register(RFM69_REG_TEMP2) + 90 + ubCalFactor;
}
void rfm69_clear_fifo()
{
	rfm69_write_register(RFM69_REG_IRQFLAGS2, RFM69_RF_IRQFLAGS2_FIFOOVERRUN);
}
