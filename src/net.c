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
					uint8_t data[];
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
					uint8_t data[];
				}tcp;
				struct{
					uint8_t type;
					uint8_t code;
					uint8_t checksum[2];
					uint8_t data[];
				}icmp;
			}p;
		}ip;
	}p;
	uint8_t res[FRAME_LENGTH-14-28];
} net;

struct t_Setting_Network Setting_Network;
struct t_arpGate arpGate;

void encProcess_ARP(void);
void encProcess_IP(void);
void encProcess_ICMP(void);
void encUpdate_IP(void);
uint16_t encCalcCRC(uint8_t *adr, uint16_t LongData, uint16_t StartCRC);

uint16_t LongPacket;

void packetReceive(void){
	// Считываем принятый пакет если есть
	uint16_t packLen = encPacketReceive((uint8_t*)&net, sizeof(net));
	if(packLen==0)return;
	
	// Определяем тип пакета
	uint16_t ETHTYPE = (net.eth.type[0]<<8)|net.eth.type[1];

	if(ETHTYPE==ETHTYPE_ARP || ETHTYPE==ETHTYPE_IP){

		encTxPackInit();
		for (uint8_t i=0;i<6;i++){ // Выставляем MAC адреса приемника и источника
			net.eth.dstMAC[i] = net.eth.srcMAC[i];
			net.eth.srcMAC[i] = Setting_Network.MAC_Addr_Core[i];
		}
		encWriteBuf((uint8_t *)&net.eth, sizeof(net.eth)); // Записываем для передачи MAC приемника и источника + тип пакета

		switch (ETHTYPE){
			case ETHTYPE_ARP: encProcess_ARP(); break;
			case ETHTYPE_IP: encProcess_IP(); break;
		}
		encRxPackDone();

	}	
}

void encProcess_ARP(void){
	uint16_t ETHTYPE = (net.p.arp.ptype[0]<<8)|net.p.arp.ptype[1];
	if (ETHTYPE !=  ETHTYPE_IP) return;

	uint8_t FlagOurIp	= 0;
	uint8_t FlagOurGate = 0;

	for (uint8_t i=0; i<4; i++){
		if (net.p.arp.tpa[i] == Setting_Network.IP_Addr_Core[i]) FlagOurIp++;
		if (net.p.arp.spa[i] == Setting_Network.IP_Addr_Gate[i]) FlagOurGate++;
	}
	
	for (uint8_t i=0; i<4; i++){
		net.p.arp.tpa[i] = net.p.arp.spa[i];
		net.p.arp.spa[i] = Setting_Network.IP_Addr_Core[i];
	}

	if ((net.p.arp.oper[1] == ARP_REQUEST) & (FlagOurIp == 4)){
		net.p.arp.oper[1] = ARP_REPLY;
	
		for (uint8_t i=0; i<6; i++){
			net.p.arp.tha[i] = net.p.arp.sha[i];
			net.p.arp.sha[i] = Setting_Network.MAC_Addr_Core[i];
		}
		encWriteBuf((uint8_t *)&net.p.arp, sizeof(net.p.arp));
		encTxPackSend();
		return;
	} else if ((net.p.arp.oper[1] == ARP_REPLY) & (FlagOurGate == 4)){
		for (uint8_t i=0; i<6; i++)
			arpGate.MAC[i] = net.p.arp.sha[i];
		return;
	}
}

void encProcess_IP(void){
	
	if (net.p.ip.ver_hlen!= 0x45) return;
	
	for (uint8_t i=0; i<4; i++)
		if (net.p.ip.ip_dst[i] == Setting_Network.IP_Addr_Core[i]) {
			net.p.ip.ip_dst[i] = net.p.ip.ip_src[i];
			net.p.ip.ip_dst[i] = Setting_Network.IP_Addr_Core[i];
		} 
		else return;
	// Устанавливаем байт TOS в 0, это для DSCP + ECN
	net.p.ip.tos = 0x00;
	// Устанавливаем флаги и смещение в 0x0000
	net.p.ip.flugs_fo[0]  = 0x00;
	net.p.ip.flugs_fo[1]  = 0x00;
	// Устанавливаем TTL
	net.p.ip.ttl = IP_TTLVALUE;
	encWriteBuf((uint8_t *)&net.p.ip, sizeof(net.p.ip));
	
	LongPacket	=  (net.p.ip.lenght[0] << 8) | net.p.ip.lenght[1]; 
	LongPacket -= IP_SIZE;
	
	uint8_t ProtoIP = net.p.ip.proto;
	if (ProtoIP == IP_PROTO_ICMP) encProcess_ICMP();
	//if (ProtoIP == IP_PROTO_TCP) encProcess_TCP();
	//if (ProtoIP == IP_PROTO_UDP) encProcess_UDP();
}

void encProcess_ICMP(void){
	if (net.p.ip.p.icmp.type != ICMP_ECHO_REQ) return;
	 
	net.p.ip.p.icmp.type = ICMP_ECHO_REPLY;				// Set answer
	uint16_t CRC_ICMP = (net.p.ip.p.icmp.checksum[1] << 8) + net.p.ip.p.icmp.checksum[0];
	CRC_ICMP += 8;
	net.p.ip.p.icmp.checksum[1] = (uint8_t)(CRC_ICMP >> 8);
	net.p.ip.p.icmp.checksum[0]	 = (uint8_t)(CRC_ICMP);
	encWriteBuf((uint8_t *)&net.p.ip.p.icmp, sizeof(net.p.ip.p.icmp));
	LongPacket -= ICMP_SIZE;
	if (LongPacket == 0){
		encUpdate_IP();
		encTxPackSend();
		return;
	}
	encWriteBuf((uint8_t *)&net.p.ip.p.icmp.data, LongPacket);
	encUpdate_IP();
	encTxPackSend();
}

void encUpdate_IP(void){
	uint16_t EndAdrrPacket = encGetWriteAdr();								// Read end address
	uint16_t LengthPacket  = EndAdrrPacket - (TX_BASE+ETH_SIZE);					// Subtraction ETH Size
	net.p.ip.lenght[0] = (uint8_t)(LengthPacket >> 8);
	net.p.ip.lenght[1]	= (uint8_t)LengthPacket;
	encSetWriteAdr(TX_BASE+ETH_SIZE+IP_LENGTH);								// Set adrress length IP Packet
	encWriteBuf((uint8_t *)(net.p.ip.lenght), 2);	// Write length IP Packet
	net.p.ip.hcs[0]	= 0x00;
	net.p.ip.hcs[1] = 0x00;
	uint16_t CRC_IP = encCalcCRC((uint8_t *)(&net.p.ip),IP_SIZE,0);	// Calculate CRC
	CRC_IP = ~CRC_IP;
								
	net.p.ip.hcs[0]	= (uint8_t)(CRC_IP >> 8);
	net.p.ip.hcs[1] = (uint8_t)CRC_IP;
	encSetWriteAdr(TX_BASE+ETH_SIZE+IP_HCS);								// Set adrress CRC IP Packet
	encWriteBuf((uint8_t *)(net.p.ip.hcs),2);	// Write CRC IP Packet
	encSetWriteAdr(EndAdrrPacket);											// Set End Address Packet
}

uint16_t encCalcCRC(uint8_t *adr, uint16_t len, uint16_t crc16){
	uint32_t crc = crc16;
	for (uint16_t i = 0; i < len; i++)
		if (i & 1) crc += (uint32_t)adr[i];
		else crc += ((uint32_t)(adr[i]))<<8;
	crc = (uint16_t)(crc >> 16)+(uint16_t)(crc);
	return (uint16_t)(crc);
}
