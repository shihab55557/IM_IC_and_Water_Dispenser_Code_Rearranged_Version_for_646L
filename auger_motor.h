#include "stm32g070xx.h"
 
 void     BLDC_Motor_Struct_Init(void);

 void     Enable_Motor_Start_Flag(void);
 void     Disable_Motor_Start_Flag(void);
 uint8_t  Is_Motor_Start(void);
 
 void     Enable_Motor_Stop_Flag(void);
 void     Disable_Motor_Stop_Flag(void);
 uint8_t  Is_Motor_Stop(void);

 void     Enable_Motor_Rotate_CW_Flag(void);
 void     Disable_Motor_Rotate_CW_Flag(void);
 uint8_t  Is_Motor_Rotate_CW(void);

 void     Enable_Motor_Rotate_CCW_Flag(void);
 void     Disable_Motor_Rotate_CCW_Flag(void);
 uint8_t  Is_Motor_Rotate_CCW(void);
 
 void     I2C1_GPIO_Init(void);
 void     I2C1_Clock_Init(void);
 void     I2C1_Config(void);
 void     I2C1_Init(void);
 uint8_t  I2C1_TX_ADDR(uint8_t addr);
 uint8_t  I2C1_TX_ADDR_READ(uint8_t addr);
 void     I2C1_Write_Register(uint8_t addr, uint8_t data);
 uint8_t  I2C1_Read_BLDC_Motor_Status(uint8_t addr);
 uint8_t  I2C1_Read_BLDC_Motor_Data(uint8_t addr);
 void     BLDC_I2C1_Init(void);
 void     Direction_Pin_Gpio_Init (void);
 
void      Auger_Motor_Rotate_CW (void);
void      Auger_Motor_Rotate_CCW (void);
uint8_t   Auger_Motor_Start (void);
uint8_t   Auger_Motor_Stop (void);

uint8_t   I2C_Read_New(uint8_t reg_addr);
uint8_t   I2C_Read_Reg(uint8_t reg_addr);

void 			Auger_Motor_Init(void);
void      Auger_Motor_CW_Continouse_Rotation(void);

void      Auger_Motor_Disp_Comm_Based_Rotation(void);
void      Auger_Motor_Handler(void);