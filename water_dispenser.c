#include "stm32g070xx.h"
#include "water_dispenser.h"
#include "disp_comm.h"
#include "timebase.h"
#include "Uart.h"
#include "Parameter.h"
#include "glvars.h"

#warning debug print is enabled in water_dispenser.h

uint8_t lever_released_flag = 0;
uint8_t Water_dispenser_AC_DC_valve_state = 0;

void Water_Dispenser_Gpio_Init(void){
	// PC3 as input for lever press 
	// PA8 for turning on DeLED
	
	SYSCFG->CFGR1 |= SYSCFG_CFGR1_UCPD1_STROBE;    // To disable the pull down register 
	
	// PC3 for Lever press input
	GPIOC->MODER  &=~ GPIO_MODER_MODE3_0;
	GPIOC->MODER  &=~ GPIO_MODER_MODE3_1;   //make PC3 as GPIO input
	
	//  PA8 for turning on DeLED
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER  &=~ GPIO_MODER_MODE8_1; 
	GPIOA->MODER  |= GPIO_MODER_MODE8_0;  	//make PA8 as GPIO output
	
	
	// PC15 for turning on AC water dispenser valve
	RCC->IOPENR |= RCC_IOPENR_GPIOCEN;    	//GPIOC clock En
	GPIOC->MODER  &=~ GPIO_MODER_MODE15_1; 
	GPIOC->MODER  |= GPIO_MODER_MODE15_0;  	//make PC15 as GPIO output
	GPIOC->OTYPER  &=~ GPIO_OTYPER_OT15;
	GPIOC -> ODR &=~ (1<<15);                //make PC15 low by default
	
	// PD0 for turning on DC water dispenser valve
	RCC->IOPENR |= RCC_IOPENR_GPIODEN;
	GPIOD->MODER  &=~ GPIO_MODER_MODE0_1; 
	GPIOD->MODER  |= GPIO_MODER_MODE0_0;  	//make PA8 as GPIO output
	Timebase_DownCounter_Set_Securely(3,12);
	
}


uint8_t Lever_Fb = 0;
int8_t Water_Dispenser_Lever_Input(void){
	if ((GPIOC->IDR & 0b01000)){  // At higher state
		Lever_Fb = 0;
	}
	
	else if (!(GPIOC->IDR & 0b01000)){  // At lower state
		Lever_Fb = 1;
	}
	return Lever_Fb;
}

void Water_Dispenser_Lever_Input_Reset(void){
	Lever_Fb = 0;
}



/*
int8_t Ice_Maker_Hall_Feedback_Checker_RT(void){
	if (!(GPIOD->IDR & 0b01000)){
		Low_state = 1;
	}
	if(Low_state == 1){
		if ((GPIOD->IDR & 0b01000)){
			Hall_feedback = 1;
			Hall_feedback2++;
			Low_state = 0;
		}
	}
	return Hall_feedback;
}

*/

int8_t Lever_low_state = 0;
int8_t Lever_rising_edge_stat = 0;
int8_t Water_Dispenser_Lever_Input_Rising_Edge(void){
	
	if ((GPIOC->IDR & 0b01000)){  // At higher state
		Lever_low_state = 1;
	}
	
	if(Lever_low_state == 1){
		if (!(GPIOC->IDR & 0b01000)){
			Lever_rising_edge_stat = 1;
			Lever_low_state = 0;
		}
	}
	return Lever_rising_edge_stat;
}

void Water_Dispenser_Lever_Input_Rising_Edge_Reset(void){
	Lever_rising_edge_stat = 0;
}


void Water_Dispenser_DeLED_High(void){
	GPIOA -> ODR |= (1<<8);               // PA8 high
}

void Water_Dispenser_DeLED_Low(void){
	GPIOA -> ODR &=~ (1<<8);               // PA8 low
}

void Water_Dispenser_DeLED_Toggle(void){
	if(GPIOA -> ODR & (1<<8)){
		GPIOA -> ODR &=~ (1<<8);
	}
	else{
		GPIOA -> ODR |= (1<<8);  
	}
}

