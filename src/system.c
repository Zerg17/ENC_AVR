#include "system.h"
#include "config.h"

// uint8_t bufUI[256];
// volatile uint8_t posTX=0, posW=0;

void uartWrite(unsigned char d){
    while(!(UCSR0A & (1<<UDRE0)));
    UDR0 = d;
}

// void uartWriteISR(unsigned char d){
//     bufUI[posW++]=d;
//     while(posW==posTX);
//     setRB(UCSR0B, UDRIE0);
// }

// ISR(USART_UDRE_vect){
//     UDR0 = bufUI[posTX++];
//     if(posTX==posW)resRB(UCSR0B, UDRIE0);
// }

void uartInit(void) {
    xdev_out(uartWrite);
    recR(UBRR0, ((F_CPU/8)-BOAD/2)/BOAD);
    setRB(UCSR0A, U2X0);
    setRB(UCSR0B, TXEN0);
}

void spi_init() {
    resR(DDRB, 0b010000);
    setR(DDRB, 0b101100);
    recR(SPCR, 0b01010000);
}

uint8_t spiWR(uint8_t d) {
    SPDR = d;
    while ((SPSR & (1 << SPIF)) == 0);  //Ожидание окончание передачи
    return SPDR;
}
