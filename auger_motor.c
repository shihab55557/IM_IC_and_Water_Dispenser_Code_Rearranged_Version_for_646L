#include "stm32g070xx.h"
#include "timebase.h"
#include "Uart.h"
#include "disp_comm.h"




#define DEVICE_ADDR      0x52


#define TIMEBASE_AUGER_MOTOR_CW_ROTATION_WINDOW    6
#define TIMEBASE_AUGER_MOTOR_CW_ROTATION_TIME      8

#define TIMEBASE_AUGER_MOTOR_STOP_WINDOW           7
#define TIMEBASE_AUGER_MOTOR_STOP_TIME             2


#define TIMEBASE_AUGER_MOTOR_CCW_ROTATION_WINDOW   8
#define TIMEBASE_AUGER_MOTOR_CCW_ROTATION_TIME     8

uint8_t Parameter_Addr[12] = { 0x20,  //01
                               0x21,  //02
                               0x22,  //03
                               0x23,  //04
                               0x24,  //05
                               0x25,  //06
                               0x26,  //07
                               0x27,  //08
	                             0x28,  //09
                               0x29,  //10
                               0x2A,  //11
                               0x2B   //12
                             };    

uint8_t Parameter_Value[12] ={ 0xC8,  //0x20 
                               0xBF,  //0x21
                               0x4A,  //0x22
                               0x00,  //0x23
                               0xC0,  //0x24	
                               0xC0,  //0x25
                               0x95,  //0x26
                               0x00,  //0x27
	                             0x0F,  //0x28
                               0x57,  //0x29
                               0x04,  //0x2A
                               0x0C   //0x2B
                             }; 

 typedef struct BLDC_Motor_t{
   uint16_t Read_Motor_Data;
	 uint16_t Debug_Motor_Data;
	 uint8_t  Motor_Start;
	 uint8_t  Motor_Stop;
	 uint8_t  Rotate_CW;
	 uint8_t  Rotate_CCW;

 }BLDC_Motor_t;
 BLDC_Motor_t BLDC_Motor__t;
  
 void BLDC_Motor_Struct_Init(void){
  BLDC_Motor__t.Read_Motor_Data=0;
	BLDC_Motor__t.Motor_Start=0;
	BLDC_Motor__t.Motor_Stop=0;
	BLDC_Motor__t.Rotate_CW=0;
	BLDC_Motor__t.Rotate_CCW=0;
 }
////////////////////////////Motor Flags////////////////////////////
///////////////////////////////////////////////////////////////////
 void Enable_Motor_Start_Flag(void){
    BLDC_Motor__t.Motor_Start = 1;
 }
 void Disable_Motor_Start_Flag(void){
    BLDC_Motor__t.Motor_Start = 0;
 }
 uint8_t Is_Motor_Start(void){
    return BLDC_Motor__t.Motor_Start;
 }
//////////////////////////////////////////////////
//////////////////////////////////////////////////
 void Enable_Motor_Stop_Flag(void){
    BLDC_Motor__t.Motor_Stop = 1;
 }
 void Disable_Motor_Stop_Flag(void){
    BLDC_Motor__t.Motor_Stop = 0;
 }
 uint8_t Is_Motor_Stop(void){
    return BLDC_Motor__t.Motor_Stop;
 }
//////////////////////////////////////////////////
//////////////////////////////////////////////////
 void Enable_Motor_Rotate_CW_Flag(void){
    BLDC_Motor__t.Rotate_CW = 1;
 }
 void Disable_Motor_Rotate_CW_Flag(void){
    BLDC_Motor__t.Rotate_CW = 0;
 }
 uint8_t Is_Motor_Rotate_CW(void){
    return BLDC_Motor__t.Rotate_CW;
 } 
