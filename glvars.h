

#include "stm32g070xx.h"


uint8_t  GlobalVars_Get_Total_Thread(void);
void     GlobalVars_Inc_Total_Thread(void);
void     GlobalVars_Reset_Total_Thread(void);

uint8_t  GlobalVars_Get_Curr_Thread(void);
void     GlobalVars_Inc_Curr_Thread(void);
void     GlobalVars_Reset_Curr_Thread(void);

uint8_t  GlobalVars_Get_Thread_Function(void);
void     GlobalVars_Inc_Thread_Function(void);
void     GlobalVars_Reset_Thread_Function(void);


uint8_t  GlobalVars_Get_Start_Flag(void);
void     GlobalVars_Set_Start_Flag(void);
void     GlobalVars_Clear_Start_Flag(void);