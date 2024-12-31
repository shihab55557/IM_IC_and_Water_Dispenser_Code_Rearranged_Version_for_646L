

#include "stm32g070xx.h"
#include "disp.h"

#define  DISP_UART_TX_EN
#define  DISP_UART_RX_EN
#define  DISP_UART_RX_INT_EN
#define  DISP_UART_BUF_SIZE 128
#define  DISP_UART_PCKT_COMP_DELAY 50

typedef struct uart_t{
  volatile uint8_t Error;
  uint8_t          Digits[8];
  uint8_t          InputNumberDigits;
  volatile uint8_t Buffer[DISP_UART_BUF_SIZE];
  volatile uint8_t BufferIndex;
	volatile uint8_t TimerEnabled;
	volatile uint8_t DataAvailable;
}uart_t;

uart_t DispUart;

void Disp_UART_Struct_Init(void){
	DispUart.Error = 0;
	for(uint8_t i=0;i<8;i++){
	  DispUart.Digits[i] = 0;
	}
  DispUart.InputNumberDigits = 0;
	for(uint8_t i=0;i<DISP_UART_BUF_SIZE;i++){
	  DispUart.Buffer[i] = 0;
	}
	DispUart.BufferIndex = 0;
	DispUart.TimerEnabled = 0;
	DispUart.DataAvailable = 0;
}
	
	
void Disp_UART_GPIO_Init(void){
  if((RCC->IOPENR & RCC_IOPENR_GPIOBEN)!=RCC_IOPENR_GPIOBEN){
     RCC->IOPENR|=RCC_IOPENR_GPIOBEN;
  }
	//Alternate Function Mode
	GPIOB->MODER  &=~ GPIO_MODER_MODE6_Msk;
	GPIOB->MODER  &=~ GPIO_MODER_MODE7_Msk;
	GPIOB->MODER  |=  GPIO_MODER_MODE6_1;
	GPIOB->MODER  |=  GPIO_MODER_MODE7_1;
	
	//Select ALT func
	GPIOB->AFR[0] &=~ GPIO_AFRL_AFSEL6_Msk;
	GPIOB->AFR[0] &=~ GPIO_AFRL_AFSEL7_Msk;
	
	//Pull-up to RX Pin
	GPIOB->PUPDR  &=~ GPIO_PUPDR_PUPD7_Msk;
	GPIOB->PUPDR  |= GPIO_PUPDR_PUPD7_0;
}

void Disp_UART_Clock_Init(void){
	RCC->APBENR2|=RCC_APBENR2_USART1EN;
	RCC->CCIPR&=~RCC_CCIPR_USART1SEL_Msk;
	RCC->CCIPR|=RCC_CCIPR_USART1SEL_1;	
}

void Disp_UART_Set_BAUD_Rate(uint32_t baud){
	if(USART1->CR1 & USART_CR1_UE){
		USART1->CR1&=~USART_CR1_UE;
	}
	
	//USART1->CR2|=USART_CR2_SWAP;      /*Tx & Rx pin swap*/
	USART1->BRR=(uint16_t)(16000000/baud);
	if((USART1->CR1 & USART_CR1_UE)!=USART_CR1_UE){
		 USART1->CR1|=USART_CR1_UE;
	}
}

void Disp_UART_TX_Config(void){
	USART1->CR1|=USART_CR1_TE;
	if((USART1->CR1 & USART_CR1_UE)!=USART_CR1_UE){
		 USART1->CR1|=USART_CR1_UE;
	}
}

void Disp_UART_RX_Config(void){
	USART1->CR1|=USART_CR1_RE;
	if((USART1->CR1 & USART_CR1_UE)!=USART_CR1_UE){
		 USART1->CR1|=USART_CR1_UE;
	}
}

void Disp_UART_RX_Int_Config(void){
	//FIFO disable
	USART1->CR1 &=~USART_CR1_FIFOEN;
	USART1->CR1 |= USART_CR1_RXNEIE_RXFNEIE;
	//add interrupt
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, 0);
}