//////////////////////////////////////////////////
//////////////////////////////////////////////////
 void Enable_Motor_Rotate_CCW_Flag(void){
    BLDC_Motor__t.Rotate_CCW = 1;
 }
 void Disable_Motor_Rotate_CCW_Flag(void){
    BLDC_Motor__t.Rotate_CCW = 0;
 }
 uint8_t Is_Motor_Rotate_CCW(void){
    return BLDC_Motor__t.Rotate_CCW;
 } 
 
 void I2C1_GPIO_Init(void){
	if((RCC->IOPENR & RCC_IOPENR_GPIOAEN)!=RCC_IOPENR_GPIOAEN){
		RCC->IOPENR|=RCC_IOPENR_GPIOAEN;
	}
	GPIOA->MODER&=~GPIO_MODER_MODE9_Msk;
	GPIOA->MODER|=GPIO_MODER_MODE9_1;
	GPIOA->MODER&=~GPIO_MODER_MODE10_Msk;
	GPIOA->MODER|=GPIO_MODER_MODE10_1;
	GPIOA->OTYPER|=GPIO_OTYPER_OT9;
	GPIOA->OTYPER|=GPIO_OTYPER_OT10;
	GPIOA->PUPDR&=~GPIO_PUPDR_PUPD9_Msk;     //SCL
	GPIOA->PUPDR|=GPIO_PUPDR_PUPD9_0;
	GPIOA->PUPDR&=~GPIO_PUPDR_PUPD10_Msk;    //SDA
	GPIOA->PUPDR|=GPIO_PUPDR_PUPD10_0;
	GPIOA->AFR[1]&=~GPIO_AFRH_AFSEL9_Msk;
	GPIOA->AFR[1]|=(0x06<<GPIO_AFRH_AFSEL9_Pos);
	GPIOA->AFR[1]&=~GPIO_AFRH_AFSEL10_Msk;
	GPIOA->AFR[1]|=(0x06<<GPIO_AFRH_AFSEL10_Pos);
}

 void I2C1_Clock_Init(void){
	RCC->APBENR1|=RCC_APBENR1_I2C1EN;
	RCC->CCIPR&=~RCC_CCIPR_I2C1SEL_Msk;
	RCC->CCIPR|=RCC_CCIPR_I2C1SEL_1;
}

 void I2C1_Config(void){
	I2C1->TIMINGR =(0x03<<I2C_TIMINGR_PRESC_Pos);
	I2C1->TIMINGR|=(0x04<<I2C_TIMINGR_SCLDEL_Pos);
	I2C1->TIMINGR|=(0x02<<I2C_TIMINGR_SDADEL_Pos);
	I2C1->TIMINGR|=(0xC3<<I2C_TIMINGR_SCLH_Pos); //0xC3
	I2C1->TIMINGR|=(0xC7<<I2C_TIMINGR_SCLL_Pos); //0xC7
	I2C1->CR1|=I2C_CR1_PE;
}

 void I2C1_Init(void){
  I2C1_GPIO_Init();
	I2C1_Clock_Init();
	I2C1_Config();
}

 uint8_t I2C1_TX_ADDR(uint8_t addr){
	uint8_t ack=1;
	I2C1->CR2=I2C_CR2_AUTOEND|(1<<16)|(addr<<1);
	I2C1->CR2|=I2C_CR2_START;
	while((I2C1->ISR & I2C_ISR_TXE)!=I2C_ISR_TXE);
	if(I2C1->ISR & I2C_ISR_NACKF){
		ack=0;
		I2C1->ICR|=I2C_ICR_NACKCF;
	}
	return ack;
}

 uint8_t I2C1_TX_ADDR_READ(uint8_t addr){
	uint8_t ack=1;
	I2C1->CR2=I2C_CR2_AUTOEND|(1<<16)|(addr<<1)|I2C_CR2_RD_WRN;
	I2C1->CR2|=I2C_CR2_START;
	while((I2C1->ISR & I2C_ISR_TXE)!=I2C_ISR_TXE);
	if(I2C1->ISR & I2C_ISR_NACKF){
		ack=0;
		I2C1->ICR|=I2C_ICR_NACKCF;
	}
	return ack;
}

 void I2C1_Write_Register(uint8_t addr, uint8_t data){
	I2C1->TXDR=addr;
  I2C1->CR2=I2C_CR2_AUTOEND|(2<<16)|(0x52<<1);
	I2C1->CR2|=I2C_CR2_START;
	while((I2C1->ISR & I2C_ISR_TXE)!=I2C_ISR_TXE);
	I2C1->TXDR=data;
	while((I2C1->ISR & I2C_ISR_TXE)!=I2C_ISR_TXE);
}

