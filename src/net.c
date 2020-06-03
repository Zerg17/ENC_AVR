#include "net.h"
#include "enc.h"
#include "config.h"
#include "xprintf.h"

// struct{
// 	uint8_t ETH_BUF_Head[ETH_BUF_HEAD_SIZE];				// Buffer ETH Header
// 	uint8_t IP_BUF_Head[IP_BUF_HEAD_SIZE];					// Buffer IP Header
// 	uint8_t TCP_BUF_Head[TCP_BUF_HEAD_SIZE];				// Buffer TCP Header
// 	uint8_t DATA_BUF_Head[BUF_HEAD_SIZE];					// Buffer Data
// } netBuf;

struct{
	struct{
		uint8_t dstMAC[6];
		uint8_t srcMAC[6];
		uint8_t type[2];
	} eth;
	union{
		struct{
			uint8_t htype[2];
			uint8_t ptype[2];
			uint8_t hlen;
			uint8_t plen;
			uint8_t oper[2];
			uint8_t sha[6];
			uint8_t spa[4];
			uint8_t tha[6];
			uint8_t tpa[4];
		}arp;
		struct{
			uint8_t ver_hlen;
			uint8_t tos;
			uint8_t lenght[2];
			uint8_t id[2];
			uint8_t flugs_fo[2];
			uint8_t ttl;
			uint8_t proto;
			uint8_t hcs[2];
			uint8_t ip_src[4];
			uint8_t ip_dst[4];
			union{
				struct{
					uint8_t srcport[2];
					uint8_t dstport[2];
					uint8_t lenght[2];
					uint8_t hcs[2];
					uint8_t data[FRAME_LENGTH-14-20-8];
				}udp;
				struct{
					uint8_t srcport[2];
					uint8_t dstport[2];
					uint8_t seqnum[4];
					uint8_t acknum[4];
					uint8_t doo;
					uint8_t flags;
					uint8_t window[2];
					uint8_t checksum[2];
					uint8_t urgent[2];
					uint8_t data[FRAME_LENGTH-14-20-20];
				}tcp;
				struct{
					uint8_t type;
					uint8_t code;
					uint8_t checksum[2];
					uint8_t ident[2];
					uint8_t seq_le[2];
					uint8_t data[FRAME_LENGTH-14-20-8];
				}icmp;
			};
		}ip;
	};
} net;

struct t_Setting_Network Setting_Network;
struct t_arpGate arpGate;

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
			net.eth.srcMAC[i] = Setting_Network.MAC_Addr_Core[i];
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
		if (net.arp.tpa[i] == Setting_Network.IP_Addr_Core[i]) FlagOurIp++;
		if (net.arp.spa[i] == Setting_Network.IP_Addr_Gate[i]) FlagOurGate++;
	}
	
	for (uint8_t i=0; i<4; i++){
		net.arp.tpa[i] = net.arp.spa[i];
		net.arp.spa[i] = Setting_Network.IP_Addr_Core[i];
	}

	if ((net.arp.oper[1] == ARP_REQUEST) & (FlagOurIp == 4)){
		net.arp.oper[1] = ARP_REPLY;
	
		for (uint8_t i=0; i<6; i++){
			net.arp.tha[i] = net.arp.sha[i];
			net.arp.sha[i] = Setting_Network.MAC_Addr_Core[i];
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
		if (net.ip.ip_dst[i] == Setting_Network.IP_Addr_Core[i]) {
			net.ip.ip_dst[i] = net.ip.ip_src[i];
			net.ip.ip_src[i] = Setting_Network.IP_Addr_Core[i];
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
	uint16_t CRC_ICMP = (net.ip.icmp.checksum[1] << 8) + net.ip.icmp.checksum[0];
	CRC_ICMP += 8;
	net.ip.icmp.checksum[1] = (uint8_t)(CRC_ICMP >> 8);
	net.ip.icmp.checksum[0]	 = (uint8_t)(CRC_ICMP);
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
