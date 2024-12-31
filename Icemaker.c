#include "stm32g070xx.h"
#include "Icemaker.h"
#include "Uart.h"
#include "ntc.h"
#include "timer.h"
#include "disp_comm.h"
#include "Parameter.h"
#include "glvars.h"
#include "User_config.h"
#include "timebase.h"
#define ICEMAKER_OFF_BY_DEF    1

typedef struct IceMaker_timer{
		volatile int16_t on_time;               //static volatile int16_t IM.timer.on_time = 0;        // To count the ice maker on time of motor in second
		volatile int16_t valve;                 //static volatile int16_t IM.timer.valve = 0;       // To count the valve on time
		volatile int16_t ic_full_wait;          //static volatile int16_t IM.timer.ic_full_wait = 0;        // To count the wait time while IB=1 and IC=1
		volatile int16_t from_ccw_mid;          //static volatile int16_t IM.timer.from_ccw_mid = 0;     // To count the last rotation time after passing CCW mid hall feedback at normal rotation(IC=0 and IB=0)
		volatile int16_t non_blocker;           //static volatile int16_t IM.timer.non_blocker = 0;
		volatile int16_t init_or_conf_rotation;      //static volatile int16_t IM.timer.init_or_conf_rotation = 0;// To count the rotation time at initial rotation so that it can be used as safety purpose
		volatile int8_t debug_data;             //static volatile int16_t IM.timer.debug_data = 0;    // Debug data will be sent in every 2 seconds
		volatile int8_t from_init_ccw_rotation; //static volatile int16_t IM.timer.from_init_ccw_rotation = 0;     // It counts the CCW rotation time from Ice_Maker_Ice_Tray_Rotation_Init(); if hall feedback found before 3 second, then it is reached at initial position; else, it found CCW mid hfb
		volatile int8_t after_reset;            //static volatile int16_t IM.timer.after_reset = 0;   //It actually counts time and waits for 5 seconds after completing each cycle(full rotation or half rotation) and before starting another cycle
		volatile int8_t cw_rotation;            //static volatile int16_t IM.timer.cw_rotation = 0;        // Timer that is used to count the time for lever feedback (if got hall feedback found before IM.timer.cw_rotation <3 ; that will count as lever feedback)
		volatile int16_t heater_run;            //static volatile int16_t IM.timer.heater_run = 0;     // Timer that count time to run heater
} IceMaker_timer;

typedef struct IceMaker_flag{
	uint8_t cw_rotation_timer;          //int8_t IM.flag.cw_rotation_timer = 0;           // maybe this is a flag
	uint8_t ice_tray;                   //int8_t IM.flag.ice_tray = 0;                          // To update ice box status
	uint8_t ice_container;              //int8_t IM.flag.ice_container = 0;                     // To update ice container status
	uint8_t valve_filled;               //int8_t IM.flag.valve_filled = 0;                 // To update whether valve filled water and ready to dispense ice
	uint8_t ice_tray_rot_op;            //int8_t IM.flag.ice_tray_rot_op = 0;  // To update whether the system will go to ice dispense operation by rotating the motor
	uint8_t im_module_op;               //int8_t IM.flag.im_module_op = 0;					 // Flag that indicate whether the ice maker will run or not from begining(from turning water valve ON)
	uint8_t non_bl_timer;               //int8_t IM.flag.non_bl_timer = 0;
	uint8_t ice_tray_init_or_conf_rotation; //int8_t IM.flag.ice_tray_init_or_conf_rotation = 0;   // To rotate the motor initially before while loop or BASED ON TEST BUTTON
	uint8_t init_rotation_done;         //int8_t init_or_conf_rotation = 0;                  // Rotate initially only for first time when temperature reaches at SET_ICE_TEMP
	uint8_t init_ccw_rotate_timer;      //int8_t IM.flag.init_ccw_rotate_timer = 0;         // 1 means it will start the ccw rotation time from Ice_Maker_Ice_Tray_Rotation_Init();
	uint8_t init_ccw_rotate_result;     //int8_t IM.flag.init_ccw_rotate_result = 0;            // 1 means ice tray returned from mid or contains water ; 0 means returned from CW end or empty
	uint8_t valve_on;                   //int8_t IM.flag.valve_on = 0;
	uint8_t ic_full_wait_time;          //int8_t IM.flag.ic_full_wait_time = 0;
	uint8_t after_reset_timer;     //int8_t IM.flag.after_reset_timer = 0;      // The flag to count the time of IM.timer.after_reset
	uint8_t disp_based_im_state;        //int8_t IM.flag.disp_based_im_state = 0;                         // 1 means ice maker in running according ot disp command; 0 means not running
	uint8_t init_ice_tray_not_at_ccw_end; //uint8_t IM.flag.init_ice_tray_not_at_ccw_end = 0;    // to determine whether the ice tray found at ccw end at initial rotation;
	uint8_t init_ice_tray_ccw_end_det_break;// uint8_t IM.flag.init_ice_tray_ccw_end_det_break = 0;
}IceMaker_flag;

typedef struct IceMaker_button{
	uint8_t low_state;     //int8_t IM.button.low_state = 0;                         // To check the low state of the test button
	uint8_t pressed;       //int8_t IM.button.pressed = 0;                           // To store the rising edge of the test button
}IceMaker_button;

typedef struct IceMaker_hallFeedback{
	uint8_t low_state;        //int8_t IM.hall_fb.low_state = 0;
	uint8_t high_state;       // int8_t IM.hall_fb.high_state = 0;
	uint8_t feedback;    //int8_t IM.hall_fb.feedback = 0;  
	uint8_t feedback2;   //int    IM.hall_fb.feedback2 = 0;                           // For hall feedback checker
}IceMaker_hallFeedback;

typedef struct IceMaker_rotationSteps{
	uint8_t step0;
	uint8_t step1;
	uint8_t step2;
	uint8_t step3;
	uint8_t step4;
}IceMaker_rotationSteps;

typedef struct Ice_maker{
	IceMaker_timer            timer;
	IceMaker_flag             flag;
	IceMaker_button           button;
	IceMaker_hallFeedback     hall_fb;
	IceMaker_rotationSteps    rotation;
	IceMaker_rotationSteps    init_rotate;
	int                       temperature;           //int    IM.temperature = 0;                       // To store the value NTC sensor
	uint8_t                   lever_feedback;        //int8_t IM.lever_feedback = 0;                           // 1 = lever is stopped by external force; 0 = not stopped by...
	uint8_t                   valve_on_cycle_count;  //int8_t IM.valve_on_cycle_count = 0;
	uint8_t                   motor_direction;       //int8_t IM.motor_direction = 0;                          // Variable that the direction of motor
}Ice_maker;
Ice_maker IM;