//Dedicated Timer for Data Packet End Detection
void Disp_UART_Timer_Init(void){
	if( (RCC->APBENR2 & RCC_APBENR2_TIM14EN) != RCC_APBENR2_TIM14EN){
		RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
	}
	TIM14->CR1 |= TIM_CR1_ARPE;
	//Prescaler 16000, to get 1ms
	TIM14->PSC  = 15999;
	//TIM14->ARR  = 0xFFFF;
	TIM14->ARR  = DISP_UART_PCKT_COMP_DELAY;
	TIM14->DIER|= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM14_IRQn);
	NVIC_SetPriority(TIM14_IRQn, 1);
}

void Disp_UART_Timer_Enable(void){
	TIM14->CR1 |= TIM_CR1_CEN;
	DispUart.TimerEnabled = 1;
}

void Disp_UART_Timer_Disable(void){
	TIM14->CR1 &=~ TIM_CR1_CEN;
	DispUart.TimerEnabled = 0;
}

uint8_t Disp_UART_Get_Timer_Status(void){
	return DispUart.TimerEnabled;
}

uint16_t Disp_UART_Get_Counter_Val(void){
	return TIM14->CNT;
}


void Disp_UART_Counter_Reset(void){
	TIM14->CNT = 0;
}


void TIM14_IRQHandler(void){
	if(DispUart.TimerEnabled == 1){
	  Disp_UART_Timer_Disable();
	}
	if(Disp_UART_Get_Buf_Index() > 0){
		DispUart.DataAvailable = 1;
	}
	else{
		DispUart.DataAvailable = 0;
	}
	TIM14->SR &=~ TIM_SR_UIF;
}










void Disp_UART_Init(uint32_t baud){
	Disp_UART_Struct_Init();
	Disp_UART_Timer_Init();
  Disp_UART_GPIO_Init();
  Disp_UART_Clock_Init();
  Disp_UART_Set_BAUD_Rate(baud);
  #ifdef DISP_UART_TX_EN
  Disp_UART_TX_Config();
	#endif
  
  #ifdef DISP_UART_RX_EN
  Disp_UART_RX_Config();
  #endif
	
	#ifdef DISP_UART_RX_INT_EN
  Disp_UART_RX_Int_Config();
  #endif
}



void Disp_UART_Transmit_Byte(uint8_t val){
	USART1->TDR=val;
	while((USART1->ISR & USART_ISR_TC)!=USART_ISR_TC);
	USART1->ICR|=USART_ICR_TCCF;
}


void USART1_IRQHandler(void){
	if(USART1->ISR & USART_ISR_RXNE_RXFNE){
		volatile uint32_t temp = USART1->RDR;
	  DispUart.Buffer[DispUart.BufferIndex] = (uint8_t)temp;
		DispUart.BufferIndex++;
		if(DispUart.BufferIndex >= DISP_UART_BUF_SIZE){
			DispUart.BufferIndex = 0;
		}
		
		
		if(DispUart.TimerEnabled == 0){
			Disp_UART_Counter_Reset();
			Disp_UART_Timer_Enable();
		}
		Disp_UART_Counter_Reset();
	}
}






//Utility functions

void Disp_UART_Tx_Buf(uint8_t *buf, uint16_t len){
	for(uint16_t i=0; i<len; i++){
		Disp_UART_Transmit_Byte(buf[i]);
	}
}



void Disp_UART_Tx_Hex(uint32_t val){
  uint16_t hex_digit, index=0, loop_counter=0;
  if(val <= 0xFF){
    index=8;
    loop_counter=2;
  }else if(val <= 0xFFFF){
    index=16;
    loop_counter=4;     
  }else{
    index=32;
    loop_counter=8;
  }
  Disp_UART_Transmit_Byte('0');
  Disp_UART_Transmit_Byte('x');
	for(uint8_t i=0;i<loop_counter;i++){
	  index-=4;
	  hex_digit=(uint8_t)((val>>index) & 0x0F);
	  if(hex_digit>9){
	    hex_digit+=55;
	  }else {
	    hex_digit+=48;
	  }
	  Disp_UART_Transmit_Byte((uint8_t)hex_digit);
	}
}

