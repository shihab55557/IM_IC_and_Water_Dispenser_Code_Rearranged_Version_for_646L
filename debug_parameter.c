#include "stm32g070xx.h"
#include "debug_parameter.h"
#include "Icemaker.h"
#include "water_dispenser.h"
#include "disp_comm.h"
#include "Uart.h"
#include "glvars.h"

void Print_All_Debug_Data(void){
	Ice_Maker_Debug_Data();
	Water_Dispenser_Debug_Data();
	Disp_Comm_Print_Heartbeat_Send_Data();
	Disp_Comm_Print_Master_Rec_Data();
	if(GlobalVars_Get_Start_Flag() && (GlobalVars_Get_Thread_Function() == 4)){
		if(GlobalVars_Get_Curr_Thread() == 43){
			UART_PrintfNL(0);
		}
		GlobalVars_Inc_Curr_Thread();
	}
	
	if(GlobalVars_Get_Curr_Thread()>45){
		GlobalVars_Clear_Start_Flag();
		GlobalVars_Reset_Curr_Thread();
		GlobalVars_Reset_Thread_Function();
	}
	
}