void Ice_Maker_IM_Struct_Init(void){
	IM.timer.on_time          = 0;      //static volatile int16_t IM.timer.on_time = 0;
	IM.timer.valve            = 0;      //static volatile int16_t IM.timer.valve = 0;
	IM.timer.ic_full_wait     = 0;      //static volatile int16_t IM.timer.ic_full_wait = 0;
	IM.timer.from_ccw_mid     = 0;      //static volatile int16_t IM.timer.from_ccw_mid = 0;
	IM.timer.non_blocker      = 0;      //static volatile int16_t IM.timer.non_blocker = 0;
	IM.timer.init_or_conf_rotation = 0;      //static volatile int16_t IM.timer.init_or_conf_rotation = 0;
	IM.timer.debug_data       = 0;      //static volatile int16_t IM.timer.debug_data = 0;
	IM.timer.from_init_ccw_rotation = 0;//static volatile int16_t IM.timer.from_init_ccw_rotation = 0;
	IM.timer.after_reset      = 0;      //static volatile int16_t IM.timer.after_reset = 0;
	IM.timer.cw_rotation      = 0;      //static volatile int16_t IM.timer.cw_rotation = 0;
	IM.timer.heater_run       = 0;      //static volatile int16_t IM.timer.heater_run = 0;
	
	IM.flag.cw_rotation_timer = 0;      //int8_t IM.flag.cw_rotation_timer = 0; 
	IM.flag.ice_tray          = 0;      //int8_t IM.flag.ice_tray = 0; 
	IM.flag.ice_container     = 0;      //int8_t IM.flag.ice_container = 0;
	IM.flag.valve_filled      = 0;      //int8_t IM.flag.valve_filled = 0;   
	IM.flag.ice_tray_rot_op   = 0;      //int8_t IM.flag.ice_tray_rot_op = 0;
	IM.flag.im_module_op      = 0;      //int8_t IM.flag.im_module_op = 0;	
	IM.flag.non_bl_timer      = 0;      //int8_t IM.flag.non_bl_timer = 0;
	IM.flag.ice_tray_init_or_conf_rotation = 0;//int8_t IM.flag.ice_tray_init_or_conf_rotation = 0;
	IM.flag.init_rotation_done= 0;      //int8_t IM.flag.init_rotation_done = 0; 
	IM.flag.init_ccw_rotate_timer = 0;  //int8_t IM.flag.init_ccw_rotate_timer = 0;
	IM.flag.init_ccw_rotate_result = 0; //int8_t IM.flag.init_ccw_rotate_result = 0;  
	IM.flag.valve_on          = 0;      //int8_t IM.flag.valve_on = 0;
	IM.flag.ic_full_wait_time = 0;      //int8_t IM.flag.ic_full_wait_time = 0;
	IM.flag.after_reset_timer = 0; //int8_t IM.flag.after_reset_timer = 0;  
	IM.flag.disp_based_im_state =0;     //int8_t IM.flag.disp_based_im_state = 0;     
	IM.flag.init_ice_tray_not_at_ccw_end = 0;//uint8_t IM.flag.init_ice_tray_not_at_ccw_end = 0;   
	IM.flag.init_ice_tray_ccw_end_det_break = 0;// uint8_t IM.flag.init_ice_tray_ccw_end_det_break = 0;
	
	IM.button.low_state       = 0;      //int8_t IM.button.low_state = 0; 
	IM.button.pressed         = 0;      //int8_t IM.button.pressed = 0;   
	
	IM.hall_fb.low_state      = 0;      //int8_t IM.hall_fb.low_state = 0;
	IM.hall_fb.high_state     = 0;      // int8_t IM.hall_fb.high_state = 0;
	IM.hall_fb.feedback       = 0;      //int8_t IM.hall_fb.feedback = 0;  
	IM.hall_fb.feedback2      = 0;      //int    IM.hall_fb.feedback2 = 0;     
	
	IM.rotation.step0         = 0;      //IM.rotation.step0
	IM.rotation.step1         = 0;      //IM.rotation.step1
	IM.rotation.step2         = 0;      //IM.rotation.step2
	IM.rotation.step3         = 0;      //IM.rotation.step3
	IM.rotation.step4         = 0;      //IM.rotation.step4
	
	IM.init_rotate.step0      = 0;      //IM.init_rotate.step0
	IM.init_rotate.step1      = 0;      //IM.init_rotate.step1
	IM.init_rotate.step2      = 0;      //IM.init_rotate.step2
	IM.init_rotate.step3      = 0;      //IM.init_rotate.step3
	IM.init_rotate.step4      = 0;      //IM.init_rotate.step4
	
	IM.temperature            = 0;      //int    IM.temperature = 0;    
	IM.lever_feedback         = 0;      //int8_t IM.lever_feedback = 0;     
	IM.valve_on_cycle_count   = 0;      //int8_t IM.valve_on_cycle_count = 0;
	IM.motor_direction        = 0;      //int8_t IM.motor_direction = 0;                          // Variable that the direction of motor
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_UIF) {
				TIM1->SR &= ~TIM_SR_UIF;
			
				IM.timer.debug_data++;
				IM.timer.heater_run++;
				if(IM.flag.after_reset_timer == 1){
					IM.timer.after_reset++;
					if(IM.timer.after_reset > 7){
						IM.flag.after_reset_timer = 0;        // No need to count the IM.timer.after_reset after 5 second 
					}
				}
				
				if(IM.flag.init_ccw_rotate_timer == 1){         // to count initial CCW rotation based on which IM.flag.init_ccw_rotate_result will be updated
					IM.timer.from_init_ccw_rotation++;
				}
				if(IM.timer.on_time <= 10){
					IM.timer.on_time++;
				}
				
				if(IM.flag.ice_tray_init_or_conf_rotation ==  1){
					IM.timer.init_or_conf_rotation ++;
				}
				
				if(IM.flag.im_module_op == 1){
					if((IM.flag.ice_tray == 0)&&(IM.flag.ice_container == 0)&&(IM.flag.valve_on == 1)){
						IM.timer.valve++;
						if(IM.timer.valve == (GET_ICE_MAKER_VALVE_ON_TIME())){                  // Mid portion of a cycle from multiple cycle for turnig a valve on
							IM.valve_on_cycle_count++;                                   // To count a partition cycle for turning valve on
						}
						if(IM.timer.valve >= (GET_ICE_MAKER_VALVE_OFF_TIME()+ GET_ICE_MAKER_VALVE_ON_TIME() + GET_ICE_MAKER_AFTER_VALVE_TIME())){
							IM.timer.valve = (GET_ICE_MAKER_VALVE_OFF_TIME()+ GET_ICE_MAKER_VALVE_ON_TIME() + GET_ICE_MAKER_AFTER_VALVE_TIME());
						}
					}
				
					if((IM.flag.ice_tray == 1)&&(IM.flag.ice_container == 1)&&(IM.flag.ic_full_wait_time == 1)){
						if(IM.timer.ic_full_wait <= GET_ICE_MAKER_IC_FULL_WAIT_TIME()){
							IM.timer.ic_full_wait++;
						}
					}
				}
				
				if(IM.flag.ice_tray_rot_op == 1){
					IM.timer.cw_rotation ++;
				}
				if(IM.flag.non_bl_timer == 1){
					IM.timer.non_blocker++;
				}
    }
}