void Disp_UART_Tx_Bin(uint32_t val){
  uint8_t loop_counter=0;
  if(val <= 0xFF){
    loop_counter=7;
  }else if(val <= 0xFFFF){
    loop_counter=15;     
  }else{
    loop_counter=31;
  }
  
  Disp_UART_Transmit_Byte('0');
  Disp_UART_Transmit_Byte('b');
  for(int i=loop_counter; i>=0; i--){
    if( (val>>i) & 1){
      Disp_UART_Transmit_Byte( 49 );   
    }else{
      Disp_UART_Transmit_Byte( 48 );         
    }
  }
}


void Disp_UART_Tx_NL(void){
  Disp_UART_Transmit_Byte('\r');
  Disp_UART_Transmit_Byte('\n');
}

void Disp_UART_Tx_SP(void){
  Disp_UART_Transmit_Byte(' ');
}

void Disp_UART_Tx_CM(void){
  Disp_UART_Transmit_Byte(',');
}





void Disp_UART_Tx_Text(char *str){
    uint8_t i=0;
    while(str[i]!='\0'){
        Disp_UART_Transmit_Byte(str[i]);
        i++;
    }
}

void Disp_UART_Tx_Text_NL(char *str){
  Disp_UART_Tx_Text(str);
  Disp_UART_Tx_NL();
}

void Disp_UART_Tx_Text_SP(char *str){
  Disp_UART_Tx_Text(str);
  Disp_UART_Tx_SP();
}

void Disp_UART_Tx_Text_CM(char *str){
  Disp_UART_Tx_Text(str);
  Disp_UART_Tx_CM();
}




void Disp_UART_Determine_Digit_Numbers(uint32_t num){
  uint8_t i=0;
  if(num==0){
    DispUart.Digits[0]=0;
    DispUart.InputNumberDigits=1;
  }else{
    while(num!=0){
      DispUart.Digits[i]=num%10;
      num/=10;
      i++;
    }
	DispUart.InputNumberDigits=i;
  }
}


void Disp_UART_Tx_Number_Digits(void){
  for(uint8_t i=DispUart.InputNumberDigits; i>0; i--){
    uint8_t temp=i;
    temp-=1;
    temp=DispUart.Digits[temp];
    temp+=48;
    Disp_UART_Transmit_Byte(temp);
  }
}

void Disp_UART_Tx_Number(int32_t num){
  if(num<0){
    Disp_UART_Transmit_Byte('-');
	num=-num;
  }
  Disp_UART_Determine_Digit_Numbers((uint32_t)num);
  Disp_UART_Tx_Number_Digits();
}





void Disp_UART_Tx_Number_NL(int32_t num){
  Disp_UART_Tx_Number(num);
  Disp_UART_Tx_NL();
}

void Disp_UART_Tx_Number_SP(int32_t num){
  Disp_UART_Tx_Number(num);
  Disp_UART_Tx_SP();
}

void Disp_UART_Tx_Number_CM(int32_t num){
  Disp_UART_Tx_Number(num);
  Disp_UART_Tx_CM();
}




void Disp_UART_Tx_Number_Hex_NL(int32_t num){
  Disp_UART_Tx_Hex((uint32_t)num);
  Disp_UART_Tx_NL();
}

void Disp_UART_Tx_Number_Hex_SP(int32_t num){
  Disp_UART_Tx_Hex((uint32_t)num);
  Disp_UART_Tx_SP();
}

void Disp_UART_Tx_Number_Hex_CM(int32_t num){
  Disp_UART_Tx_Hex((uint32_t)num);
  Disp_UART_Tx_CM();
}