void Water_Dispenser_DeLED(void){
	if(Water_Dispenser_Lever_Input() == 1){
		GPIOA -> ODR |= (1<<8);               // PA8 high
	}
	if(Water_Dispenser_Lever_Input() == 0){
		GPIOA -> ODR &=~ (1<<8);               // PA8 low
	}
}


void Water_Dispenser_Variable_Init(void){
	lever_released_flag = 0;
	Water_dispenser_AC_DC_valve_state = 0;
}


void Water_Dispenser_Init(void){
	Water_Dispenser_Gpio_Init();
	Water_Dispenser_Variable_Init();
}

/*
void Water_Dispenser_Handler(void){
	//Water_Dispenser_DeLED();
	if(Water_Dispenser_Lever_Input() == 1){
		//Disp_Comm_Water_Disp_Set_Lever_Stat(1);
		Disp_Comm_Heartbeat_Set_Data(5,1,0);
	}
	else if(Water_Dispenser_Lever_Input() != 1){
		//Disp_Comm_Water_Disp_Set_Lever_Stat(0);
		Disp_Comm_Heartbeat_Set_Data(5,0,0);
	}
	
	
	if(Disp_Comm_Master_Data_Get_WaterDispenser() & (1 << 6)){
		Water_Dispenser_DeLED_High();
	}
	else{
		Water_Dispenser_DeLED_Low();
	}
}
*/





void Water_Dispenser_Disp_Based_DeLED_Control(void){
	if((Disp_Comm_Master_Data_Get_Mode() & (1 << 2)) == ((1 << 2))){ //Water dispenser On
		Disp_Comm_Heartbeat_Set_Data(9,1,1);
		if(Water_Dispenser_Lever_Input() == 1){
			Water_Dispenser_DeLED_High();
			Disp_Comm_Heartbeat_Set_Data(5,1,0);
		}
		else{
			Water_Dispenser_DeLED_Low();
			Disp_Comm_Heartbeat_Set_Data(5,0,0);
		}
	}
	else{  //Water dispenser Off
		Disp_Comm_Heartbeat_Set_Data(9,0,1);
		if(Water_Dispenser_Lever_Input() == 1){
			if(lever_released_flag == 0){
			  Timebase_DownCounter_SS_Set_Securely(WATER_DISPENSER_LED_BLINK_TIMER_WINDOW, WATER_DISPENSER_LED_BLINK_TIMER_VAL);
			  Timebase_DownCounter_SS_Set_Period_Value_Securely(WATER_DISPENSER_LED_BLINK_TIMER_WINDOW, WATER_DISPENSER_LED_BLINK_TIMER_PERIOD);
				lever_released_flag = 1;
			}
			Disp_Comm_Heartbeat_Set_Data(5,1,0);
		}
		else{
			Disp_Comm_Heartbeat_Set_Data(5,0,0);
			lever_released_flag = 0;
		}
	}
	
	
	if(Timebase_DownCounter_SS_Period_Value_Expired_Event(WATER_DISPENSER_LED_BLINK_TIMER_WINDOW)){
		Water_Dispenser_DeLED_Toggle();
		Timebase_DownCounter_SS_Set_Period_Value_Securely(WATER_DISPENSER_LED_BLINK_TIMER_WINDOW, WATER_DISPENSER_LED_BLINK_TIMER_PERIOD);
	}
		
	if(Timebase_DownCounter_SS_Expired_Event(WATER_DISPENSER_LED_BLINK_TIMER_WINDOW)){
		Water_Dispenser_DeLED_Low();
		
	}

}


void Water_Dispenser_DC_Valve_On(void){
	GPIOD -> ODR |= (1<<0);                 //make D0 high
}

void Water_Dispenser_DC_Valve_Off(void){
	GPIOD -> ODR &=~ (1<<0);                //make D0 low
}

void Water_Dispenser_AC_Valve_On(void){
	GPIOC -> ODR |= (1<<15);                //make PC15 high
}

void Water_Dispenser_AC_Valve_Off(void){
	GPIOC -> ODR &=~ (1<<15);                //make PC15 low
}

void Water_Dispenser_AC_And_DC_Valve_On(void){
	Water_Dispenser_AC_Valve_On();
	Water_Dispenser_DC_Valve_On();
	Water_dispenser_AC_DC_valve_state = 1;
}

