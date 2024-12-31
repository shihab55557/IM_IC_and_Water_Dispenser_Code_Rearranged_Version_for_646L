#include "stm32g070xx.h"
#include "Parameter.h"
#include "disp_comm.h"

//------------------------------------------------------------------PARAMETER FOR ICEMAKER------------------------------------------------------------------//
#define ICE_MAKER_VALVE_ON_TIME         2                                                          // Valve on time 
#define ICE_MAKER_VALVE_OFF_TIME        3
#define ICE_MAKER_VALVE_ON_CYCLE        3
#define ICE_MAKER_AFTER_VALVE_TIME      3                                                          // Wait for turning water into ice
 
#define ICE_MAKER_IC_FULL_WAIT_TIME     3                                                          // Wait time to IC be empty if it full 
#define ICE_MAKER_SET_ICE_TEMP          (200)                                                    // Set ice temperature LOGIC WILL BE CHANGED IN 2 PLACES
#define ICE_MAKER_HEATER_RUN_TIME       600        
#define ICE_MAKER_HEATER_TEMP_RANGE     50

#define ICE_MAKER_HEATER_RUNNING_TEMP_MIN          (ICE_MAKER_SET_ICE_TEMP + 5)                     // Set ice temperature 
#define ICE_MAKER_HEATER_RUNNING_TEMP_MAX          (ICE_MAKER_SET_ICE_TEMP + 5 + ICE_MAKER_HEATER_TEMP_RANGE)   // Set ice temperature 
//------------------------------------------------------------------PARAMETER FOR ICEMAKER------------------------------------------------------------------//



//------------------------------------------------------------------PARAMETER FOR WATER DISPENSER------------------------------------------------------------------//
#define WATER_DISPENSER_MID_WATER_FILL_TIME_SEC      10
#define WATER_DISPENSER_LOW_WATER_FILL_TIME_SEC      5
//------------------------------------------------------------------PARAMETER FOR WATER DISPENSER------------------------------------------------------------------//

int16_t Valve_on_time = 0;
int16_t Ice_maker_valve_on_cycle = 0;

int16_t Valve_off_time = 0;
int32_t Ice_maker_after_valve_time = 0;
int32_t Ice_maker_ic_full_wait_time = 0;
int32_t Ice_maker_set_ice_temp = 0;
int32_t Ice_maker_heater_temp_range = 0;
int32_t Ice_maker_heater_running_temp_min = 0;
int32_t Ice_maker_heater_running_temp_max = 0;
int16_t heater_run_time = 0;
int8_t  Water_dispenser_mid_water_level_time_sec = 0;
int8_t  Water_dispenser_low_water_level_time_sec = 0;






void Parameter_Init(void){            // Default parameters
	Valve_on_time = ICE_MAKER_VALVE_ON_TIME;
	Ice_maker_valve_on_cycle = ICE_MAKER_VALVE_ON_CYCLE;
}


void Parameter_Get_Disp_Comm_Val(void){    // Parameter gets from disp comm
	if((Disp_Comm_Master_Data_Get_IceMaker() & (1<<0)) == (1<<0)){
		//Valve_on_time = Disp_Comm_Master_Data_Get_IMValveOnTime();
		//Ice_maker_valve_on_cycle = Disp_Comm_Master_Data_Get_IMValveOnCycle();
		Disp_Comm_Heartbeat_Set_Data(8,1,6);
	}
	else{
		Disp_Comm_Heartbeat_Set_Data(8,0,6);
	}
}


int16_t GET_ICE_MAKER_VALVE_ON_TIME(void){
	return Valve_on_time;
}

int16_t GET_ICE_MAKER_VALVE_ON_CYCLE(void){
	return Ice_maker_valve_on_cycle;
}




int16_t GET_ICE_MAKER_VALVE_OFF_TIME(void){
	Valve_off_time = ICE_MAKER_VALVE_OFF_TIME;
	return Valve_off_time;
}


int32_t GET_ICE_MAKER_AFTER_VALVE_TIME(void){
	Ice_maker_after_valve_time = ICE_MAKER_AFTER_VALVE_TIME;
	return Ice_maker_after_valve_time;
}


int32_t GET_ICE_MAKER_IC_FULL_WAIT_TIME(void){
	Ice_maker_ic_full_wait_time = ICE_MAKER_IC_FULL_WAIT_TIME;
	return Ice_maker_ic_full_wait_time;
}

int GET_ICE_MAKER_SET_ICE_TEMP(void){
	Ice_maker_set_ice_temp = ICE_MAKER_SET_ICE_TEMP;
	return Ice_maker_set_ice_temp;
}

int32_t GET_ICE_MAKER_HEATER_TEMP_RANGE(void){
	Ice_maker_heater_temp_range = ICE_MAKER_HEATER_TEMP_RANGE;
	return Ice_maker_heater_temp_range;
}

int32_t GET_ICE_MAKER_HEATER_RUNNING_TEMP_MIN(void){
	Ice_maker_heater_running_temp_min = ICE_MAKER_HEATER_RUNNING_TEMP_MIN;
	return Ice_maker_heater_running_temp_min;
}

int32_t GET_ICE_MAKER_HEATER_RUNNING_TEMP_MAX(void){
	Ice_maker_heater_running_temp_max = ICE_MAKER_HEATER_RUNNING_TEMP_MAX;
	return Ice_maker_heater_running_temp_max;
}



int16_t GET_ICE_MAKER_HEATER_RUN_TIME(void){
	heater_run_time = ICE_MAKER_HEATER_RUN_TIME;
	return heater_run_time;
}

int8_t GET_WATER_DISPENSER_MID_WATER_FILL_TIME_SEC(void){
	Water_dispenser_mid_water_level_time_sec = WATER_DISPENSER_MID_WATER_FILL_TIME_SEC;
	return Water_dispenser_mid_water_level_time_sec;
}

int8_t GET_WATER_DISPENSER_LOW_WATER_FILL_TIME_SEC(void){
	Water_dispenser_low_water_level_time_sec = WATER_DISPENSER_LOW_WATER_FILL_TIME_SEC;
	return Water_dispenser_low_water_level_time_sec;
}