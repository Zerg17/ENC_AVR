#ifndef __Network_H__
#define __Network_H__

#include <stdint.h>
#include "config.h"

// Длинна или позиция байт разных протоколов
// Ethernet протокол
#define ETH_SIZE		14 // Общая длинна параметров
#define ETH_DST			0  // 6 байт. MAC адрес назначения 
#define ETH_SRC			6  // 6 байт. MAC адрес источника
#define ETH_TYPE		12 // 2 байт. Тип интернета
// Уникальные идентификаторы Интернета
#define ETHTYPE_IP		0x0800 // Пакет IP v.4  
#define ETHTYPE_ARP		0x0806 // Пакет ARP 

// ARP протокол
#define	ARP_SIZE		28 // Общая длинна параметров 
#define	ARP_HTYPE		0  // 2 байт. Номер канального протокола
#define ARP_PTYPE		2  // 2 байт. Код сетевого протокола
#define ARP_HLEN		4  // 1 байт. Длина физ адреса(для интернета 6 байт)
#define ARP_PLEN		5  // 1 байт. Длина логич адреса(IP v.4 = 4 байт)
#define ARP_OPER		6  // 2 байт. Код операции отправителя(0001 - запрос; 0002 - ответ)
#define ARP_SHA			8  // 6 байт. MAC адрес отправителя
#define ARP_SPA			14 // 4 байт. IP адрес отправителя
#define	ARP_THA			18 // 6 байт. MAC адрес получателя
#define ARP_TPA			24 // 4 байт. IP адрес получателя

#define ARP_REQUEST		1  // ЭХО запрос
#define ARP_REPLY		2  // ЭХО ответ


// IP протокол
#define IP_SIZE			20 // Общая длинна параметров (данные не учитываются)
#define IP_VER_HLEN		0  // 1 байт. Версия протокола(4 бит) + Длинна заголовка IP пакета 
#define IP_TOS			1  // 1 байт. DSCP + ECN
#define IP_LENGTH		2  // 2 байт. Длина пакета (заголовок + данные)
#define IP_ID			4  // 2 байт. Идентификатор(При фрагментир пакет одинаковое значение)
#define IP_FLAGS_FO		6  // 2 байт. 3 бита флагов + Смещение фрагмента
#define IP_TTL			8  // 1 байт. TTL, сколько раз макс пройдем через маршрутизаторы
#define IP_PROTO		9  // 1 байт. Идентификатор протокола(TCP,UDP,ICMP)
#define IP_HCS			10 // 2 байт. Контрольная сумма заголовка
#define IP_SRC			12 // 4 байт. IP адрес отправителя
#define IP_DST			16 // 4 байт. IP адрес получателя
// Уникальные идентификаторы для параметра IP_PROTO
#define IP_PROTO_ICMP	1  // ICMP протокол
#define IP_PROTO_TCP	6  // TCP  протокол
#define IP_PROTO_UDP	17 // UDP  протокол

// UDP протокол
#define UDP_SIZE        8  // Общая длинна параметров (данные не учитываются)
#define UDP_SRCPORT		0  // 2 байт. Порт источника, то есть при отсылке наш
#define UDP_DSTPORT		2  // 2 байт. Порт назначения, то есть на чей шлём
#define UDP_LENGTH		4  // 2 байт. Длина пакета (заголовок + данные)
#define UDP_HCS			6  // 2 байт. Контрольная сумма заголовка

// ICMP пакет
#define ICMP_SIZE		4 // Общая длинна параметров 
#define ICMP_TYPE		0 // 1 байт. Тип
#define ICMP_CODE		1 // 1 байт. Код
#define ICMP_CHECKSUM	2 // 2 байт. Контрольная сумма
// Уникальные идентификаторы типов ICMP пакета
#define ICMP_ECHO_REQ	8 // Эхо-запрос
#define ICMP_ECHO_REPLY	0 // Эхо-ответ

// TCP протокол
#define TCP_SIZE		20 // Общая длинна параметров (данные не учитываются)
#define TCP_SRCPORT		0  // 2 байт. Порт источника, то есть при отсылке наш
#define TCP_DSTPORT		2  // 2 байт. Порт назначения, то есть на чей шлём 
#define TCP_SEQNUM		4  // 4 байт. Порядковый номер пакета
#define TCP_ACKNUM		8  // 4 байт. Номер подтверждения
#define TCP_DO			12 // 1 байт.(первая тетрада) Длина заголовка (4 байтные слова)
#define TCP_FLAGS		13 // 1 байт. Флаги(смотреть в блоке ниже)
#define TCP_WINDOW		14 // 2 байт. Размер окна 
#define TCP_CHECKSUM	16 // 2 байт. Контрольная сумма
#define TCP_URGENT		18 // 2 байт. Указатель важности
#define TCP_OPTIONS		20 // 4 байт. Опции (необязательное поле)
// Флаги TCP протокола
#define FIN				0 // Указывает на завершение соединения
#define SYN				1 // Синхронизация номеров последовательности
#define	RST				2 // Обрываем соединение, очистка буфера
#define	PSH				3 // Инструктирует получателя протолкнуть приемный буфер
#define ACK				4 // Подтверждение
#define URG				5 // Указатель важности
#define ECE				6 // Возможность принимать эхо пакеты
#define CWR				7 // Устанавливается отправителем, что принят пакет с ECE
///////////////////////////////////////////

#define BUF_HEAD_SIZE 1500   			 // Значение резерва для чтения заголовков пакетов 1500
#define ETH_BUF_HEAD_SIZE ETH_SIZE		 //	Значение резерва для чтения заголовков пакетов ETH
#define IP_BUF_HEAD_SIZE  IP_SIZE		 // Значение резерва для чтения заголовков пакетов IP
#define TCP_BUF_HEAD_SIZE TCP_SIZE+12	 // Значение резерва для чтения заголовков пакетов TCP
#define IP_TTLVALUE	  128				 // Значение TTL пакета (кол-во переходов через роутеры)
///////////////////////////////////////////

void QueryMACGate(void);

typedef struct{
	uint8_t MAC[6];
	uint8_t TimeOut;
	uint8_t	Flag;
} arpGate_t;

typedef struct{
	uint8_t  MAC[6];
	uint8_t	 IP[4];
	uint8_t	 GW[4];
} netSettings_t;

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
					uint16_t checksum;
					uint8_t ident[2];
					uint8_t seq_le[2];
					uint8_t data[FRAME_LENGTH-14-20-8];
				}icmp;
			};
		}ip;
	};
} net;

extern netSettings_t netSettings;
extern arpGate_t arpGate;

void packetReceive(void);

#endif