void Water_Dispenser_AC_And_DC_Valve_Off(void){
	Water_Dispenser_AC_Valve_Off();
	Water_Dispenser_DC_Valve_Off();
	Water_dispenser_AC_DC_valve_state = 0;
}



enum Water_dispenser_water_level{
	Continuous = 5,
	Mid = 6,
	Low = 7
};

uint8_t Water_disp_water_level = 0;
void Water_Dispenser_Reset_Water_Level_Selection(void){
	Water_disp_water_level = 0; //or Continuous;
	
	/*
	// Reset the buffer that stores low mid and continouse water level selection
	Disp_Comm_Master_Data_Extract_Buffer_Reset(4,5);
	Disp_Comm_Master_Data_Extract_Buffer_Reset(4,6);
	Disp_Comm_Master_Data_Extract_Buffer_Reset(4,7);
	*/
}



uint8_t Water_Dispenser_Disp_Command(void){
	uint8_t Water_disp_on = 0;
	if(((Disp_Comm_Master_Data_Get_Mode() & (1 << 2)) == (1<<2))){
		Water_disp_on = 1;
	}
	return Water_disp_on;
}


uint8_t Water_Dispenser_Water_Level_Selection(void){
	if((Disp_Comm_Master_Data_Get_Mode() & (1 << 2)) == (1<<2)){
		if((Disp_Comm_Master_Data_Get_WaterDispenser() & (1<<7)) == (1<<7)){
			// for continous water
			Water_disp_water_level = Continuous;
			Disp_Comm_Heartbeat_Set_Data(9,1,4);
		}
		
		else {
			Disp_Comm_Heartbeat_Set_Data(9,0,4);
		}
		
		if((Disp_Comm_Master_Data_Get_WaterDispenser() & (1<<6)) == (1<<6)){
			// for mid water
			Disp_Comm_Heartbeat_Set_Data(9,1,3);
			if(Water_Dispenser_Lever_Input_Rising_Edge() == 1){
				Water_disp_water_level = Mid;
				Water_Dispenser_Lever_Input_Rising_Edge_Reset();
			}
		}
		else{
			Disp_Comm_Heartbeat_Set_Data(9,0,3);
		}
		
		if((Disp_Comm_Master_Data_Get_WaterDispenser() & (1<<5)) == (1<<5)){
			// for low water
			Disp_Comm_Heartbeat_Set_Data(9,1,2);
			if(Water_Dispenser_Lever_Input_Rising_Edge() == 1){
				Water_disp_water_level = Low;
				Water_Dispenser_Lever_Input_Rising_Edge_Reset();
			}
		}
		else{
			Disp_Comm_Heartbeat_Set_Data(9,0,2);
		}
	}
	return Water_disp_water_level;
}