void Ice_Maker_Init(void){
	  // FOR GPIO
		SYSCFG->CFGR1 |= SYSCFG_CFGR1_UCPD2_STROBE;// To undo the force pull down of PD2 and PD1 as GPIO input
	  SYSCFG->CFGR1 |= SYSCFG_CFGR1_UCPD1_STROBE;
		//For motor controll
		RCC->IOPENR|=RCC_IOPENR_GPIOBEN;
		GPIOB->MODER  &=~ GPIO_MODER_MODE13_1; 
		GPIOB->MODER  |= GPIO_MODER_MODE13_0;  	//make PB13 as GPIO output
		GPIOB->OTYPER &=~ GPIO_OTYPER_OT13_Msk ;	
		GPIOB->MODER  &=~ GPIO_MODER_MODE14_1; 
		GPIOB->MODER  |= GPIO_MODER_MODE14_0;  	//make PB14 as GPIO output
		GPIOB->OTYPER &=~ GPIO_OTYPER_OT14_Msk ;	
		//For hall feedback input
		RCC->IOPENR |= RCC_IOPENR_GPIODEN;    	//GPIOD clock En
		GPIOD->MODER  &=~ GPIO_MODER_MODE3_0;
		GPIOD->MODER  &=~ GPIO_MODER_MODE3_1;   //make PD3 as GPIO input
		//For valve control
		RCC->IOPENR |= RCC_IOPENR_GPIOCEN;    	//GPIOC clock En
		GPIOC->MODER  &=~ GPIO_MODER_MODE14_1; 
		GPIOC->MODER  |= GPIO_MODER_MODE14_0;  	//make PC14 as GPIO output
		GPIOC->OTYPER  &=~ GPIO_OTYPER_OT14;
		GPIOC -> ODR &=~ (1<<14);                //make PC14 low by default
		// For heater control
		GPIOB->MODER  &=~ GPIO_MODER_MODE8_1; 
		GPIOB->MODER  |= GPIO_MODER_MODE8_0;  	//make PB8 as GPIO output
		// For test button input
		RCC->IOPENR |= RCC_IOPENR_GPIODEN;    	//GPIOD clock En
		GPIOD->MODER  &=~ GPIO_MODER_MODE2_0;
		GPIOD->MODER  &=~ GPIO_MODER_MODE2_1;   //make PD2 as GPIO input
		
		NTC_Sensor_Init();
		Ice_Maker_IM_Struct_Init();
		Parameter_Init();                       // Default parameter set from Parameter.c
}

enum motor_switch{
	Forward = 1,
	Reverse = 2,
	Motor_off =3
};

void Ice_Maker_Motor_Control(int dir){            	// dir= direction of rotation	
	if(dir == Forward){                     // M1 forward
		GPIOB -> ODR |= (1<<13);               // PB13 high
		GPIOB -> ODR &=~ (1<<14);;              // PB14  low
		IM.motor_direction = 1;
	}
	else if(dir == Reverse){                // M1 reverse
		GPIOB -> ODR &=~ (1<<13);               // PB13 low
		GPIOB -> ODR |= (1<<14);;              // PB14  high
		IM.motor_direction = 2;
	}
	else if (dir == Motor_off){             // M1 stop
		GPIOB -> ODR &=~ (1<<13);               // PB13 low
		GPIOB -> ODR &=~ (1<<14);;              // PB14  low      
		IM.motor_direction = 0;
	}
	//return IM.motor_direction;
}

uint8_t Ice_Maker_Get_Motor_Direction(void){
	return IM.motor_direction;
}

int8_t Ice_Maker_Hall_Feedback_checker(void){                          // This function checks the raw hall feedback checking the input GPIO pin
	int Hall_Fb = 0;         // Hall feedback
		if ((GPIOD->IDR & 0b01000)){  // At higher state
			Hall_Fb = 1;
		}
		else {
			Hall_Fb = 0;
		}
	return Hall_Fb;
}

int8_t Ice_Maker_Hall_Feedback_Checker_RT(void){
	if (!(GPIOD->IDR & 0b01000)){
		IM.hall_fb.low_state = 1;
	}
	if(IM.hall_fb.low_state == 1){
		if ((GPIOD->IDR & 0b01000)){
			IM.hall_fb.feedback = 1;
			IM.hall_fb.feedback2++;
			IM.hall_fb.low_state = 0;
		}
	}
	return IM.hall_fb.feedback;
}

void Ice_Maker_Reset_Hall_Feedback(void){
	IM.hall_fb.feedback = 0;
}

void Ice_Maker_Heater_Config(void){
	if((IM.temperature <= GET_ICE_MAKER_HEATER_RUNNING_TEMP_MAX())){
		// Turn heater ON
		GPIOB -> ODR |= (1<<8);               // PB8 high
	}
	else{
		// Turn heater OFF
		GPIOB -> ODR &=~ (1<<8);               //PB8 low
	}
}

void Ice_Maker_Time_Based_Heater_Config(void){
	if(IM.flag.disp_based_im_state == 0){
		IM.timer.heater_run = 0;
	}
	if((IM.timer.heater_run >= 3) && (IM.timer.heater_run <= GET_ICE_MAKER_HEATER_RUN_TIME())){
		//Turn heater on
		GPIOB -> ODR |= (1<<8);               // PB8 high
	}
	else if(IM.timer.heater_run >= GET_ICE_MAKER_HEATER_RUN_TIME()){
		// Turn heater OFF
		GPIOB -> ODR &=~ (1<<8);               //PB8 low
		if(IM.timer.heater_run >= (GET_ICE_MAKER_HEATER_RUN_TIME()*2)){
			IM.timer.heater_run = 0;
		}
	}
}

