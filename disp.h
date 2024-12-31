

#include "stm32g070xx.h"

void     Disp_UART_Struct_Init(void);
void     Disp_UART_GPIO_Init(void);
void     Disp_UART_Clock_Init(void);
void     Disp_UART_Set_BAUD_Rate(uint32_t baud);
void     Disp_UART_TX_Config(void);
void     Disp_UART_RX_Config(void);
void     Disp_UART_RX_Int_Config(void);

void     Disp_UART_Timer_Init(void);
void     Disp_UART_Timer_Enable(void);
void     Disp_UART_Timer_Disable(void);
uint8_t  Disp_UART_Get_Timer_Status(void);
uint16_t Disp_UART_Get_Counter_Val(void);
void     Disp_UART_Counter_Reset(void);

void     Disp_UART_Init(uint32_t baud);
void     Disp_UART_Transmit_Byte(uint8_t val);

void     Disp_UART_Tx_Buf(uint8_t *buf, uint16_t len);

void     Disp_UART_Tx_Hex(uint32_t val);
void     Disp_UART_Tx_Bin(uint32_t val);

void     Disp_UART_Tx_NL(void);
void     Disp_UART_Tx_SP(void);
void     Disp_UART_Tx_CM(void);

void     Disp_UART_Tx_Text(char *str);
void     Disp_UART_Tx_Text_NL(char *str);
void     Disp_UART_Tx_Text_SP(char *str);
void     Disp_UART_Tx_Text_CM(char *str);

void     Disp_UART_Determine_Digit_Numbers(uint32_t num);
void     Disp_UART_Tx_Number_Digits(void);
void     Disp_UART_Tx_Number(int32_t num);

void     Disp_UART_Tx_Number_NL(int32_t num);
void     Disp_UART_Tx_Number_SP(int32_t num);
void     Disp_UART_Tx_Number_CM(int32_t num);

void     Disp_UART_Tx_Number_Hex_NL(int32_t num);
void     Disp_UART_Tx_Number_Hex_SP(int32_t num);
void     Disp_UART_Tx_Number_Hex_CM(int32_t num);

void     Disp_UART_Tx_Number_Bin_NL(int32_t num);
void     Disp_UART_Tx_Number_Bin_SP(int32_t num);
void     Disp_UART_Tx_Number_Bin_CM(int32_t num);


void     Disp_UART_Tx_Parameter_NL(char *name, int32_t num);
void     Disp_UART_Tx_Parameter_SP(char *name, int32_t num);
void     Disp_UART_Tx_Parameter_CM(char *name, int32_t num);

void     Disp_UART_Tx_Parameter_Hex_NL(char *name, int32_t num);
void     Disp_UART_Tx_Parameter_Hex_SP(char *name, int32_t num);
void     Disp_UART_Tx_Parameter_Hex_CM(char *name, int32_t num);

void     Disp_UART_Tx_Parameter_Bin_NL(char *name, int32_t num);
void     Disp_UART_Tx_Parameter_Bin_SP(char *name, int32_t num);
void     Disp_UART_Tx_Parameter_Bin_CM(char *name, int32_t num);

void     Disp_UART_Flush_Buf(void);
uint8_t  Disp_UART_Get_Buf(uint16_t index);
uint8_t  Disp_UART_Get_Buf_Index(void);

uint8_t  Disp_UART_Data_Available(void);
void     Disp_UART_Clear_Data_Available_Flag(void);
uint8_t  Disp_UART_Get_Data_Len(void);
void     Disp_UART_Data_Copy_Buf(uint8_t *buf);
void     Disp_UART_Print_Buf_Data(void);