void Water_Dispenser_Disp_Based_Valve_Control(void){
		if((Water_Dispenser_Water_Level_Selection() == Continuous) && (Water_Dispenser_Disp_Command() == 1)){
			if(Water_Dispenser_Lever_Input() == 1){
				Water_Dispenser_AC_And_DC_Valve_On();
				//UART_Transmit_Text("VALVE ON");
			}
			else{
				Water_Dispenser_AC_And_DC_Valve_Off();   
				//UART_Transmit_Text("VALVE OFF");
			}
		}
		else if((Disp_Comm_Master_Data_Get_Mode() & (1 << 2)) != (1<<2)){
			Water_Dispenser_AC_And_DC_Valve_Off();
		}
		
		if((Water_Dispenser_Water_Level_Selection() == Mid) && (Water_Dispenser_Disp_Command() == 1)){
			// turn on water disp for mid level
			Disp_Comm_Heartbeat_Set_Data(5,0,5);
			Water_Dispenser_AC_And_DC_Valve_On();
			Timebase_DownCounter_Set_Securely(WATER_DISPENSER_MID_WATER_FILL_TIMER_WINDOW, GET_WATER_DISPENSER_MID_WATER_FILL_TIME_SEC());
			Water_Dispenser_Reset_Water_Level_Selection();
		}
		
		else if((Water_Dispenser_Water_Level_Selection() == Low) && (Water_Dispenser_Disp_Command() == 1)){
			// turn on water disp for low level
			Disp_Comm_Heartbeat_Set_Data(5,0,4);
			Water_Dispenser_AC_And_DC_Valve_On();
			Timebase_DownCounter_Set_Securely(WATER_DISPENSER_LOW_WATER_FILL_TIMER_WINDOW, GET_WATER_DISPENSER_LOW_WATER_FILL_TIME_SEC());
			Water_Dispenser_Reset_Water_Level_Selection();
		}
		
		
		if((Water_Dispenser_Lever_Input() == 0) || (Water_Dispenser_Disp_Command() != 1)){
			Disp_Comm_Heartbeat_Set_Data(5,1,5);
			Water_Dispenser_AC_And_DC_Valve_Off();
			Timebase_DownCounter_Clear_All_Flags(WATER_DISPENSER_MID_WATER_FILL_TIMER_WINDOW);
		}
		
		if((Timebase_DownCounter_Expired_Event(WATER_DISPENSER_MID_WATER_FILL_TIMER_WINDOW))){
			Disp_Comm_Heartbeat_Set_Data(5,1,5);
			Water_Dispenser_AC_And_DC_Valve_Off();
		}
		
		if(((Water_Dispenser_Lever_Input() == 0)) || (Water_Dispenser_Disp_Command() != 1)){
			Disp_Comm_Heartbeat_Set_Data(5,1,4);
			Water_Dispenser_AC_And_DC_Valve_Off();
			Timebase_DownCounter_Clear_All_Flags(WATER_DISPENSER_LOW_WATER_FILL_TIMER_WINDOW);
		}
		
		
		if( Timebase_DownCounter_Expired_Event(WATER_DISPENSER_LOW_WATER_FILL_TIMER_WINDOW)){
			Disp_Comm_Heartbeat_Set_Data(5,1,4);
			Water_Dispenser_AC_And_DC_Valve_Off();
		}
		
		
		
}



void Water_Dispenser_Send_Filter_Status(void){
	// For dummy testing purpose
	//Timebase_DownCounter_Set_Securely(2,4);  // Remove it from init function
	if(Timebase_DownCounter_Expired_Event(3)){
		Disp_Comm_Heartbeat_Set_Data(5,1,1);
		Disp_Comm_Heartbeat_Set_Data(5,0,3);
		Timebase_DownCounter_Clear_All_Flags(3);
		Timebase_DownCounter_Set_Securely(4,480);
	}
	if(Timebase_DownCounter_Expired_Event(4)){
		Disp_Comm_Heartbeat_Set_Data(5,1,2);
		Disp_Comm_Heartbeat_Set_Data(5,0,1);
		Timebase_DownCounter_Clear_All_Flags(4);
		Timebase_DownCounter_Set_Securely(5,480);
	}
	if(Timebase_DownCounter_Expired_Event(5)){
		Disp_Comm_Heartbeat_Set_Data(5,1,3);
		Disp_Comm_Heartbeat_Set_Data(5,0,2);
		Timebase_DownCounter_Clear_All_Flags(5);
		Timebase_DownCounter_Set_Securely(3,480);
	}
}


void Water_Dispenser_Handler(void){
	Water_Dispenser_Disp_Based_DeLED_Control();
	Water_Dispenser_Disp_Based_Valve_Control();
	Water_Dispenser_Send_Filter_Status();
}


void Water_Dispenser_Debug_Data(void){
	
	if(GlobalVars_Get_Start_Flag() && (GlobalVars_Get_Thread_Function() == 1)){
	  if(GlobalVars_Get_Curr_Thread() == 20){
			UART_PrintfD_NL("WD_water_level ",Water_disp_water_level);
		}
		else if(GlobalVars_Get_Curr_Thread() == 21){
			UART_PrintfD_NL("WD_lever_input ",Water_Dispenser_Lever_Input());
		}
	  else if(GlobalVars_Get_Curr_Thread() == 22){
			UART_PrintfD_NL("WD_AC_DC_valve_state ",Water_dispenser_AC_DC_valve_state);
			GlobalVars_Inc_Thread_Function();
		}
	  GlobalVars_Inc_Curr_Thread();
	}
}

