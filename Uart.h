#include "stm32g070xx.h"

#define  UART_ENABLE_TX        1
#define  UART_ENABLE_RX        0
#define  UART_ENABLE_RX_INT    0
#define  UART_BUF_SIZE         32 

void UART_GPIO_Init(void);
void UART_Clock_Init(void);
void UART_Set_BAUD_Rate(uint32_t baud);
void UART_TX_Config(void);
void UART_RX_Config(void);
void UART_Init(uint32_t baud);
void UART_Transmit_Byte(uint8_t val);
void UART_Transmit_Byte_Hex(uint32_t val);
void UART_Transmit_Byte_Hex_64(uint8_t hex, uint64_t val);
void UART_Transmit_Byte_Bin(uint8_t val);

void UART_Transmit_Text(char *str);
void UART_Determine_Digit_Numbers(uint32_t num);
void UART_Transmit_Number_Digits(void);
void UART_Transmit_Number(int32_t num);
void UART_Transmit_Fixed_Digit_Number(int32_t num, uint8_t digits);

void UART_PrintfD (char *str, int32_t value);
void UART_PrintfD_NL (char *str, int32_t value);
void UART_PrintfH (char *str,uint8_t digit, uint64_t value);
void UART_PrintfNL (uint8_t value);



void UART_Tx_Bin(uint32_t val);
void UART_PrintfD_BIN (char *str, int32_t value);
void UART_PrintfD_HEX (char *str, int32_t value);

void UART_Print_Raw_Data(void);