void Ice_Maker_Valve_On(void){
	GPIOC -> ODR |= (1<<14);
}

void Ice_Maker_Valve_Off(void){
	GPIOC -> ODR &=~ (1<<14);
}

void Ice_Maker_Valve_Config(void){
	//IM.flag.valve_on = 1;                // will be 1 when (IM.timer.after_reset >= 5)
	if(IM.flag.init_ccw_rotate_result != 1){         // If not returned from mid
		if((IM.timer.valve > 0) && (IM.timer.valve <= (GET_ICE_MAKER_VALVE_ON_TIME()))){
			Ice_Maker_Valve_On();
		}
		if((IM.timer.valve > (GET_ICE_MAKER_VALVE_ON_TIME())) && (IM.timer.valve <= (GET_ICE_MAKER_VALVE_OFF_TIME()+GET_ICE_MAKER_VALVE_ON_TIME()))){
			Ice_Maker_Valve_Off();
		}
		if((IM.timer.valve >= (GET_ICE_MAKER_VALVE_OFF_TIME()+GET_ICE_MAKER_VALVE_ON_TIME() )) && (IM.valve_on_cycle_count < GET_ICE_MAKER_VALVE_ON_CYCLE())){
			IM.timer.valve = 0;
		}
	}
	if(IM.timer.valve >= (GET_ICE_MAKER_VALVE_OFF_TIME()+ GET_ICE_MAKER_VALVE_ON_TIME() + GET_ICE_MAKER_AFTER_VALVE_TIME())){
		IM.flag.valve_filled = 1;
	}
	else {
		IM.flag.valve_filled = 0;
	}
}

uint8_t Ice_Maker_Valve_On_Operation(void){
	uint8_t Valve_on_Operation = 0;
	if((IM.timer.valve > 0) && (IM.timer.valve <= (GET_ICE_MAKER_VALVE_ON_TIME() + GET_ICE_MAKER_VALVE_OFF_TIME()+1))){
		Valve_on_Operation = 1;
	}
	else{
		Valve_on_Operation = 0;
	}
	return Valve_on_Operation;
}

// Check whether ice is ready or not
void Ice_Maker_Ice_Temp_Check(void){
	if(IM.temperature >= GET_ICE_MAKER_SET_ICE_TEMP()){
		IM.flag.ice_tray_rot_op = 1;
	}
}

uint8_t Ice_Maker_Test_Button(void){
	if (!(GPIOD->IDR & 0b00100)){
		IM.button.low_state = 1;
	}
	if(IM.button.low_state == 1){
		if ((GPIOD->IDR & 0b00100)){
			IM.button.pressed = 1;
			IM.button.low_state = 0;
		}
	}
	return IM.button.pressed;
}

void Ice_Maker_Reset_Test_Button(void){
	IM.button.pressed = 0;
}

enum motor_rotation_statement{
	Undone = 0,
	Done = 1
};

void Ice_Maker_CCW_End_Detection(void){
	if(IM.timer.init_or_conf_rotation <= 1){
		if(IM.flag.init_ice_tray_ccw_end_det_break == 0){
			if((Ice_Maker_Hall_Feedback_checker() == 0)){
				IM.flag.init_ice_tray_not_at_ccw_end = 1;
				IM.flag.init_ice_tray_ccw_end_det_break = 1;
			}
			else{
				IM.flag.init_ice_tray_not_at_ccw_end = 0;
			}
		}
	}
	else if(IM.timer.init_or_conf_rotation > 1){
		if(IM.flag.init_ice_tray_not_at_ccw_end == 0){
			IM.timer.init_or_conf_rotation = 9;
			//IM.timer.non_blocker = 2;
			//UART_Transmit_Text("only one");
			IM.flag.init_ice_tray_not_at_ccw_end = 3;
		}
	}
}

uint8_t Timebased_temp = 0;
void Ice_Maker_Timebase_CCW_End_Detection(void){
	if(Timebased_temp != 1){
		Timebase_DownCounter_SS_Set_Securely(3,800);
		Timebased_temp = 1;
	}
	if(!(Timebase_DownCounter_SS_Expired_Event(3))){
		if(IM.flag.init_ice_tray_ccw_end_det_break == 0){
			if((Ice_Maker_Hall_Feedback_checker() == 0)){
				IM.flag.init_ice_tray_not_at_ccw_end = 1;
				IM.flag.init_ice_tray_ccw_end_det_break = 1;
			}
			else{
				IM.flag.init_ice_tray_not_at_ccw_end = 0;
			}
		}
	}
	else if(Timebase_DownCounter_SS_Expired_Event(3)){
		if(IM.flag.init_ice_tray_not_at_ccw_end == 0){
			IM.timer.init_or_conf_rotation = 9;
			//IM.timer.non_blocker = 2;
			UART_Transmit_Text("only one");
			IM.flag.init_ice_tray_not_at_ccw_end = 3;
		}
	}
}

void Ice_Maker_Reset_All_Parmaeters(void){                          // To reset all the parameter after a cycle
	//IM.timer.on_time = 0;
	IM.timer.from_ccw_mid = 0;
	IM.timer.ic_full_wait = 0;
	IM.flag.ice_tray_rot_op = 0;
	IM.timer.valve = 0;
	IM.valve_on_cycle_count = 0;
	
  IM.rotation.step0 = 0;
	IM.rotation.step1 = 0;
	IM.rotation.step2 = 0;
	IM.rotation.step3 = 0;
	IM.rotation.step4 = 0;
	
	IM.timer.cw_rotation = 0;
	IM.flag.cw_rotation_timer = 0;
	IM.flag.init_ccw_rotate_result = 0;
	//Ice_Maker_Reset_Test_Button();
	
	IM.flag.valve_on = 0;
	IM.flag.ic_full_wait_time = 0;
	
	IM.timer.after_reset = 0;
	IM.flag.after_reset_timer = 1;
	
	if(IM.lever_feedback == 0){
		IM.flag.ice_tray = 0;                                   // Ice box is full
		IM.flag.ice_container = 0;                              // Ice container is full
		IM.flag.valve_filled = 0;
	}
	else if(IM.lever_feedback == 1){
		IM.flag.ice_tray = 1;                                       // Ice box is full
		IM.flag.ice_container = 1;                                  // Ice container is full
		IM.flag.valve_filled = 1;
	}
}

