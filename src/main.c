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
    Setting_Network.MAC_Addr_Core[0] = MAC_0;
    Setting_Network.MAC_Addr_Core[1] = MAC_1;
    Setting_Network.MAC_Addr_Core[2] = MAC_2;
    Setting_Network.MAC_Addr_Core[3] = MAC_3;
    Setting_Network.MAC_Addr_Core[4] = MAC_4;
    Setting_Network.MAC_Addr_Core[5] = MAC_5;

    Setting_Network.IP_Addr_Core[0] = Addr_IP_0;
    Setting_Network.IP_Addr_Core[1] = Addr_IP_1;
    Setting_Network.IP_Addr_Core[2] = Addr_IP_2;
    Setting_Network.IP_Addr_Core[3] = Addr_IP_3;

    Setting_Network.IP_Addr_Gate[0] = Addr_Gate_0;
    Setting_Network.IP_Addr_Gate[1] = Addr_Gate_1;
    Setting_Network.IP_Addr_Gate[2] = Addr_Gate_2;
    Setting_Network.IP_Addr_Gate[3] = Addr_Gate_3;

    encInit();

    //sei();
    while(1){
        packetReceive();
    }
}