uint8_t I2C1_Read_BLDC_Motor_Status(uint8_t addr){
  I2C1->TXDR=addr;
	I2C1_TX_ADDR(0x52);
	I2C1_TX_ADDR_READ(0x52);
	return BLDC_Motor__t.Read_Motor_Data=I2C1->RXDR;
}

void BLDC_I2C1_Init(void){
	BLDC_Motor_Struct_Init();
	I2C1_Init();
}

void Direction_Pin_Gpio_Init (void){
  if((RCC->IOPENR & RCC_IOPENR_GPIOAEN)!=RCC_IOPENR_GPIOAEN){
		RCC->IOPENR|=RCC_IOPENR_GPIOAEN;
	}
	GPIOA->MODER&=~GPIO_MODER_MODE12_Msk;
	GPIOA->MODER|=GPIO_MODER_MODE12_0;
	GPIOA->OTYPER&=~ GPIO_OTYPER_OT12_Msk;
}

void Auger_Motor_Rotate_CW (void){
	 GPIOA->ODR &=~ (1<<12);
}
void Auger_Motor_Rotate_CCW (void){
	 GPIOA->ODR |= (1<<12);
}

uint8_t Auger_motor_op = 0;
uint8_t Auger_Motor_Start (void){
	for(int i=0;i<2;i++){
	 I2C1_Write_Register(0x00,0xFF);
	 I2C1_Write_Register(0x01,0b10000001);
   I2C1_Write_Register(0x03,0b11000000);
	 for(int i=0;i<12;i++){
     I2C1_Write_Register(Parameter_Addr[i],Parameter_Value[i]);
	 }
	}
	Auger_motor_op = 1;
	return Auger_motor_op;
}
uint8_t Auger_Motor_Stop (void){
	for(int i=0;i<2;i++){
	 I2C1_Write_Register(0x00,0x00);
	 I2C1_Write_Register(0x01,0b10000000);
	 I2C1_Write_Register(0x03,0b01000000);
	 for(int i=0;i<12;i++){
     I2C1_Write_Register(Parameter_Addr[i],Parameter_Value[i]);
	 }
  }
	Auger_motor_op = 0;
	return Auger_motor_op;
}

/*
uint8_t Index = 0;
uint8_t Loop = 0;

void Auger_Motor_Start (void){
	if(Loop<2){
	 I2C1_Write_Register(0x00,0xFF);
	 I2C1_Write_Register(0x01,0b10000001);
   I2C1_Write_Register(0x03,0b11000000);
 
   I2C1_Write_Register(Parameter_Addr[Index],Parameter_Value[Index]);
	 Index++;
	 if(Index>12){
		 Index=0;
		 Loop++;
	 }	 
  }
}
void Auger_Motor_Stop (void){
	if(Loop<2){
	 I2C1_Write_Register(0x00,0x00);
	 I2C1_Write_Register(0x01,0b10000000);
	 I2C1_Write_Register(0x03,0b01000000);

   I2C1_Write_Register(Parameter_Addr[Index],Parameter_Value[Index]);
	 Index++;
	 if(Index>12){
		 Index=0;
		 Loop++;
	 }
  }	 
}*/