void Ice_Maker_Init_Rotation_Reset(void){
	IM.init_rotate.step0 = 0;
	IM.init_rotate.step1 = 0;
	IM.init_rotate.step2 = 0;
	IM.init_rotate.step3 = 0;
	IM.init_rotate.step4 = 0;
	IM.timer.init_or_conf_rotation = 0;
	IM.flag.ice_tray_init_or_conf_rotation = 0;
	IM.flag.init_ccw_rotate_timer = 0;
	IM.timer.from_init_ccw_rotation = 0;
	IM.timer.after_reset = 0;
	IM.flag.after_reset_timer = 1;
}

uint8_t Timebase_startup_temp = 0;

uint8_t IM_rotation_busy = 0; // Flag that will prevent user from custom rotation from configure window, if it rotates following regular ice maker algorithm

void Ice_Maker_Ice_Tray_Rotation_Init(void){
	IM.flag.ice_tray_init_or_conf_rotation = 1;
	Ice_Maker_CCW_End_Detection();
	//Ice_Maker_Timebase_CCW_End_Detection();
	
	if((IM.flag.ice_tray_init_or_conf_rotation == 1) && (IM.init_rotate.step0 == Undone)){
		//uart_send_char("  rotation start  ");
		IM_rotation_busy = 1;
		Ice_Maker_Motor_Control(Forward);
		IM.init_rotate.step0 = Done;
	}
	
	if((IM.init_rotate.step0 == Done)&&(IM.init_rotate.step1 == Undone)){
		if((Ice_Maker_Hall_Feedback_Checker_RT() == 1) || (IM.timer.init_or_conf_rotation >= 9)){
			Ice_Maker_Motor_Control(Motor_off);
		  IM.flag.non_bl_timer = 1;            // A delay or waiting is needed at the end of initial rotation CW end
			if(IM.timer.non_blocker >= 2){              // If the delay timer (wait time) crosses 2 seconds, make IM.init_rotate.step1 done so that it can go now to next step. All other parameters are set here
				IM.init_rotate.step1 = Done;
				Ice_Maker_Reset_Hall_Feedback();
				IM.flag.non_bl_timer = 0;
				IM.timer.non_blocker = 0;
			}
			if(Timebase_startup_temp != 1){
				Timebase_DownCounter_SS_Set_Securely(4,500);
				Timebase_startup_temp = 1;
			}
			if(Timebase_DownCounter_SS_Expired_Event(4)){
				IM.init_rotate.step1 = Done;
				Ice_Maker_Reset_Hall_Feedback();
				IM.flag.non_bl_timer = 0;
				IM.timer.non_blocker = 0;
			}
		}
	}
	if((IM.init_rotate.step1 == Done)&&(IM.init_rotate.step2 == Undone)){
		Ice_Maker_Motor_Control(Reverse);                           // Turn a flag IM.flag.init_ccw_rotate_timer on to count IM.timer.from_init_ccw_rotation time for this reverse rotation starts
		IM.init_rotate.step2 = Done;
		IM.flag.init_ccw_rotate_timer = 1;
		Ice_Maker_Reset_Hall_Feedback();
		Timebase_startup_temp = 0;                        // To start the time base timer again which is used in previous step
	}
	if((IM.init_rotate.step2 == Done)&&(IM.init_rotate.step3 == Undone)){
		if((Ice_Maker_Hall_Feedback_Checker_RT() == 1) || (IM.timer.init_or_conf_rotation >= 18)){       // Got CCW MID HFB
			if(IM.timer.from_init_ccw_rotation > 3){
				Ice_Maker_Motor_Control(Reverse);                       // If this hall feedback time IM.timer.from_init_ccw_rotation is less than 3 second, than , it should be Ice_Maker_Motor_Control(Motor_off); else if greater than 3 seconds it should be Ice_Maker_Motor_Control(Reverse);
			}
			else if(IM.timer.from_init_ccw_rotation <= 3){
				Ice_Maker_Motor_Control(Motor_off);
				IM.flag.init_ccw_rotate_result = 1;
			}
			//uart_send_char("  Got CCW mid  ");
			IM.flag.init_ccw_rotate_timer = 0;
			IM.init_rotate.step3 = Done;
			Ice_Maker_Reset_Hall_Feedback();
		}
	}
	if((IM.init_rotate.step3 == Done)&&(IM.init_rotate.step4 == Undone)){
		if((Ice_Maker_Hall_Feedback_Checker_RT() == 1)|| (IM.timer.init_or_conf_rotation >= 21) || (IM.timer.from_init_ccw_rotation <= 3)){
			Ice_Maker_Motor_Control(Motor_off);
			IM.init_rotate.step4 = Done;
			Ice_Maker_Reset_Hall_Feedback();
			//uart_send_char("  Got CCW end  ");
			Ice_Maker_Init_Rotation_Reset();
			IM.flag.im_module_op = 1;
			IM.flag.init_rotation_done = 1;
			IM_rotation_busy = 0;
		}
	}
}

void Ice_Maker_Module_Flag_Reset(void){
	IM.flag.ice_tray = 0;                                   
	IM.flag.ice_container = 0;                              
	IM.flag.valve_filled = 0;
}

void Ice_Maker_Reset_All_Parameter(void){
	Ice_Maker_Init_Rotation_Reset();
	Ice_Maker_Reset_All_Parmaeters();
	Ice_Maker_Module_Flag_Reset();
	Ice_Maker_Valve_Off();
}

