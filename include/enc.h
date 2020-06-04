#ifndef __ENC_H__
#define __ENC_H__

#include <stdint.h>

void encInit(void);
uint16_t encPacketReceive(uint8_t* data, uint16_t maxlen);
void encPacketTransmit(uint8_t* data, uint16_t len);

#endif