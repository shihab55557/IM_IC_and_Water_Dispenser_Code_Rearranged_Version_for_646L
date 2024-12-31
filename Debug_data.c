#include "stm32g070xx.h"
#include "Debug_data.h"
#include "Uart.h"
#include "timer.h"

static volatile int16_t Debug_data_send_timer = 0;

/*

void TIM1_BRK_UP_TRG_COM_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_UIF) {
				TIM1->SR &= ~TIM_SR_UIF;
			
				
				Debug_data_send_timer++;
			Print_Debug_Data();
    }
}
*/




void Print_Debug_Data(void){
	UART_PrintfD_NL ( "timer ",Debug_data_send_timer);
	UART_PrintfD_NL ( "HFB ",Hall_Feedback_checker_d());
}



int Hall_Feedback_checker_d(void){                          // This function checks the raw hall feedback checking the input GPIO pin
	int Hall_Fb = 0;         // Hall feedback
		if ((GPIOD->IDR & 0b01000)){  // At higher state
			Hall_Fb = 1;
		}
		else {
			Hall_Fb = 0;
		}
	return Hall_Fb;
}

void Motor_Gpio(void){
	
	//GPIOB -> ODR |= (1<<13);              
	//GPIOB -> ODR |= (1<<14);   
	

	
	if (Debug_data_send_timer >= 9){
		GPIOC -> ODR |= (1<<14);      // For valve on
		GPIOB -> ODR |= (1<<8);       // For heater on
		
	}
	
	
	
}


void Debug_Fnc(void){
	Motor_Gpio();
}


void IceMaker_GPIO_Init(void){
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
		RCC->IOPENR |= RCC_IOPENR_GPIOCEN;    	//GPIOB clock En
		GPIOC->MODER  &=~ GPIO_MODER_MODE14_1; 
		GPIOC->MODER  |= GPIO_MODER_MODE14_0;  	//make PC14 as GPIO output
		GPIOC->OTYPER  &=~ GPIO_OTYPER_OT14;
		GPIOC -> ODR &=~ (1<<14);                //make PC14 low by default
		
		// For heater control
		GPIOB->MODER  &=~ GPIO_MODER_MODE8_1; 
		GPIOB->MODER  |= GPIO_MODER_MODE8_0;  	//make PB8 as GPIO output
 }





