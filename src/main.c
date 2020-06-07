#include "system.h"
#include "enc.h"
#include "net.h"
#include "config.h"

int main(void){
    uartInit();

    //xprintf("Zerg17 Server\n");
    encInit();
    countInit();
    sei();
    while(1){
        packetReceive();
    }
}