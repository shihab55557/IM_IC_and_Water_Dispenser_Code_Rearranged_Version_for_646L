

#include "stm32g070xx.h"

uint16_t CRC_CVTE_Block(uint8_t *buffer, uint16_t len);
uint16_t CRC_X_Modem(uint16_t crc, uint8_t data);
uint16_t CRC_X_Modem_Block(uint8_t *buf, uint8_t len);