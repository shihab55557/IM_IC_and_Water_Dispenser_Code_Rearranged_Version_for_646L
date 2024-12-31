#include "stm32g070xx.h"
#include "Uart.h"
#include "water_dispenser.h"
#include "timebase.h"
#include "flapper_motor.h"
#include "disp_comm.h"



void Flapper_Gpio_Init(void){
	//PC12 as SigACH1 as output for running the flapper motor
	RCC->IOPENR |= RCC_IOPENR_GPIOCEN;    	//GPIOC clock En
	GPIOC->MODER  &=~ GPIO_MODER_MODE12_1; 
	GPIOC->MODER  |= GPIO_MODER_MODE12_0;  	//make PC12 as GPIO output
	
	//PA4 as NTC1_P as input for reading the position of flapper motor
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;    	//GPIOA clock En
	GPIOA->MODER  &=~ GPIO_MODER_MODE4_0;
	GPIOA->MODER  &=~ GPIO_MODER_MODE4_1;   //make PA4 as GPIO input
}

void Flapper_Motor_Init(void){
	Flapper_Gpio_Init();
}

enum flapper_supply{
	On = 1,
	Off = 0
};

void Flapper_Motor_Power_Supply(uint8_t on_state){
	if(on_state){
		GPIOC -> ODR |= (1<<12);               // PC12 high
	}
	else{
		GPIOC -> ODR &=~ (1<<12);               // PC12 low
	}
}

uint8_t Flapper_Motor_Read_Raw_Feedback(void){
	uint8_t flapper_fb = 0;
	if ((GPIOA->IDR & (1<<4))){  // At higher state
			flapper_fb = 1;
	}
	else{
		flapper_fb = 0;
	}
	return flapper_fb;
}

uint8_t Flapper_motor_state = 0;

enum flapper_position{
	Initial_state = 0,
	Opening_state = 1,
	Opening_end   = 2,
	Closing_delay = 3,
	Closing_state = 4
};

void Flapper_Motor_Lever_Based_Control(void){
	if(Flapper_motor_state == Initial_state){
		if(Water_Dispenser_Lever_Input()){
			Timebase_DownCounter_SS_Set_Forcefully(FLAPPER_MOTOR_OPENING_TIMER_WINDOW, FLAPPER_MOTOR_OPENING_TIME_SS);
			Flapper_Motor_Power_Supply(On);
			Flapper_motor_state = Opening_state;
		}
	}
	if(Flapper_motor_state == Opening_state){
		if( Timebase_DownCounter_SS_Expired_Event(FLAPPER_MOTOR_OPENING_TIMER_WINDOW) ){
			Flapper_Motor_Power_Supply(Off);
			Flapper_motor_state = Opening_end;
		}
	}
	if(Flapper_motor_state == Opening_end){
		if(!Water_Dispenser_Lever_Input()){
			Timebase_DownCounter_SS_Set_Forcefully(FLAPPER_MOTOR_CLOSING_DELAY_WINDOW, FLAPPER_MOTOR_CLOSING_DELAY_TIME_SS);
			Flapper_motor_state = Closing_delay;
		}
	}
	if(Flapper_motor_state == Closing_delay){
		if( Timebase_DownCounter_SS_Expired_Event(FLAPPER_MOTOR_CLOSING_DELAY_WINDOW) ){
			Flapper_Motor_Power_Supply(On);
			Timebase_DownCounter_SS_Set_Forcefully(FLAPPER_MOTOR_CLOSING_TIMER_WINDOW, FLAPPER_MOTOR_CLOSING_TIME_SS);
			Flapper_motor_state = Closing_state;
		}
	}
	if(Flapper_motor_state == Closing_state){
		if((!Flapper_Motor_Read_Raw_Feedback()) || (Timebase_DownCounter_SS_Expired_Event(FLAPPER_MOTOR_CLOSING_TIMER_WINDOW))){
			Flapper_Motor_Power_Supply(Off);
			Flapper_motor_state = Initial_state;
		}
	}
}

uint8_t Flapper_Motor_Get_Flapper_State(void){
	return Flapper_motor_state;
}


// will be deleted

void Flapper_Motor_Magnet_Based_Control(void){
	if(Flapper_Motor_Read_Raw_Feedback()){
		Flapper_Motor_Power_Supply(On);
		Water_Dispenser_DeLED_High();
	}
	else{
		Flapper_Motor_Power_Supply(Off);
		Water_Dispenser_DeLED_Low();
	}
}



