#ifndef _CONFIG_H_
#define _CONFIG_H_

#define BOAD 2000000

#define MAC_0        0xf8
#define MAC_1        0x1a
#define MAC_2        0x67
#define MAC_3        0x60
#define MAC_4        0x70
#define MAC_5        0x71

#define Addr_IP_0	 192
#define Addr_IP_1	 168
#define Addr_IP_2	 1
#define Addr_IP_3	 120

#define Addr_Gate_0	 192
#define Addr_Gate_1	 168
#define Addr_Gate_2	 1
#define Addr_Gate_3	 101

#define RX_START    (0x0000) // Параметр, откуда начинается буфер приема(кольцевой)
#define RX_END		(0x17ff) // Параметр, где заканчивается буфер приема(кольцевой)
#define TX_START	(0x1800) // Параметр, откуда начинается буфер передачи(кольцевой)
#define TX_END		(0x1FFF) // Параметр, где заканчивается буфер передачи(кольцевой)
#define TX_BASE		(TX_START+1)
#define MAX_ENC_CHIP_BUFF (0x1FFF)

#define FRAME_LENGTH  (1518) // Значение длины кадра(1518 длина по даташиту)

#define HTTP_PORT	80
#define HTTP_SERV_NAME "Zerg17 Server"

#endif