uint8_t IM_on_conf_rotation = 0;
void Ice_Maker_Config_Rotation(void){
	IM.flag.ice_tray_init_or_conf_rotation = 1;
	Ice_Maker_CCW_End_Detection();                // THIS FUNCTION SHOULD BE COMMENTED OR IGNORED
	//Ice_Maker_Timebase_CCW_End_Detection();
	if((IM.flag.ice_tray_init_or_conf_rotation == 1) && (IM.init_rotate.step0 == Undone)){
		//uart_send_char("  rotation start  ");
		Ice_Maker_Motor_Control(Forward);
		IM.init_rotate.step0 = Done;
		IM_on_conf_rotation = 1;
	}
	if((IM.init_rotate.step0 == Done)&&(IM.init_rotate.step1 == Undone)){
		if((Ice_Maker_Hall_Feedback_Checker_RT() == 1) || (IM.timer.init_or_conf_rotation >= 9)){
			Ice_Maker_Motor_Control(Motor_off);
		  IM.flag.non_bl_timer = 1;            // A delay or waiting is needed at the end of initial rotation CW end
			if(IM.timer.non_blocker >= 2){              // If the delay timer (wait time) crosses 2 seconds, make IM.init_rotate.step1 done so that it can go now to next step. All other parameters are set here
				IM.init_rotate.step1 = Done;
				Ice_Maker_Reset_Hall_Feedback();
				IM.flag.non_bl_timer = 0;
				IM.timer.non_blocker = 0;
			}
			if(Timebase_startup_temp != 1){
				Timebase_DownCounter_SS_Set_Securely(4,500);
				Timebase_startup_temp = 1;
			}
			if(Timebase_DownCounter_SS_Expired_Event(4)){
				IM.init_rotate.step1 = Done;
				Ice_Maker_Reset_Hall_Feedback();
				IM.flag.non_bl_timer = 0;
				IM.timer.non_blocker = 0;
			}
		}
	}
	if((IM.init_rotate.step1 == Done)&&(IM.init_rotate.step2 == Undone)){
		Ice_Maker_Motor_Control(Reverse);                           // Turn a flag IM.flag.init_ccw_rotate_timer on to count IM.timer.from_init_ccw_rotation time for this reverse rotation starts
		IM.init_rotate.step2 = Done;
		IM.flag.init_ccw_rotate_timer = 1;
		Ice_Maker_Reset_Hall_Feedback();
		Timebase_startup_temp = 0;                        // To start the time base timer again which is used in previous step
	}
	if((IM.init_rotate.step2 == Done)&&(IM.init_rotate.step3 == Undone)){
		if((Ice_Maker_Hall_Feedback_Checker_RT() == 1) || (IM.timer.init_or_conf_rotation >= 18)){       // Got CCW MID HFB
			if(IM.timer.from_init_ccw_rotation > 3){
				Ice_Maker_Motor_Control(Reverse);                       // If this hall feedback time IM.timer.from_init_ccw_rotation is less than 3 second, than , it should be Ice_Maker_Motor_Control(Motor_off); else if greater than 3 seconds it should be Ice_Maker_Motor_Control(Reverse);
			}
			else if(IM.timer.from_init_ccw_rotation <= 3){
				Ice_Maker_Motor_Control(Motor_off);
				//IM.flag.init_ccw_rotate_result = 1;
			}
			//uart_send_char("  Got CCW mid  ");
			IM.flag.init_ccw_rotate_timer = 0;
			IM.init_rotate.step3 = Done;
			Ice_Maker_Reset_Hall_Feedback();
		}
	}
	if((IM.init_rotate.step3 == Done)&&(IM.init_rotate.step4 == Undone)){
		if((Ice_Maker_Hall_Feedback_Checker_RT() == 1)|| (IM.timer.init_or_conf_rotation >= 21) || (IM.timer.from_init_ccw_rotation <= 3)){
			Ice_Maker_Motor_Control(Motor_off);
			if(IM.timer.init_or_conf_rotation >= 23){
				IM.init_rotate.step4 = Done;
				Ice_Maker_Reset_Hall_Feedback();
				Ice_Maker_Init_Rotation_Reset();
				IM_on_conf_rotation = 0;
			}
		}
	}
}

uint8_t Ice_Maker_Ice_Tray_On_Config_Rotation(void){
	return IM_on_conf_rotation;
}

void Ice_Maker_Ice_Tray_Rotation(void){
	if((IM.flag.ice_tray_rot_op == 1)&&(IM.rotation.step0 == Undone)){   // The rotation starts with the Ice_tray_rotation_operation = 1 and the IM.rotation.step0
		IM.flag.cw_rotation_timer = 1;
		//uart_send_char("  rotation start  ");       // IM.rotation.step0 to start the rotation at CW direction
		Ice_Maker_Motor_Control(Forward);
		IM.rotation.step0 = Done;
		IM_rotation_busy = 1;
	}
	if((IM.rotation.step0 == Done)&&(IM.rotation.step1 == Undone)){       // IM.rotation.step1 is to stop the Motor at CW end and wait for 3 seconds
		if((Ice_Maker_Hall_Feedback_Checker_RT() == 1)||(IM.timer.cw_rotation >= 10)){  // Check the hall fb at CW end , if somehow misses th HFB check for the time limit (IM.timer.cw_rotation >= 10)
			Ice_Maker_Motor_Control(Motor_off);                 // If the HALL FEEDBACK is presented, Turn motor OFF at CW end
			//uart_send_char("  CW End  ");
			IM.flag.non_bl_timer = 1;            // A delay or waiting is needed at the end of CW end
			if(IM.timer.non_blocker >= 2){              // If the delay timer (wait time) crosses 2 seconds, make IM.rotation.step1 done so that it can go now to next step. All other parameters reset are done here
				IM.rotation.step1 = Done;
				Ice_Maker_Reset_Hall_Feedback();
				IM.flag.non_bl_timer = 0;
				IM.timer.non_blocker = 0;
			}  
			if(Timebase_startup_temp != 1){
				Timebase_DownCounter_SS_Set_Securely(4,300);
				Timebase_startup_temp = 1;
			}
			
			if(Timebase_DownCounter_SS_Expired_Event(4)){
				IM.rotation.step1 = Done;
				Ice_Maker_Reset_Hall_Feedback();
				IM.flag.non_bl_timer = 0;
				IM.timer.non_blocker = 0;
			}
		}
	}
	
	if((IM.rotation.step1 == Done)&&(IM.rotation.step2 == Undone)){        // IM.rotation.step2 for calculating the LEVER FEEDBACK status and then rotating the motor to CCW direction from CW end
		if(IM.timer.cw_rotation <= (4+1)){              // if reach at cw dircetion before 4 second and then wait for 1 seconds ((CW rotation time + CW end wait time)), got the LEVER FEEDBACK
			IM.lever_feedback = 1;                        // Got the lever feedback as reached before 4 seconds and waited for 3 seconds
			//uart_send_char("  got lever feedback  ");
		}
		else if(IM.timer.cw_rotation > (4+1)){          // if reach at cw dircetion after 4 second and then wait for 1 seconds , have not got LEVER FEEDBACK ; HERE, 1 SECONDS OF WAIT TIME OR DELAY TIME IS FIXED, REST WILL BE CW ROTATION TIME(EXPECTED LESS THAN 4 SECONDS)
			IM.lever_feedback = 0;                        // Have not got the lever feedback as reached after 4 seconds and than waited for 2 seconds
			//uart_send_char(" havnt got lever fb");
		}
		Ice_Maker_Motor_Control(Reverse);                      // Rotate the motor from CW end to CCW direction
		IM.rotation.step2 = Done;
		Ice_Maker_Reset_Hall_Feedback();
		Timebase_startup_temp = 0;                   // To start the time base timer again which is used in previous step
	}
	
	if((IM.rotation.step2 == Done)&&(IM.rotation.step3 == Undone)){        // Step3 works based on the result of IM.lever_feedback
		if(IM.lever_feedback == 0){                     // If IM.lever_feedback is not presented, It started rotating from CW end.so, wait for CCW mid HFB and make IM.rotation.step3 done
			if((Ice_Maker_Hall_Feedback_Checker_RT() == 1)||(IM.timer.cw_rotation >= 16)){       // Check for CCW MID HFB , if somehow misses the hall fb, consider the time limit IM.timer.cw_rotation >= 18
				Ice_Maker_Motor_Control(Reverse);
				//uart_send_char("  Got CCW mid  ");
				IM.rotation.step3 = Done;
			}
		}
		else if(IM.lever_feedback == 1){                // If IM.lever_feedback is presented, No need to wait for the HALL FB from CCW mid and just make the IM.rotation.step3 done. and it will rotate back at CCW direction from CW mid
			IM.rotation.step3 = Done;
			//uart_send_char("  FROM MIDPOINT ");
		}
		Ice_Maker_Reset_Hall_Feedback();
	}
	
	if((IM.rotation.step3 == Done)&&(IM.rotation.step4 == Undone)){         // Step4 waits for CCW end HFB, if at CCW end, HIGH pulse of hall feedback found, just stop the motor and RESET all parameter.
		if((Ice_Maker_Hall_Feedback_Checker_RT() == 1)||(IM.timer.cw_rotation >= 21)){
			Ice_Maker_Motor_Control(Motor_off);
			IM.rotation.step4 = Done;
			Ice_Maker_Reset_Hall_Feedback();			
			Ice_Maker_Reset_All_Parmaeters();                      // Reset all the parameters and as THIS IS THE END OF THE CYCLE
			IM_rotation_busy = 0;
		}
	}
}

