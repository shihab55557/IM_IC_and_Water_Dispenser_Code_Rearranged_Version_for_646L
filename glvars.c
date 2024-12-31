

#include "stm32g070xx.h"
#include "glvars.h"


typedef struct global_vars_debug_t{
	uint8_t TotalThread;
	uint8_t CurrThread;
	uint8_t ThreadFunc;
	uint8_t StartFlag;
}global_vars_debug_t;


global_vars_debug_t DebugThread = {
	0,
	0,
	0,
	0
};

uint8_t GlobalVars_Get_Total_Thread(void){
	return DebugThread.TotalThread;
}

void GlobalVars_Inc_Total_Thread(void){
	DebugThread.TotalThread++;
}

void GlobalVars_Reset_Total_Thread(void){
	DebugThread.TotalThread = 0;
}



uint8_t GlobalVars_Get_Curr_Thread(void){
	return DebugThread.CurrThread;
}


void GlobalVars_Inc_Curr_Thread(void){
	DebugThread.CurrThread++;
}

void GlobalVars_Reset_Curr_Thread(void){
	DebugThread.CurrThread = 0;
}





uint8_t GlobalVars_Get_Thread_Function(void){
	return DebugThread.ThreadFunc;
}


void GlobalVars_Inc_Thread_Function(void){
	DebugThread.ThreadFunc++;
}

void GlobalVars_Reset_Thread_Function(void){
	DebugThread.ThreadFunc = 0;
}





uint8_t GlobalVars_Get_Start_Flag(void){
	return DebugThread.StartFlag;
}

void GlobalVars_Set_Start_Flag(void){
	DebugThread.StartFlag = 1;
}

void GlobalVars_Clear_Start_Flag(void){
	DebugThread.StartFlag = 0;
}


