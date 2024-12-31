#include "stm32g070xx.h"


#define WATER_DISPENSER_LED_BLINK_TIMER_WINDOW       1
#define WATER_DISPENSER_LED_BLINK_TIMER_PERIOD       200
#define WATER_DISPENSER_LED_BLINK_TIMER_VAL          1200

#define WATER_DISPENSER_MID_WATER_FILL_TIMER_WINDOW  0
//#define WATER_DISPENSER_MID_WATER_FILL_TIME_SEC      5    // Edit it from parameter.c
#define WATER_DISPENSER_LOW_WATER_FILL_TIMER_WINDOW  1
//#define WATER_DISPENSER_LOW_WATER_FILL_TIME_SEC      3


void    Water_Dispenser_Variable_Init(void);
void    Water_Dispenser_Gpio_Init(void);
uint8_t Water_Dispenser_Disp_Command(void);
int8_t  Water_Dispenser_Lever_Input(void);
void    Water_Dispenser_Lever_Input_Reset(void);
int8_t  Water_Dispenser_Lever_Input_Rising_Edge(void);
void    Water_Dispenser_Lever_Input_Rising_Edge_Reset(void);
void    Water_Dispenser_DeLED_High(void);
void    Water_Dispenser_DeLED_Low(void);
void    Water_Dispenser_DeLED_Toggle(void);
void    Water_Dispenser_Debug_Data(void);
void    Water_Dispenser_Init(void);
void    Water_Dispenser_Disp_Based_DeLED_Control(void);
void 		Water_Dispenser_DC_Valve_On(void);
void	  Water_Dispenser_DC_Valve_Off(void);
void    Water_Dispenser_AC_Valve_On(void);
void    Water_Dispenser_AC_Valve_Off(void);
void 		Water_Dispenser_AC_And_DC_Valve_On(void);
void 		Water_Dispenser_AC_And_DC_Valve_Off(void);
void 		Water_Dispenser_Disp_Based_Valve_Control(void);
uint8_t Water_Dispenser_Water_Level_Selection(void);
void    Water_Dispenser_Disp_Based_Valve_Control(void);
void    Water_Dispenser_Send_Filter_Status(void);
void    Water_Dispenser_Handler(void);