uint8_t Ice_Maker_Ice_Tray_On_Operation(void){
	return IM_rotation_busy;
}

void Ice_Maker_Debug_Data(void){
	if(GlobalVars_Get_Start_Flag() && (GlobalVars_Get_Thread_Function() == 0) ){
	  if(GlobalVars_Get_Curr_Thread() == 0){
			UART_PrintfD_NL ( "IM_Valve_on_time ",IM.timer.valve);
		}
		else if(GlobalVars_Get_Curr_Thread() == 1){
			UART_PrintfD_NL ( "IM_Valve_filled_flag ",IM.flag.valve_filled);
		}
		else if(GlobalVars_Get_Curr_Thread() == 2){
			UART_PrintfD_NL ( "IM_temperature ",IM.temperature);
		}
		else if(GlobalVars_Get_Curr_Thread() == 3){
			UART_PrintfD_NL ( "IM_Motor_direction ",IM.motor_direction);
		}
		else if(GlobalVars_Get_Curr_Thread() == 4){
			UART_PrintfD_NL ( "IM_Ice_tray_rot_op_flag ",IM.flag.ice_tray_rot_op);
		}
		else if(GlobalVars_Get_Curr_Thread() == 5){
			UART_PrintfD_NL ( "IM_CW_rotation_timer ",IM.timer.cw_rotation);
		}
		else if(GlobalVars_Get_Curr_Thread() == 6){
			UART_PrintfD_NL ( "IM_Raw_hall_feedback ",Ice_Maker_Hall_Feedback_checker());
		}
		else if(GlobalVars_Get_Curr_Thread() == 7){
			UART_PrintfD_NL ( "IM_Hall_feedback_status ",Ice_Maker_Hall_Feedback_Checker_RT());
		}
		else if(GlobalVars_Get_Curr_Thread() == 8){
			UART_PrintfD_NL ( "IM_Hall_feedback_update ",IM.hall_fb.feedback2);
		}
		else if(GlobalVars_Get_Curr_Thread() == 9){
			UART_PrintfD_NL ( "IM_Lever_feedback ",IM.lever_feedback);
		}
		else if(GlobalVars_Get_Curr_Thread() == 10){
			UART_PrintfD_NL ( "IM_IC_full_wait_time ",IM.timer.ic_full_wait);
		}
		else if(GlobalVars_Get_Curr_Thread() == 11){
			UART_PrintfD_NL ( "IM_Init_rot_time_count ",IM.timer.init_or_conf_rotation);
		}
		
		//Extrass
		else if(GlobalVars_Get_Curr_Thread() == 12){
			UART_PrintfD_NL ( "IM_on_time ",IM.timer.on_time);
		}
		else if(GlobalVars_Get_Curr_Thread() == 13){
			UART_PrintfD_NL ( "IM_Testbutton ",Ice_Maker_Test_Button());
		}
	  else if(GlobalVars_Get_Curr_Thread() == 14){
			UART_PrintfD_NL ( "IM_After_rot_time_count ",IM.timer.after_reset);
		}
		else if(GlobalVars_Get_Curr_Thread() == 15){
			UART_PrintfD_NL ( "IM_Valve_on_flag ",IM.flag.valve_on);
		}
		else if(GlobalVars_Get_Curr_Thread() == 16){
			UART_PrintfD_NL ( "IM_IC_full_wait_time_flag ",IM.flag.ic_full_wait_time);
		}
		else if(GlobalVars_Get_Curr_Thread() == 17){
			UART_PrintfD_NL ( "IM_Ice_tray_rot_init_flag ",IM.flag.ice_tray_init_or_conf_rotation);
		}
		else if(GlobalVars_Get_Curr_Thread() == 18){
			UART_PrintfD_NL ( "IM_initial_rotation ",IM.flag.init_rotation_done);
		}
		else if(GlobalVars_Get_Curr_Thread() == 19){
			UART_PrintfD_NL ( "IM_Ice_maker_module_status ",IM.flag.im_module_op);
			GlobalVars_Inc_Thread_Function();
		}
	  GlobalVars_Inc_Curr_Thread();
		//UART_Transmit_Text("\r\n");
	}
}

