#include "enc.h"
#include "enc_reg.h"
#include "config.h"
#include "system.h"

#define ENC_SET resRB(PORTB, 2)
#define ENC_RES setRB(PORTB, 2)

struct{
    uint16_t RX_Addr;
    uint16_t RX_Packet_Length;
    uint16_t RX_Packet_Status;
} encPackedHead = {RX_START, 0x00, 0x00};

 // Мягкая перезагрузка
static inline void encSoftReset(void){
    ENC_SET;
	spiWR(CMD_RESET);
	ENC_RES;
	_delay_ms(1);
}

// Передача 2 байтов в ENC
void encDoubleWrite(uint8_t data1, uint8_t data2){
    ENC_SET;
	spiWR(data1);
    data2 = spiWR(data2);
    ENC_RES;
}

// Записать регистр
void encWriteReg(uint8_t adr, uint8_t data){
	encDoubleWrite(CMD_WCR|(adr&0x1f),data);	
}

// Записать регистр
void encWriteDudleReg(uint8_t adr, uint16_t data){
	encDoubleWrite(CMD_WCR|(adr&0x1f),data);
    encDoubleWrite(CMD_WCR|(adr&0x1f)+1,data>>8);
}

// Чтение регистра
uint8_t encReadReg(uint8_t adr){
	ENC_SET;
	spiWR(CMD_RCR|(0x1f&adr));
	uint8_t data = spiWR(0xFF);
	if ((adr>>7) & 1) data = spiWR(0xFF);
	ENC_RES;
	return data;
}

// Установить биты в регистре по маске
void encSetReg(uint8_t Address, uint8_t mask){
	encDoubleWrite(CMD_BFS|(Address&0x1f), mask);
}

// Сбросить биты в регистре по маске
void encClearReg(uint8_t Address, uint8_t mask){
	encDoubleWrite(CMD_BFC|(Address&0x1f), mask);
}

// Установить активный банк регистров
void encSetBank(uint8_t bank) {
	encDoubleWrite(CMD_BFS|(ECON1&0x1f),bank);
	encDoubleWrite(CMD_BFC|(ECON1&0x1f),~(bank)&0x03);
}

// Забись буфера в ENC
void encWriteBuf(uint8_t *buf, uint16_t len){
    ENC_SET;
    spiWR(CMD_WBM);	
    for(uint16_t i = 0; i < len; i++) 
        spiWR(buf[i]);
    ENC_RES;
}

// Чтение буфера из ENC
void encReadBuf(uint8_t *buf, uint16_t len){
    ENC_SET;
    spiWR(CMD_RBM);
    while (len--) 
        *buf++ = spiWR(0xFF);	
    ENC_RES;
}

// Запись в регистр периферии
void encWritePHY(uint8_t adr, uint16_t data){
	encSetBank(2);
	encDoubleWrite(CMD_WCR|(0x1f&MIREGADR),adr);
    encWriteDudleReg(CMD_WCR|(0x1f&MIWR), data);
	encSetBank(3);
	while(encReadReg(MISTAT) != 0);
}

void encInit(void){	
    spi_init();
    encSoftReset();
    encSetBank(0);
    
    encWriteDudleReg(ERXST, RX_START);     // Устанавливаем адрес начала буфера приема
    encWriteDudleReg(ERXRDPT, RX_START);   // Устанавливаем адрес указателя приема
    encWriteDudleReg(ERXNDL, RX_END);       // Устанавливаем адрес конца буфера приема
    encWriteDudleReg(ETXSTL, TX_START);     // Устанавливаем адрес начала буфера передачи
    
    encSetBank(2);
    encWriteReg(MACON1, MACON1_TXPAUS|MACON1_RXPAUS|MACON1_MARXEN);
    encWriteReg(MACON3, MACON3_PADCFG0|MACON3_FRMLNEN|MACON3_FULDPX|MACON3_TXCRCEN);
    encWriteDudleReg(MAMXFLL, FRAME_LENGTH);
    encWriteReg(MABBIPG, 0x15);
    encWriteReg(MAIPGL, 0x12);
    encWriteReg(MAIPGH, 0x0c);
    encSetBank(3);
    encWriteReg(MAADR5, MAC_0);
    encWriteReg(MAADR4, MAC_1);
    encWriteReg(MAADR3, MAC_2);
    encWriteReg(MAADR2, MAC_3);
    encWriteReg(MAADR1, MAC_4);
    encWriteReg(MAADR0, MAC_5);
    encWritePHY(PHCON1,PHCON1_PDPXMD);
    encWritePHY(PHCON2,PHCON2_HDLDIS);
    encWritePHY(PHLCON,PHLCON_LACFG2 |PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0 |PHLCON_LFRQ0|PHLCON_STRCH);	
    encSetReg(ECON1,ECON1_RXEN);
}

uint16_t encGetWriteAdr(void){
    encSetBank(0);
    uint16_t data;
    data = encReadReg(EWRPTL);
    data |= encReadReg(EWRPTH)<<8;
    return data;
}

