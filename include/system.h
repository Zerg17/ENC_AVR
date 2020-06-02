#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "xprintf.h"
#include "define.h"

void uartInit(void);
void uartWrite(unsigned char d);
void spi_init();
uint8_t spiWR(uint8_t d);

#endif