void Ice_Maker_Set_Reset_Ice_Maker_Module_Status( int8_t Imms){
	IM.flag.im_module_op = Imms;
}

void Ice_Maker_Get_Temperature_Value(void){
	NTC_Calculate_Single_Sample(0);
	IM.temperature = NTC_Get_Temperature(0);
}

void Ice_Maker_Ice_Making_Operation(void){
	if(IM.timer.debug_data >= 1){
		IM.timer.debug_data = 0;
	}	
	Ice_Maker_Test_Button();
	if(IM.flag.init_rotation_done == 0){
		if((IM.temperature >= GET_ICE_MAKER_SET_ICE_TEMP()) && (IM.timer.on_time >= 5)){
			Ice_Maker_Ice_Tray_Rotation_Init();
			//IM.flag.init_rotation_done = 1;
		}
	}
	if((IM.flag.im_module_op == 1)){
		if((IM.flag.ice_tray == 0)&&(IM.flag.ice_container == 0)){        // Step-1
				//IM.flag.valve_on = 1;                          //Inserted into Ice_Maker_Valve_Config() function
			if((IM.flag.valve_on == 1)){
				Ice_Maker_Valve_Config();
			}
		}
		else if ((IM.flag.ice_tray == 1)&&(IM.flag.ice_container == 1)){  // IB and IC are full. wait for a while 
			//IM.flag.ic_full_wait_time = 1;
			if(IM.timer.ic_full_wait <= GET_ICE_MAKER_IC_FULL_WAIT_TIME()){     // Wait till the defined time and make IM.flag.valve_filled = 0; Ice_operation = 0; to stop the turning ON of the valve and other process
				IM.flag.valve_filled = 0;
				IM.flag.ice_tray_rot_op = 0;
			}
			else if(IM.timer.ic_full_wait > GET_ICE_MAKER_IC_FULL_WAIT_TIME()){
				IM.flag.valve_filled = 1;                        // After the wait time ends make IM.flag.valve_filled = 1; to resume the process directly   skipping the valve to be turned ON
			}
		}
		if(IM.flag.valve_filled == 1){                       // Step-2
			Ice_Maker_Ice_Temp_Check();
		}
		
		if(IM.flag.ice_tray_rot_op == 1){             // Step-3
			Ice_Maker_Ice_Tray_Rotation();
		}
	}
	if(IM.timer.after_reset >= 5){
		if((!Ice_Maker_Ice_Tray_On_Config_Rotation()) && (!User_Config_Valve_on_User_Config()) && (!User_Config_User_In_Configuration_Mode())){
			IM.flag.valve_on = 1;
			IM.flag.ic_full_wait_time = 1;
		}
		else{
			IM.flag.valve_on = 0;
			IM.flag.ic_full_wait_time = 0;
		}
	}
}
//-------------------Code till this is for running ice maker by itself------------------//
//-------------------Code to make it compitible with display starts from here-----------//

void Ice_Maker_Disp_Comm_Based_Ice_Maker_Mode_Update_For_Heartbeat_Data(void){
		if((Disp_Comm_Master_Data_Get_Mode() & (1<<0)) == (1<<0)){
			Disp_Comm_Heartbeat_Set_Data(9,1,0);
		}
		else{
			Disp_Comm_Heartbeat_Set_Data(9,0,0);
		}
}

int8_t Ice_Maker_IM_On_Operation(void){
	int8_t Im_on_stat = 0;
	if((IM.timer.init_or_conf_rotation > 0) || (IM.timer.valve > 0) || (IM.timer.ic_full_wait > 0) || (IM.valve_on_cycle_count > 0) ){  // Put all the condition when the ice maker should not be turned off
		Im_on_stat = 1;
	}
	return Im_on_stat;
}

void Ice_Maker_Disp_Comm_Based_Ice_Maker_Mode(void){
			if((Disp_Comm_Master_Data_Get_Mode() & (1<<0)) == (1<<0)){
				IM.flag.disp_based_im_state = 1;
			}
			else if((Disp_Comm_Master_Data_Get_Mode() & (1<<0)) != (1<<0) && (Ice_Maker_IM_On_Operation() != 1)){
				IM.flag.disp_based_im_state = 0;              // Unomment this one to run ice maker based on Disp Command
				//IM.flag.disp_based_im_state = 1;                // Comment this one to run ice maker based on Disp Command 
			}
			if(IM.flag.disp_based_im_state == 1){
				Ice_Maker_Ice_Making_Operation();
			}
}

void Ice_Maker_Disp_Comm_Based_Update_Ice_Container_Full_Status_Bit(void){
	if(IM.flag.after_reset_timer == 1){
		/*
		if((IM.lever_feedback == 0)){
			// Cycle complete
			Disp_Comm_Heartbeat_Set_Data(8,1,0);
		}
		else{
			Disp_Comm_Heartbeat_Set_Data(8,0,0);
		}
		*/
		if((IM.lever_feedback == 1)){
			// Ice container if full
			Disp_Comm_Heartbeat_Set_Data(8,1,2);
		}
		else{
			Disp_Comm_Heartbeat_Set_Data(8,0,2);
		}
	}
}

void Ice_Maker_Disp_Comm_Based_Update_NTC_Error_Bit(void){
	if((IM.temperature == (-1000)) || ((IM.temperature == (-2000)))){
		Disp_Comm_Heartbeat_Set_Data(8,1,4);
	}
	else{
		Disp_Comm_Heartbeat_Set_Data(8,0,4);
	}
}

void Ice_Maker_Disp_Comm_Based_Ice_Maker_Handler(void){
	User_Config_Ice_Maker_Handler();
	Parameter_Get_Disp_Comm_Val();
	Ice_Maker_Get_Temperature_Value();
	Ice_Maker_Time_Based_Heater_Config();
	Ice_Maker_Disp_Comm_Based_Update_NTC_Error_Bit();
	Ice_Maker_Disp_Comm_Based_Ice_Maker_Mode_Update_For_Heartbeat_Data();
	Ice_Maker_Disp_Comm_Based_Ice_Maker_Mode();          
	Ice_Maker_Disp_Comm_Based_Update_Ice_Container_Full_Status_Bit();
}

/*Before putting into actual operation:
1. Parameters are set as required
2. Logic of two place for ice have been changed
3. Ice maker is running based on disp command (IM.flag.disp_based_im_state)
*/