#include "stm32g070xx.h"
#include "disp.h"
#include "Uart.h"

#define UART_1             0
#define UART_2             1

typedef struct uart_t{
  volatile uint8_t Error;
  uint8_t          Digits[8];
  uint8_t          InputNumberDigits;
  volatile uint8_t Buffer[UART_BUF_SIZE];
  volatile uint8_t BufferIndex;
}uart_t;

uart_t UART;

void UART_GPIO_Init(void){
	#if UART_1

	  // Please Init the GPIO //
	
	#endif
	#if UART_2
    if((RCC->IOPENR & RCC_IOPENR_GPIODEN)!=RCC_IOPENR_GPIODEN){
       RCC->IOPENR|=RCC_IOPENR_GPIODEN;
    }
		if((RCC->IOPENR & RCC_IOPENR_GPIOAEN)!=RCC_IOPENR_GPIOAEN){
       RCC->IOPENR|=RCC_IOPENR_GPIOAEN;
    }
		GPIOD->MODER&=~GPIO_MODER_MODE5_Msk;
	  GPIOD->MODER|=GPIO_MODER_MODE5_1;
	  GPIOA->MODER&=~GPIO_MODER_MODE15_Msk;
	  GPIOA->MODER|=GPIO_MODER_MODE15_1;
	  GPIOD->AFR[0]&=~GPIO_AFRL_AFSEL5_Msk;
	  GPIOA->AFR[1]&=~GPIO_AFRH_AFSEL15_Msk;
	  GPIOA->AFR[1]|=(0<<GPIO_AFRH_AFSEL15_Pos);
		#endif
}

void UART_Clock_Init(void){
	#if UART_1
	  RCC->APBENR2|=RCC_APBENR2_USART1EN;
	  RCC->CCIPR&=~RCC_CCIPR_USART1SEL_Msk;
	  RCC->CCIPR|=RCC_CCIPR_USART1SEL_1;	
	#endif
	#if UART_2
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;
	#endif
}

void UART_Set_BAUD_Rate(uint32_t baud){
	#if UART_1
	  if(USART1->CR1 & USART_CR1_UE){
		  USART1->CR1&=~USART_CR1_UE;
	  }
	  //USART1->CR2|=USART_CR2_SWAP;      /*Tx & Rx pin swap*/
	  USART1->BRR=(uint16_t)(16000000/baud);
	  USART1->CR1|=USART_CR1_UE;
	#endif	
	#if UART_2
    if(USART2->CR1 & USART_CR1_UE){
        USART2->CR1 &= ~USART_CR1_UE;
    }
    USART2->BRR = (uint16_t)(SystemCoreClock / baud);
    USART2->CR1 |= USART_CR1_UE;
	#endif
}

void UART_TX_Config(void){
	#if UART_1
	  USART1->CR1|=USART_CR1_TE;
	  if((USART1->CR1 & USART_CR1_UE)!=USART_CR1_UE){
		   USART1->CR1|=USART_CR1_UE;
	  }
	#endif
	#if UART_2
    USART2->CR1 |= USART_CR1_TE;
    if((USART2->CR1 & USART_CR1_UE) != USART_CR1_UE){
       USART2->CR1 |= USART_CR1_UE;
    }
	#endif
}

void UART_RX_Config(void){
	#if UART_1
	  USART1->CR1|=USART_CR1_RE;
	  if((USART1->CR1 & USART_CR1_UE)!=USART_CR1_UE){
		   USART1->CR1|=USART_CR1_UE;
	}
	#endif	
	#if UART_2
    USART2->CR1 |= USART_CR1_RE;
    if((USART2->CR1 & USART_CR1_UE) != USART_CR1_UE){
       USART2->CR1 |= USART_CR1_UE;
    }
	#endif
}

void UART_Init(uint32_t baud){
   UART_GPIO_Init();
   UART_Clock_Init();
   UART_Set_BAUD_Rate(baud);
   if(UART_ENABLE_TX){
      UART_TX_Config();
   }
   if(UART_ENABLE_RX){
      UART_RX_Config();
   }
}

void UART_Transmit_Byte(uint8_t val){
	#if UART_1
	  USART1->TDR=val;
	  while((USART1->ISR & USART_ISR_TC)!=USART_ISR_TC);
	  USART1->ICR|=USART_ICR_TCCF;
	#endif
	
	#if UART_2
    USART2->TDR = val;
    while((USART2->ISR & USART_ISR_TC) != USART_ISR_TC);
    USART2->ICR |= USART_ICR_TCCF;
	#endif
}

void UART_Transmit_Byte_32(uint32_t val){
	#if UART_1
	  USART1->TDR=val;
	  while((USART1->ISR & USART_ISR_TC)!=USART_ISR_TC);
	  USART1->ICR|=USART_ICR_TCCF;	
  #endif
	#if UART_2
    USART2->TDR = val;
    while((USART2->ISR & USART_ISR_TC) != USART_ISR_TC);
    USART2->ICR |= USART_ICR_TCCF;
	#endif
}

/////--------------------UART Transmit Data Type Function---------------------/////
///////////////////////////////////////////////////////////////////////////////////

