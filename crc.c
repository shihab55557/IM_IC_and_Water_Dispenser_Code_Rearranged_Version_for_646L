
#include "stm32g070xx.h"
#include "crc.h"


uint16_t CRC_CVTE_CrcTable[16] = {
   0x0000, 0xCC01, 0xD801, 0x1400,
   0xF001, 0x3C00, 0x2800, 0xE401,
   0xA001, 0x6C00, 0x7800, 0xB401,
   0x5000, 0x9C01, 0x8801, 0x4400
};

uint16_t CRC_CVTE_Block(uint8_t *buffer, uint16_t len){
   uint16_t crc = 0xFFFF, i;
   uint8_t Data;
   for (i = 0; i < len; i++) {
     Data = *buffer++;
     crc = CRC_CVTE_CrcTable[(Data ^ crc) & 0x0f] ^ (crc >> 4);
     crc = CRC_CVTE_CrcTable[((Data >> 4) ^ crc) & 0x0f] ^ (crc >> 4);
 }
 crc = ((crc & 0xFF) << 8) | ((crc >> 8) & 0xFF);
 return crc;
}


uint16_t CRC_X_Modem(uint16_t crc, uint8_t data){
  crc=crc^((uint16_t)data<<8);
  for(uint8_t i = 0; i < 8; i++){
    if(crc & 0x8000){
	  crc = (crc<<1)^0x1021;
	}
    else{
	  crc <<= 1;
	}
  }
  return crc;
}

uint16_t CRC_X_Modem_Block(uint8_t *buf, uint8_t len){
  uint16_t crc = 0;
  for(uint8_t i = 0; i < len; i++){
    crc = CRC_X_Modem(crc,buf[i]);
  }
  return crc;
}



