#include "net.h"
#include "enc.h"
#include "config.h"
#include "xprintf.h"
#include "system.h"

netSettings_t netSettings;
arpGate_t arpGate;

uint16_t netProcessARP(void);
uint16_t netProcessIP(void);
uint16_t netProcessICMP(void);
uint16_t netProcessUDP(void);
void netUpdate_IP(void);
uint16_t netCalcCRC(uint8_t *adr, uint16_t LongData, uint16_t StartCRC);

uint16_t LongPacket;

void packetReceive(void){
	// Считываем принятый пакет если есть
	uint16_t packLen = encPacketReceive((uint8_t*)&net, sizeof(net));
	if (packLen==0) return;

	uint16_t packCount = 0;
	switch (net.eth.type){
		case ETHTYPE_ARP: packCount+=netProcessARP(); break;
		case ETHTYPE_IP:  packCount+=netProcessIP(); break;
	}

	if(packCount){
		for (uint8_t i=0;i<6;i++){ // Выставляем MAC адреса приемника и источника
			net.eth.dstMAC[i] = net.eth.srcMAC[i];
			net.eth.srcMAC[i] = netSettings.mac[i];
		}
		encPacketTransmit((uint8_t*)&net, packCount + sizeof(net.eth));
	}	
}

uint16_t netProcessARP(void){
	if (net.arp.ptype !=  ETHTYPE_IP) return 0;

	uint8_t FlagOurIp	= net.arp.tpa == netSettings.ip;
	uint8_t FlagOurGate = net.arp.spa == netSettings.gw;

	net.arp.tpa = net.arp.spa;
	net.arp.spa = netSettings.ip;

	if ((net.arp.oper == ARP_REQUEST) & FlagOurIp){
		net.arp.oper = ARP_REPLY;
	
		for (uint8_t i=0; i<6; i++){
			net.arp.tha[i] = net.arp.sha[i];
			net.arp.sha[i] = netSettings.mac[i];
		}
		return sizeof(net.arp);
	} else if ((net.arp.oper == ARP_REPLY) & FlagOurGate){
		for (uint8_t i=0; i<6; i++)
			arpGate.MAC[i] = net.arp.sha[i];
		return 0;
	}
	return 0;
}

uint16_t netProcessIP(void){
	if (net.ip.ver_hlen!= 0x45) return 0;

	if (net.ip.ip_dst == netSettings.ip){
		net.ip.ip_dst = net.ip.ip_src;
		net.ip.ip_src = netSettings.ip;
	} else return 0;
	
	// Устанавливаем байт TOS в 0, это для DSCP + ECN
	net.ip.tos = 0x00;
	// Устанавливаем флаги и смещение в 0
	net.ip.flags_fo  = 0;
	// Устанавливаем TTL
	net.ip.ttl = IP_TTLVALUE;
	uint16_t packCount = IP_SIZE;
	
	LongPacket	=  ntohs(net.ip.lenght); 
	LongPacket -= IP_SIZE;
	
	uint8_t ProtoIP = net.ip.proto;
	if (ProtoIP == IP_PROTO_ICMP) packCount+=netProcessICMP();
	//if (ProtoIP == IP_PROTO_TCP) encProcess_TCP();
	if (ProtoIP == IP_PROTO_UDP) packCount+=netProcessUDP();
	if (packCount==IP_SIZE) return 0;
	else{
		net.ip.lenght = htons(packCount);
		net.ip.hcs = 0;
		uint16_t CRC_IP = netCalcCRC((uint8_t *)(&net.ip),IP_SIZE,0);						
		net.ip.hcs = htons(CRC_IP);
		return packCount;
	}
}

uint16_t netProcessICMP(void){
	if (net.ip.icmp.type != ICMP_ECHO_REQ) return 0;
	net.ip.icmp.type = ICMP_ECHO_REPLY;
	uint16_t CRC_ICMP = net.ip.icmp.checksum;
	CRC_ICMP += 8;
	net.ip.icmp.checksum = CRC_ICMP;
	return LongPacket;
}

uint16_t netProcessUDP(void){
	uint16_t len = ((net.ip.udp.lenght[0] << 8) | net.ip.udp.lenght[1]) - 8; 

    for(uint16_t i = 0; i < len; i++)
        uartWrite(net.ip.udp.data[i]);
	return 0;
}

uint16_t netCalcCRC(uint8_t *adr, uint16_t len, uint16_t crc16){
	uint32_t crc = crc16;
	for (uint16_t i = 0; i < len; i++)
		if (i & 1) crc += (uint32_t)adr[i];
		else crc += ((uint32_t)(adr[i]))<<8;
	crc = (uint16_t)(crc >> 16)+(uint16_t)(crc);
	return ~crc;
}