uint8_t I2C_Read_New(uint8_t reg_addr){
	uint32_t rx_data=0;
	I2C1->CR2 &=~ I2C_CR2_NBYTES_Msk;
	I2C1->CR2 |=  (1<<I2C_CR2_NBYTES_Pos);        //1 byte to be transmitted
	I2C1->CR2 &=~ I2C_CR2_RD_WRN;                 //Write cmd
	I2C1->TXDR = reg_addr;
	
	I2C1->CR2 &=~ I2C_CR2_SADD_Msk;
	I2C1->CR2 |= (0x52<<1);                       //Check addr
	I2C1->CR2 |= I2C_CR2_START;
	while((I2C1->CR2 & I2C_CR2_START)==I2C_CR2_START);   //SlaveAddr+Write
	while((I2C1->ISR & I2C_ISR_TXE)==0);
	
	if( (I2C1->ISR & I2C_ISR_NACKF) ==1){                //NACK received
		//I2C.Error=0x01;                                  //Generate Error Code
	}else{                                               //ACK received
		I2C1->CR2 |= I2C_CR2_STOP;                         //Stop request
	  while((I2C1->CR2 & I2C_CR2_STOP)==I2C_CR2_STOP);   //Wait until Stop is sent
	  while((I2C1->ISR & I2C_ISR_STOPF)==0);             //Wait for Stop flag is set
	  I2C1->ICR |= I2C_ICR_STOPCF;                       //Request for Stop flag clear
	  while((I2C1->ISR & I2C_ISR_STOPF)==I2C_ISR_STOPF); //Wait until stop flag is cleared    //Stop
	  I2C1->CR2 &=~ I2C_CR2_NBYTES_Msk;
	  I2C1->CR2 |=  (1<<I2C_CR2_NBYTES_Pos);             //1 byte to be transmitted
	  I2C1->CR2 |= I2C_CR2_RD_WRN;                       //Read cmd
	  I2C1->CR2 &=~ I2C_CR2_SADD_Msk;
	  I2C1->CR2 |= (0x52<<1);
	  I2C1->CR2 |= I2C_CR2_START;
	  while((I2C1->CR2 & I2C_CR2_START)==I2C_CR2_START); //Start+SLAVE_ADD
	  while((I2C1->ISR & I2C_ISR_RXNE)==1);              //Wait until RXDR is full
	  rx_data=I2C1->RXDR;
	  I2C1->CR2 |= I2C_CR2_STOP;                         //Stop request
	  while((I2C1->CR2 & I2C_CR2_STOP)==I2C_CR2_STOP);   //Wait until Stop is sent
	  while((I2C1->ISR & I2C_ISR_STOPF)==0);             //Wait for Stop flag is set
	  I2C1->ICR |= I2C_ICR_STOPCF;                       //Request for Stop flag clear
	  while((I2C1->ISR & I2C_ISR_STOPF)==I2C_ISR_STOPF); //Wait until stop flag is cleared
		//I2C.Error=0x00;                                  //Clear Error code
	}
	return (uint8_t)rx_data;
}

uint8_t I2C_Read_Reg(uint8_t reg_addr){
	I2C_Read_New(reg_addr);
	return I2C_Read_New(reg_addr);
}




void Auger_Motor_Init(void){
	BLDC_I2C1_Init();
	Direction_Pin_Gpio_Init();
}

void Auger_Motor_CW_Continouse_Rotation(void){
	Auger_Motor_Start();
	Auger_Motor_Rotate_CW();
}




//------Disp comm based auger motor control------//


void Auger_Motor_Disp_Comm_Based_Rotation(void){
	if((Disp_Comm_Master_Data_Get_IceMaker() & (1<<2)) == (1<<2)){
		Auger_Motor_Rotate_CCW();
		if(Auger_motor_op != 1){
			Auger_Motor_Start();
		}
		Disp_Comm_Heartbeat_Set_Data(6,1,0);
	}
	else {
		if(Auger_motor_op != 0){
			Auger_Motor_Stop();
		}
		Disp_Comm_Heartbeat_Set_Data(6,0,0);
	}
}




void Auger_Motor_Handler(void){
	Auger_Motor_Disp_Comm_Based_Rotation();
}






