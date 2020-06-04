#include "net.h"
#include "enc.h"
#include "config.h"
#include "xprintf.h"

netSettings_t netSettings;
arpGate_t arpGate;

uint16_t netProcess_ARP(void);
uint16_t netProcess_IP(void);
uint16_t netProcess_ICMP(void);
void netUpdate_IP(void);
uint16_t netCalcCRC(uint8_t *adr, uint16_t LongData, uint16_t StartCRC);

uint16_t LongPacket;

void packetReceive(void){
	// Считываем принятый пакет если есть
	uint16_t packLen = encPacketReceive((uint8_t*)&net, sizeof(net));
	if(packLen==0)return;

	// Определяем тип пакета
	uint16_t ETHTYPE = (net.eth.type[0]<<8)|net.eth.type[1];

	if(ETHTYPE==ETHTYPE_ARP || ETHTYPE==ETHTYPE_IP){

		for (uint8_t i=0;i<6;i++){ // Выставляем MAC адреса приемника и источника
			net.eth.dstMAC[i] = net.eth.srcMAC[i];
			net.eth.srcMAC[i] = netSettings.MAC[i];
		}
		//encWriteBuf((uint8_t *)&net.eth, sizeof(net.eth)); // Записываем для передачи MAC приемника и источника + тип пакета
		uint16_t packCount = 0;
		switch (ETHTYPE){
			case ETHTYPE_ARP: packCount+=netProcess_ARP(); break;
			case ETHTYPE_IP:  packCount+=netProcess_IP(); break;
		}
		
		if(packCount) encPacketTransmit((uint8_t*)&net, packCount + sizeof(net.eth));
	}	
}

uint16_t netProcess_ARP(void){
	uint16_t ETHTYPE = (net.arp.ptype[0]<<8)|net.arp.ptype[1];
	if (ETHTYPE !=  ETHTYPE_IP) return 0;

	uint8_t FlagOurIp	= 0;
	uint8_t FlagOurGate = 0;

	for (uint8_t i=0; i<4; i++){
		if (net.arp.tpa[i] == netSettings.IP[i]) FlagOurIp++;
		if (net.arp.spa[i] == netSettings.GW[i]) FlagOurGate++;
	}
	
	for (uint8_t i=0; i<4; i++){
		net.arp.tpa[i] = net.arp.spa[i];
		net.arp.spa[i] = netSettings.IP[i];
	}

	if ((net.arp.oper[1] == ARP_REQUEST) & (FlagOurIp == 4)){
		net.arp.oper[1] = ARP_REPLY;
	
		for (uint8_t i=0; i<6; i++){
			net.arp.tha[i] = net.arp.sha[i];
			net.arp.sha[i] = netSettings.MAC[i];
		}
		return sizeof(net.arp);
	} else if ((net.arp.oper[1] == ARP_REPLY) & (FlagOurGate == 4)){
		for (uint8_t i=0; i<6; i++)
			arpGate.MAC[i] = net.arp.sha[i];
		return 0;
	}
	return 0;
}

uint16_t netProcess_IP(void){
	if (net.ip.ver_hlen!= 0x45) return 0;
	
	for (uint8_t i=0; i<4; i++)
		if (net.ip.ip_dst[i] == netSettings.IP[i]) {
			net.ip.ip_dst[i] = net.ip.ip_src[i];
			net.ip.ip_src[i] = netSettings.IP[i];
		} else return 0;
	// Устанавливаем байт TOS в 0, это для DSCP + ECN
	net.ip.tos = 0x00;
	// Устанавливаем флаги и смещение в 0x0000
	net.ip.flugs_fo[0]  = 0x00;
	net.ip.flugs_fo[1]  = 0x00;
	// Устанавливаем TTL
	net.ip.ttl = IP_TTLVALUE;
	uint16_t packCount = IP_SIZE;
	
	LongPacket	=  (net.ip.lenght[0] << 8) | net.ip.lenght[1]; 
	LongPacket -= IP_SIZE;
	
	uint8_t ProtoIP = net.ip.proto;
	if (ProtoIP == IP_PROTO_ICMP) packCount+=netProcess_ICMP();
	//if (ProtoIP == IP_PROTO_TCP) encProcess_TCP();
	//if (ProtoIP == IP_PROTO_UDP) encProcess_UDP();
	if(packCount==IP_SIZE) return 0;
	else{
		net.ip.lenght[0] = (uint8_t)(packCount >> 8);
		net.ip.lenght[1] = (uint8_t)packCount;
		net.ip.hcs[0] = 0x00;
		net.ip.hcs[1] = 0x00;
		
		uint16_t CRC_IP = netCalcCRC((uint8_t *)(&net.ip),IP_SIZE,0);						
		net.ip.hcs[0] = (uint8_t)(CRC_IP >> 8);
		net.ip.hcs[1] = (uint8_t)CRC_IP;

		return packCount;
	}
}

uint16_t netProcess_ICMP(void){
	if (net.ip.icmp.type != ICMP_ECHO_REQ) return 0;
	net.ip.icmp.type = ICMP_ECHO_REPLY;
	uint16_t CRC_ICMP = net.ip.icmp.checksum;
	CRC_ICMP += 8;
	net.ip.icmp.checksum = CRC_ICMP;
	return LongPacket;
}

uint16_t netCalcCRC(uint8_t *adr, uint16_t len, uint16_t crc16){
	uint32_t crc = crc16;
	for (uint16_t i = 0; i < len; i++)
		if (i & 1) crc += (uint32_t)adr[i];
		else crc += ((uint32_t)(adr[i]))<<8;
	crc = (uint16_t)(crc >> 16)+(uint16_t)(crc);
	return ~crc;
}
