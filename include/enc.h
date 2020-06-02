#ifndef __ENC_H__
#define __ENC_H__

#include <stdint.h>

void encInit(void);
uint16_t encPacketReceive(uint8_t* data, uint16_t maxlen);
void encWriteBuf(uint8_t *buf, uint16_t len);
void encReadBuf(uint8_t *buf, uint16_t len);
uint8_t encTestTransmitBusy(void);
uint16_t encIdentifiPacket(void);
void encTxPackInit(void);
uint16_t encGetWriteAdr(void);
void encSetWriteAdr(uint16_t adr);
void encRxPackDone(void);
void encTxPackSend(void);

#endif