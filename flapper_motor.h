#include "stm32g070xx.h"

#define FLAPPER_MOTOR_OPENING_TIMER_WINDOW      5
#define FLAPPER_MOTOR_OPENING_TIME_SS           2200
#define FLAPPER_MOTOR_CLOSING_DELAY_WINDOW      6
#define FLAPPER_MOTOR_CLOSING_DELAY_TIME_SS     1000
#define FLAPPER_MOTOR_CLOSING_TIMER_WINDOW      7
#define FLAPPER_MOTOR_CLOSING_TIME_SS           12000


void     Flapper_Gpio_Init(void);
void     Flapper_Motor_Init(void);
void     Flapper_Motor_Power_Supply(uint8_t on_state);
uint8_t  Flapper_Motor_Read_Raw_Feedback(void);
void     Flapper_Motor_Lever_Based_Control(void);
void     Flapper_Motor_Magnet_Based_Control(void);
uint8_t  Flapper_Motor_Get_Flapper_State(void);