#include "system.h"
#include "enc.h"
#include "net.h"
#include "config.h"

int main(void){
    uartInit();

    // recR(TIMSK1, 0b001);
    // recR(ICR1, 62500-1);
    // recR(TCCR1A, 0b00000010);
    // recR(TCCR1B, 0b00011100);

    //xprintf("Zerg17 Server\n");
    netSettings.MAC[0] = MAC_0;
    netSettings.MAC[1] = MAC_1;
    netSettings.MAC[2] = MAC_2;
    netSettings.MAC[3] = MAC_3;
    netSettings.MAC[4] = MAC_4;
    netSettings.MAC[5] = MAC_5;

    netSettings.IP[0] = Addr_IP_0;
    netSettings.IP[1] = Addr_IP_1;
    netSettings.IP[2] = Addr_IP_2;
    netSettings.IP[3] = Addr_IP_3;

    netSettings.GW[0] = Addr_Gate_0;
    netSettings.GW[1] = Addr_Gate_1;
    netSettings.GW[2] = Addr_Gate_2;
    netSettings.GW[3] = Addr_Gate_3;

    encInit();

    //sei();
    while(1){
        packetReceive();
    }
}