void Disp_UART_Tx_Number_Bin_NL(int32_t num){
  Disp_UART_Tx_Bin((uint32_t)num);
  Disp_UART_Tx_NL();
}

void Disp_UART_Tx_Number_Bin_SP(int32_t num){
  Disp_UART_Tx_Bin((uint32_t)num);
  Disp_UART_Tx_SP();
}

void Disp_UART_Tx_Number_Bin_CM(int32_t num){
  Disp_UART_Tx_Bin((uint32_t)num);
  Disp_UART_Tx_CM();
}





void Disp_UART_Tx_Parameter_NL(char *name, int32_t num){
  Disp_UART_Tx_Text(name);
  Disp_UART_Tx_SP();
  Disp_UART_Tx_Number_NL(num);
}

void Disp_UART_Tx_Parameter_SP(char *name, int32_t num){
  Disp_UART_Tx_Text(name);
  Disp_UART_Tx_SP();
  Disp_UART_Tx_Number_SP(num);
}

void Disp_UART_Tx_Parameter_CM(char *name, int32_t num){
  Disp_UART_Tx_Text(name);
  Disp_UART_Tx_SP();
  Disp_UART_Tx_Number_CM(num);
}




void Disp_UART_Tx_Parameter_Hex_NL(char *name, int32_t num){
  Disp_UART_Tx_Text(name);
  Disp_UART_Tx_SP();
  Disp_UART_Tx_Number_Hex_NL(num);
}

void Disp_UART_Tx_Parameter_Hex_SP(char *name, int32_t num){
  Disp_UART_Tx_Text(name);
  Disp_UART_Tx_SP();
  Disp_UART_Tx_Number_Hex_SP(num);
}

void Disp_UART_Tx_Parameter_Hex_CM(char *name, int32_t num){
  Disp_UART_Tx_Text(name);
  Disp_UART_Tx_SP();
  Disp_UART_Tx_Number_Hex_CM(num);
}




void Disp_UART_Tx_Parameter_Bin_NL(char *name, int32_t num){
  Disp_UART_Tx_Text(name);
  Disp_UART_Tx_SP();
  Disp_UART_Tx_Number_Bin_NL(num);
}

void Disp_UART_Tx_Parameter_Bin_SP(char *name, int32_t num){
  Disp_UART_Tx_Text(name);
  Disp_UART_Tx_SP();
  Disp_UART_Tx_Number_Bin_SP(num);
}

void Disp_UART_Tx_Parameter_Bin_CM(char *name, int32_t num){
  Disp_UART_Tx_Text(name);
  Disp_UART_Tx_SP();
  Disp_UART_Tx_Number_Bin_CM(num);
}




//Receiver Functions

void Disp_UART_Flush_Buf(void){
	for(uint8_t i=0;i<DISP_UART_BUF_SIZE;i++){
	  DispUart.Buffer[i] = 0;
	}
	DispUart.BufferIndex = 0;
}

uint8_t Disp_UART_Get_Buf(uint16_t index){
	return DispUart.Buffer[index];
}

uint8_t Disp_UART_Get_Buf_Index(void){
	return DispUart.BufferIndex;
}




//Data Availablity chek using timer
uint8_t Disp_UART_Data_Available(void){
	return DispUart.DataAvailable;
}

void Disp_UART_Clear_Data_Available_Flag(void){
	DispUart.DataAvailable = 0;
}


uint8_t Disp_UART_Get_Data_Len(void){
	return Disp_UART_Get_Buf_Index();
}



void Disp_UART_Data_Copy_Buf(uint8_t *buf){
	for(uint16_t i=0;i<Disp_UART_Get_Data_Len();i++){
		buf[i] = Disp_UART_Get_Buf(i);
	}
}


void Disp_UART_Print_Buf_Data(void){
	if(Disp_UART_Data_Available()){
	  for(uint16_t i=0; i<Disp_UART_Get_Data_Len(); i++){
		  Disp_UART_Tx_Hex( Disp_UART_Get_Buf(i) );
	  }
  }
}