void encSetWriteAdr(uint16_t adr){
    encSetBank(0);
    encWriteDudleReg(EWRPT,adr);
}

// Инициализируем начальные значения и адрес для передачи паета
void encTxPackInit(void){
    encSetWriteAdr(TX_START);
    ENC_SET;
    spiWR(CMD_WBM);
    spiWR(0x00); // 1 байт в пакете задает настройки передачи при 0 настройки по умолчанию
    ENC_RES;
}

void encSetReadAdr(uint16_t adr){
	encSetBank(0);
    encWriteDudleReg(ERDPT,adr);
}

void encSetTxEndAdr(uint16_t adr){
	encSetBank(0);
    encWriteDudleReg(ETXND,adr);
}

void encTxPackSend(void){
    encSetTxEndAdr((encGetWriteAdr() - 1) & MAX_ENC_CHIP_BUFF);
    encSetReg(ECON1, ECON1_TXRTS);
}

void encRxPackDone(void){
	encPackedHead.RX_Addr &= MAX_ENC_CHIP_BUFF;
	encSetBank(0);
    encWriteDudleReg(ERXRDPT,encPackedHead.RX_Addr);
	encSetReadAdr(encPackedHead.RX_Addr); 
}

uint8_t encTestTransmitBusy(void){
	if (encReadReg(EIR) & EIR_TXERIF){
		encSetReg(ECON1,ECON1_TXRST);
		encClearReg(ECON1,ECON1_TXRST);
		_delay_ms(1);
		encSetReg(ECON1,ECON1_TXRTS);
		_delay_ms(1);
		while(encTestTransmitBusy()){}
		return 0;
	}
	if (encReadReg(ECON1) & ECON1_TXRTS) return 1;
	return 0;
}

uint16_t encIdentifiPacket(void){
    uint16_t Length_Packet = 0;
    encSetBank(1); 
    if(encReadReg(EPKTCNT) == 0) return 0; // Провиряем наличае принятых пакетов

	encSetReadAdr(encPackedHead.RX_Addr);
	encReadBuf((uint8_t *)&encPackedHead.RX_Addr,6); // Получаем адрес следующего пакета, длину и статус текущего

	if ((((uint8_t)encPackedHead.RX_Packet_Status) >> RecivePKTBitOk) & 1) // Проверяем валидность пакета 
	    Length_Packet = encPackedHead.RX_Packet_Length;

	encSetReg(ECON2,ECON2_PKTDEC); // Уменьшаем счетчик пакетов на 1
	return Length_Packet;
}

uint16_t encPacketReceive(uint8_t* data, uint16_t maxlen){
    static uint16_t gNextPacketPtr=RX_START;
    uint16_t rxstat;
	uint16_t len;
    encSetBank(1); 
    // Провиряем наличае доступных пакетов
    if(encReadReg(EPKTCNT) == 0) return 0; 
	// Устанавливаем указатель чтения на начало пакета
    encSetBank(0);
	encWriteDudleReg(ERDPT, gNextPacketPtr);
	// Читаем указатель для следующего пакета
    encReadBuf((uint8_t*)&gNextPacketPtr, 2);
	// Считываем размер пакета
    encReadBuf((uint8_t*)&len, 2);
	len -= 4; // удаляем из размера CRC
	// Считываем статус пакета
    encReadBuf((uint8_t*)&rxstat, 2);
	// Ограничиваем размер пакета
    if (len>maxlen-1)len=maxlen-1;
    // Проверям статус и считываем пакет    
    if ((rxstat & 0x80)==0) len=0;
    else encReadBuf(data, len);
        
	// Перемещаем указатель чтения на начало следующего пакета для освобождения памяти
    if (((gNextPacketPtr-1) < RX_START) || ((gNextPacketPtr-1) > RX_END))
        encWriteDudleReg(ERXRDPT, RX_START);
    else 
        encWriteDudleReg(ERXRDPT, (gNextPacketPtr-1));
    
	// Уменьшаем счетчик пакетов на 1
	encSetReg(ECON2, ECON2_PKTDEC);
	return len;

}

void encPacketTransmit(uint8_t* data, uint16_t len){
    // Жем завершения преддыдущей передачи
    while (encReadReg(ECON1) & ECON1_TXRTS){
        if( (encReadReg(EIR) & EIR_TXERIF) ) {
            encSetReg(ECON1, ECON1_TXRST);
		    encClearReg(ECON1, ECON1_TXRST);
        }
    }
    encSetBank(0);
    // Устанавливаем указатель записи на начало буфера
    encWriteDudleReg(EWRPT, TX_START);
    // Первый байт в пакете задает настройки передачи при 0 настройки по умолчанию
	encWriteBuf(0, 1);
	// копируем пакет в буфер передачи
	encWriteBuf(data, len);
    // Устанавливаем указатель конца буфера в соответствии с размером пакета
	encWriteDudleReg(ETXND, TX_START+len);
	// Запускаем передачу
	encSetReg(ECON1, ECON1_TXRTS);
}
