#include "stm32g070xx.h"
#include "timer.h"
//#include "serial.h"

void timer_Init(uint16_t presclaller, uint16_t arr){
	RCC->APBENR2 |= RCC_APBENR2_TIM1EN;
	
	//TIM1->CR1 &=~ TIM_CR1_CEN;             //Timer1 enabled
	
	TIM1->CR1 |= TIM_CR1_CEN;                //Timer1 enabled
	
	TIM1->CR1 &=~ TIM_CR1_DIR;               //Timer1 as up-counter
	
	TIM1->CR1 &=~ TIM_CR1_CMS_0;             
	TIM1->CR1 &=~ TIM_CR1_CMS_1;             //Edge alligned mode
	
	TIM1->CR1 &=~ TIM_CR1_CKD_0;             
	TIM1->CR1 &=~ TIM_CR1_CKD_1;             //Clock division used by dead time generator and filter
	
	TIM1->CR1 |= TIM_CR1_ARPE;               // Change to ARR register will take action immediatly
	
	TIM1->PSC = presclaller;
	
	TIM1->CCMR1 &=~  TIM_CCMR1_IC1F_0;
	TIM1->CCMR1 |=  TIM_CCMR1_IC1F_1;
	TIM1->CCMR1 |=  TIM_CCMR1_IC1F_2;
	TIM1->CCMR1 &=~  TIM_CCMR1_IC1F_3;
	TIM1->ARR   = arr;
	
	TIM1->DIER |= TIM_DIER_UIE;
	NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 12);
	NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
}

uint32_t timer1_ouput(void){
	return (TIM1->CNT);
}







void TIM1_IRQHandler(void){
	  TIM1->SR &=~ TIM_SR_UIF;
}






// An unusual blocking delay for emergency use case
/*void delay(uint16_t ind){
		for(uint32_t i=0;i<(735*ind);i++){
			__NOP();
		}
}

*/






















