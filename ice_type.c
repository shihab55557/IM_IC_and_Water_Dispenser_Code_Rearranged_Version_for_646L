#include "stm32g070xx.h"
#include "Uart.h"
#include "water_dispenser.h"
#include "timebase.h"
#include "flapper_motor.h"
#include "auger_motor.h"
#include "disp_comm.h"

enum{
	Flapper_motor_initial_state = 0,
	Flapper_motor_closing_state = 4,
	Flapper_motor_on = 1,
	Flapper_motor_off = 0
};

uint8_t Flapper_motor_status = 0;

void Ice_Type_Disp_Based_Flapper_Motor_Control(void){
	if(((Disp_Comm_Master_Data_Get_IceMaker() & (1<<2)) == (1<<2)) || ((Disp_Comm_Master_Data_Get_IceMaker() & (1<<2)) == (1<<2))){  //Will be replaced with condition for cube ice or crushed             or         only for ice mode(instead of water dispensder mode)
		Flapper_motor_status = Flapper_motor_on;
	}
	else if(Flapper_Motor_Get_Flapper_State() == Flapper_motor_initial_state){
		Flapper_motor_status = Flapper_motor_off;
	}
	if(Flapper_motor_status == Flapper_motor_on){
		Flapper_Motor_Lever_Based_Control();
	}
}


void Ice_Type_Cube_Crush_Selection(void){
	if((Disp_Comm_Master_Data_Get_IceMaker() & (1<<2)) == (1<<2)){
	
	}
}