void UART_Transmit_Byte_Hex(uint32_t val){
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
  UART_Transmit_Byte('0');
  UART_Transmit_Byte('x');
	for(uint8_t i=0;i<loop_counter;i++){
	  index-=4;
	  hex_digit=(uint8_t)((val>>index) & 0x0F);
	  if(hex_digit>9){
	    hex_digit+=55;
	  }else {
	    hex_digit+=48;
	  }
	  UART_Transmit_Byte((uint8_t)hex_digit);
	}
}

void UART_Transmit_Byte_Hex_64(uint8_t hex, uint64_t val){
  uint8_t hex_digits[hex];
	UART_Transmit_Byte('0');
  UART_Transmit_Byte('x');
   for (int i=0; i<hex; i++) {
      hex_digits[i] = (val >> (((hex-1) - i) * 4)) & 0x0F; 
      if(hex_digits[i]>9){hex_digits[i]+=55;}
      else {hex_digits[i]+=48;}
   UART_Transmit_Byte(hex_digits[i]);
  }	
}

void UART_Transmit_Byte_Bin(uint8_t val){
  uint8_t bin_digits[8];
  for(uint8_t i=0;i<8;i++){
    bin_digits[i]= (uint8_t)((val & 0x01)+48);
    val>>=1;
  }
  UART_Transmit_Byte('0');
  UART_Transmit_Byte('b');
  for(uint8_t i=8;i>0;i--){
    uint8_t index=(uint8_t)(i-1);
    UART_Transmit_Byte(bin_digits[index]);
  }
}

void UART_Transmit_Text(char *str){
  uint8_t i=0;
  while(str[i]!='\0'){
    UART_Transmit_Byte(str[i]);
    i++;
  }
}

void UART_Determine_Digit_Numbers(uint32_t num){
  uint8_t i=0;
  if(num==0){
    UART.Digits[0]=0;
    UART.InputNumberDigits=1;
  }else{
    while(num!=0){
      UART.Digits[i]=num%10;
      num/=10;
      i++;
    }
	UART.InputNumberDigits=i;
  }
}

void UART_Transmit_Number_Digits(void){
  for(uint8_t i=UART.InputNumberDigits; i>0; i--){
    uint8_t temp=i;
    temp-=1;
    temp=UART.Digits[temp];
    temp+=48;
    UART_Transmit_Byte(temp);
  }
}

void UART_Transmit_Number(int32_t num){
  if(num<0){UART_Transmit_Byte('-');num=-num;}
  UART_Determine_Digit_Numbers((uint32_t)num);
  UART_Transmit_Number_Digits();
}

void UART_Transmit_Fixed_Digit_Number(int32_t num, uint8_t digits){
  if(num<0){UART_Transmit_Byte('-');num=-num;}
  UART_Determine_Digit_Numbers((uint32_t)num);
  for(int i=0;i<(digits-UART.InputNumberDigits);i++){
    UART_Transmit_Byte(48);
  }
  UART_Transmit_Number_Digits();
}

/////--------------------------UART Printf Functions--------------------------/////
///////////////////////////////////////////////////////////////////////////////////


void UART_PrintfD_NL (char *str, int32_t value){
	UART_Transmit_Text(str);
	UART_Transmit_Number(value);
	UART_Transmit_Text("\r\n");
}
void UART_PrintfD (char *str, int32_t value){
	UART_Transmit_Text(str);
	UART_Transmit_Number(value);
}
void UART_PrintfH (char *str,uint8_t digit, uint64_t value){
	UART_Transmit_Text(str);
	UART_Transmit_Byte_Hex_64(digit, value);
	UART_Transmit_Text("\r\n");
}

void UART_PrintfNL (uint8_t value){
	for(int i=0; i<value; i++){
		UART_Transmit_Text("\r\n");
	}
}





void UART_Tx_Bin(uint32_t val){
  uint8_t loop_counter=0;
  if(val <= 0xFF){
    loop_counter=7;
  }else if(val <= 0xFFFF){
    loop_counter=15;     
  }else{
    loop_counter=31;
  }
  
  UART_Transmit_Byte('0');
  UART_Transmit_Byte('b');
  for(int i=loop_counter; i>=0; i--){
    if( (val>>i) & 1){
      UART_Transmit_Byte( 49 );   
    }else{
      UART_Transmit_Byte( 48 );         
    }
  }
}


void UART_PrintfD_BIN (char *str, int32_t value){
	UART_Transmit_Text(str);
	UART_Tx_Bin(value);
}




// For printing hex

void UART_Tx_Hex(uint32_t val){
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
  UART_Transmit_Byte('0');
  UART_Transmit_Byte('x');
	for(uint8_t i=0;i<loop_counter;i++){
	  index-=4;
	  hex_digit=(uint8_t)((val>>index) & 0x0F);
	  if(hex_digit>9){
	    hex_digit+=55;
	  }else {
	    hex_digit+=48;
	  }
	  UART_Transmit_Byte((uint8_t)hex_digit);
	}
}



void UART_PrintfD_HEX (char *str, int32_t value){
	UART_Transmit_Text(str);
	UART_Transmit_Byte_Hex(value);
	UART_Transmit_Text("\r\n");
}

void UART_Print_Raw_Data(void){
	if(Disp_UART_Data_Available()){
	  for(uint16_t i=0; i<Disp_UART_Get_Data_Len(); i++){
		  UART_Transmit_Byte_Hex( Disp_UART_Get_Buf(i) );
			UART_Transmit_Text(" ");
	  }
		UART_PrintfNL(1);
  }
}