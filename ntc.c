#include "stm32g070xx.h"
#include "timebase.h"
//#include "timer.h"
#include "iirlpf.h"


#define  NTC0_ADC_CHANNEL 3
#define  NTC1_ADC_CHANNEL 4
#define  NTC2_ADC_CHANNEL 5
#define  NTC3_ADC_CHANNEL 6
#define  NTC4_ADC_CHANNEL 7


///////////////////////////////////////////////////////
  uint8_t  ADC_Channel[5] = {NTC0_ADC_CHANNEL,NTC1_ADC_CHANNEL,NTC2_ADC_CHANNEL,NTC3_ADC_CHANNEL,NTC4_ADC_CHANNEL};

  typedef struct ntc_t{
    int32_t  RawADC[5];
    int32_t  Temperature[5];
    uint8_t  InitMode;
    uint8_t  Error[5];
  }ntc_t;
  ntc_t  NTC;
///////////////////////////////////////////////////////
  void NTC_Struct_Init(void){
    for(uint8_t i=0;i<5;i++){
      NTC.RawADC[i]=0;
			NTC.Temperature[i]=0;
    }
    NTC.InitMode=1;
  }
/////////////////////////////////////////////////////// 
  void NTC_Sensor_GPIO_Init (void){	
	  if(!(RCC->IOPENR & RCC_IOPENR_GPIOAEN)){
		   RCC->IOPENR|=RCC_IOPENR_GPIOAEN;
	  }
	  GPIOA->MODER|= (GPIO_MODER_MODE3_Msk | GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk |
                 		GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE7_Msk); //11 select for analog mode
  } 
	
  void NTC_Sensor_ADC_Init (void){
     if((RCC->APBENR2 & RCC_APBENR2_ADCEN)!=RCC_APBENR2_ADCEN){
		    RCC->APBENR2|=RCC_APBENR2_ADCEN;
		 }
		 RCC->CCIPR&=~RCC_CCIPR_ADCSEL_Msk;  //HSI16
	   RCC->CCIPR|=RCC_CCIPR_ADCSEL_1;	 
		 if(ADC1->CR & ADC_CR_ADEN){
		    ADC1->CR|=ADC_CR_ADDIS;
	   }
	   while((ADC1->CR & ADC_CR_ADEN)==ADC_CR_ADEN);
	   ADC1->CR = 0x00;
	   ADC1->CR|= ADC_CR_ADVREGEN;
	   Timebase_Timer_Delay_SubSeconds(100);
		 //delay(10);
	   ADC1->CR|= ADC_CR_ADCAL;
	   while(ADC1->CR & ADC_CR_ADCAL){  };
     ADC1->CFGR1=0x00;
	   ADC1->CFGR2=0x00;
	   ADC1->SMPR=0x00;
	   ADC1->SMPR|=ADC_SMPR_SMP1_0|ADC_SMPR_SMP1_1|ADC_SMPR_SMP1_2;    
	   ADC1->SMPR|=ADC_SMPR_SMP2_0|ADC_SMPR_SMP2_1|ADC_SMPR_SMP2_2;
	  ADC1->CHSELR=0x01;
	  while((ADC1->ISR & ADC_ISR_CCRDY)==0);
	  ADC->CCR=(0x08<<ADC_CCR_PRESC_Pos)|ADC_CCR_VREFEN;
	  if(ADC1->ISR & ADC_ISR_ADRDY){
		   ADC1->ISR|=ADC_ISR_ADRDY;
	  }
	  ADC1->CR|=ADC_CR_ADEN;
	  while((ADC1->ISR & ADC_ISR_ADRDY) == 0);
  }

  uint16_t Read_ADC(uint8_t channel){
	  if(ADC1->CHSELR != (1<<channel)){
		  ADC1->CHSELR=(1<<channel);
		  while((ADC1->ISR & ADC_ISR_CCRDY)==0);
	  }
	  ADC1->CR|=ADC_CR_ADSTART;
    while((ADC1->ISR & ADC_ISR_EOC) == 0);
	  uint16_t data=(uint16_t)ADC1->DR;
	  return data;
  }

	
	uint16_t NTC_Sensor_Reading(uint8_t sensoor){
	  int16_t NTC_sensor_data = Read_ADC(ADC_Channel[sensoor]);
	  NTC.RawADC[sensoor] = NTC_sensor_data;
	  return NTC_sensor_data;
  }
	
	
	void NTC_Calculate_Single_Sample(uint8_t sensor){ //12 bit ADC data is used 
     NTC_Sensor_Reading(sensor);
	   NTC_Sensor_Reading(sensor);

	   NTC_Sensor_Reading(sensor);	
	
     if(NTC.RawADC[sensor]>3764){ //coldest temp
        NTC.Temperature[sensor]=-1000;
    }else if(NTC.RawADC[sensor]>3549){
        NTC.Temperature[sensor]= (-479*NTC.RawADC[sensor] + 1391491)>>10;		
    }else if(NTC.RawADC[sensor]>3252){
        NTC.Temperature[sensor]=(-346*NTC.RawADC[sensor] + 920562)>>10	;
    }else if(NTC.RawADC[sensor]>2880){
        NTC.Temperature[sensor]= (-277*NTC.RawADC[sensor] + 692787)>>10;
    }else if(NTC.RawADC[sensor]>2456){
        NTC.Temperature[sensor]= (-243*NTC.RawADC[sensor] + 594791)>>10;
    }else if(NTC.RawADC[sensor]>2020){
        NTC.Temperature[sensor]= (-236*NTC.RawADC[sensor] + 578148)>>10;
    }else if(NTC.RawADC[sensor]>1612){
        NTC.Temperature[sensor]= (-252*NTC.RawADC[sensor] + 610626)>>10;
    }else if(NTC.RawADC[sensor]>1256){
        NTC.Temperature[sensor]= (-289*NTC.RawADC[sensor] + 669782)>>10;
    }else if(NTC.RawADC[sensor]>964){
        NTC.Temperature[sensor]= (-352*NTC.RawADC[sensor] + 749173)>>10;
    }else if(NTC.RawADC[sensor]>736){
        NTC.Temperature[sensor]= (-450*NTC.RawADC[sensor] + 842554)>>10;
    }else{
        NTC.Temperature[sensor]=-2000; //hottest temp
    }
		
		
    NTC.Temperature[sensor]=LPF_Get_Filtered_Value(sensor,NTC.Temperature[sensor]);
		
		
		if(NTC.Temperature[sensor] <= -1900){ //value will not be == -2000 due to LPF
			  NTC.Temperature[sensor] = -2000;
        NTC.Error[sensor]=0x02;
    }else if(NTC.Temperature[sensor] <= -900){  //value will not be == -1000 due to LPF
			  NTC.Temperature[sensor] = -1000;
        NTC.Error[sensor]=0x01;
    }else{
        NTC.Error[sensor]=0x00;
    }
  }

  int NTC_Get_RawADC(uint8_t i){
    return NTC.RawADC[i];
  }
  int NTC_Get_Temperature(uint8_t i){
    return NTC.Temperature[i];
  }
  uint8_t NTC_Get_Error(uint8_t i){
    return NTC.Error[i];
  }
  void NTC_Sensor_Init(void){
	  NTC_Struct_Init();
    LPF_Init();
	  NTC_Sensor_GPIO_Init ();
	  NTC_Sensor_ADC_Init();
	  LPF_Set_Alpha(0,30);
		LPF_Set_Alpha(1,30);
		LPF_Set_Alpha(2,30);
		LPF_Set_Alpha(3,30);
		LPF_Set_Alpha(4,30);
  }
			


