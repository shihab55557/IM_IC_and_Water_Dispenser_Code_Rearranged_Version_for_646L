
#include "stm32g070xx.h"
#include "timebase.h"

#define  TIMEBASE_COUNT_ATOMIC_OPERATION
//#define  TIMEBASE_TOKEN_FUNCTIONS
#define  TIMEBASE_TIME_WINDOW_CALCULATION





/********************************Structure & Enumeration Start******************************/

typedef union {
  struct {
    volatile uint8_t       WatchDogTimer  :1;
    volatile uint8_t       RealTimeCounter:1;
    volatile uint8_t       GeneralTimer   :1;
    volatile uint8_t       TimerIndex     :5;
  };
  volatile uint8_t         Value            ;
} timebase_timer_t;

typedef struct timebase_time_t{
  volatile uint8_t         OVFUpdateValue   ;
  volatile int32_t         LastSample       ;
  volatile uint16_t        SubSeconds       ;
  volatile int32_t         Seconds          ;
  volatile uint16_t        SubSecondsShadow ;
  volatile int32_t         SecondsShadow    ;
  volatile uint8_t         VariablesSync    ;
  
  #ifdef TIMEBASE_TIME_WINDOW_CALCULATION
  int32_t                  StartTimeSeconds   ;
  int32_t                  StartTimeSubSeconds;
  uint8_t                  Status             ;
  #endif
}timebase_time_t;

typedef union {
  struct {
    uint8_t               PeriodFlag     :1;
    uint8_t               Value          :4;
  };
  volatile uint8_t        StatusByte       ;
} timebase_status_t;


#ifdef TIMEBASE_UPCOUNTER_SUBSECONDS
typedef struct timebase_upcounter_ss_t{
  timebase_status_t       Status           ;
  int32_t                 EndValueSec      ;
  int32_t                 EndValueSubSec   ;
  int32_t                 Target           ;
  int32_t                 Temporary        ;
  int32_t                 Value            ;
  int32_t                 PeriodValue      ;
}timebase_upcounter_ss_t;
#endif


#ifdef TIMEBASE_UPCOUNTER
typedef struct timebase_upcounter_t{
  timebase_status_t       Status           ;
  int32_t                 EndValue         ;
  int32_t                 Target           ;
  int32_t                 Temporary        ;
  int32_t                 Value            ;
  int32_t                 PeriodValue      ;
}timebase_upcounter_t;
#endif



#ifdef TIMEBASE_DOWNCOUNTER_SUBSECONDS
typedef struct timebase_downcounter_ss_t{
  timebase_status_t       Status           ;
  int32_t                 EndValueSec      ;
  int32_t                 EndValueSubSec   ;
  int32_t                 Value            ;
  int32_t                 PeriodValue      ;
}timebase_downcounter_ss_t;
#endif


#ifdef TIMEBASE_DOWNCOUNTER
typedef struct timebase_downcounter_t{
  timebase_status_t         Status         ;
  int32_t                   EndValue       ;
  int32_t                   Value          ;
  int32_t                   PeriodValue    ;
}timebase_downcounter_t;
#endif

typedef struct timebase_config_t{
  timebase_timer_t        TimerType        ;
  volatile uint16_t       UpdateRate       ;
}timebase_config_t;

typedef struct timebase_t{
  timebase_config_t      Config                             ;
  timebase_time_t        Time                               ;
  volatile uint8_t       UpdateRequest                      ;
  
  #ifdef TIMEBASE_TOKEN_FUNCTIONS
  volatile uint8_t       ActiveTokens                       ;
  #endif
  
  #ifdef TIMEBASE_UPCOUNTER_SUBSECONDS
  timebase_upcounter_ss_t UpCounterSS[TIMEBASE_UPCOUNTER_SUBSECONDS] ;
  #endif
  
  #ifdef TIMEBASE_UPCOUNTER
  timebase_upcounter_t   UpCounter[TIMEBASE_UPCOUNTER]      ;
  #endif
  
  #ifdef TIMEBASE_DOWNCOUNTER_SUBSECONDS
  timebase_downcounter_ss_t DownCounterSS[TIMEBASE_DOWNCOUNTER_SUBSECONDS] ;
  #endif
  
  #ifdef TIMEBASE_DOWNCOUNTER
  timebase_downcounter_t DownCounter[TIMEBASE_DOWNCOUNTER]  ;
  #endif
  
}timebase_t;



enum{
  COUNTER_STATE_RESET       = 0,
  COUNTER_STATE_START       = 1,
  COUNTER_STATE_STARTED     = 1,
  COUNTER_STATE_STOP        = 2,
  COUNTER_STATE_STOPPED     = 2,
  COUNTER_STATE_EXPIRED     = 4
};

enum{
  FLAG_STATE_RESET          = 0,
  FLAG_STATE_SET            = 1,
};

enum{
  TIMEBASE_FALSE            = 0,
  TIMEBASE_TRUE             = 1,
};

enum{
  UPCOUNTER_UPDATE_REQ      = 1,
  DOWNCOUNTER_UPDATE_REQ    = 2,
  UPCOUNTER_SS_UPDATE_REQ   = 4,
  DOWNCOUNTER_SS_UPDATE_REQ = 8
};
  

  

timebase_t Timebase_type;
timebase_t *Timebase;

/********************************Structure & Enumeration End******************************/










/************************************Basic Functions Start*******************************/

void Timebase_Struct_Init(void){
  Timebase=&Timebase_type;
  Timebase->Config.TimerType.Value = 0;
  Timebase->Config.TimerType.GeneralTimer = 1;
  Timebase->Config.TimerType.TimerIndex = 0;
  Timebase->Config.UpdateRate = 1;
  Timebase->Time.OVFUpdateValue=0;
  Timebase->Time.SubSecondsShadow = 0;
  Timebase->Time.SecondsShadow = 0;
  Timebase->Time.SubSeconds = 0;
  Timebase->Time.Seconds = 0;
  Timebase->Time.VariablesSync = 0;
  Timebase->Time.LastSample = 0;
  
  #ifdef TIMEBASE_TIME_WINDOW_CALCULATION
    Timebase->Time.StartTimeSeconds = 0;
	Timebase->Time.StartTimeSubSeconds = 0;
	Timebase->Time.Status = 0;
  #endif
  
  Timebase->UpdateRequest = 0;
  
  #ifdef TIMEBASE_TOKEN_FUNCTIONS
  Timebase->ActiveTokens = 0;
  #endif

  #ifdef TIMEBASE_UPCOUNTER
  for(uint8_t i=0; i < TIMEBASE_UPCOUNTER; i++){
    Timebase->UpCounter[i].Status.StatusByte = 0;  
    Timebase->UpCounter[i].EndValue = 0;
    Timebase->UpCounter[i].Target = 0;
    Timebase->UpCounter[i].Temporary = 0;    
    Timebase->UpCounter[i].Value = 0;
    Timebase->UpCounter[i].PeriodValue = 0;
  }
  #endif
  
  #ifdef TIMEBASE_UPCOUNTER_SUBSECONDS
  for(uint8_t i=0; i < TIMEBASE_UPCOUNTER_SUBSECONDS; i++){
    Timebase->UpCounterSS[i].Status.StatusByte = 0;  
    Timebase->UpCounterSS[i].EndValueSec = 0;
	Timebase->UpCounterSS[i].EndValueSubSec = 0;
    Timebase->UpCounterSS[i].Target = 0;
    Timebase->UpCounterSS[i].Temporary = 0;    
    Timebase->UpCounterSS[i].Value = 0;
    Timebase->UpCounterSS[i].PeriodValue = 0;
  }
  #endif

  #ifdef TIMEBASE_DOWNCOUNTER
  for(uint8_t i=0; i < TIMEBASE_DOWNCOUNTER; i++){
    Timebase->DownCounter[i].Status.StatusByte = 0; 
    Timebase->DownCounter[i].EndValue = 0;
    Timebase->DownCounter[i].Value = 0;
    Timebase->DownCounter[i].PeriodValue = 0;
  }
  #endif
  
  #ifdef TIMEBASE_DOWNCOUNTER_SUBSECONDS
  for(uint8_t i=0; i < TIMEBASE_DOWNCOUNTER_SUBSECONDS; i++){
    Timebase->DownCounterSS[i].Status.StatusByte = 0; 
    Timebase->DownCounterSS[i].EndValueSec = 0;
	Timebase->DownCounterSS[i].EndValueSubSec = 0;
    Timebase->DownCounterSS[i].Value = 0;
    Timebase->DownCounterSS[i].PeriodValue = 0;
  }
  #endif
  
}

//Minimum 15Hz
void Timebase_Timer_Config(uint16_t UpdateRateHz){
  RCC->APBENR1 |= RCC_APBENR1_TIM3EN;
	//TIM3->CR1 |= TIM_CR1_ARPE;
	//Prescaler 16000, to get 1ms
	TIM3->PSC  = 15;
	TIM3->ARR  = 1000000/UpdateRateHz;
	TIM3->DIER|= TIM_DIER_UIE;
	TIM3->CR1 |= TIM_CR1_CEN;
	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_SetPriority(TIM3_IRQn, 1);
  Timebase->Config.UpdateRate=UpdateRateHz;
}


/**********************************Basic Functions End********************************/









/********************************Atomic Functions Start*******************************/


void Timebase_Atomic_Operation_Start(void){
  __disable_irq();
}

void Timebase_Atomic_Operation_End(void){
  __enable_irq();
}


/*********************************Atomic Functions End********************************/









/*********************************Token Functions Start*******************************/

#ifdef TIMEBASE_TOKEN_FUNCTIONS
uint8_t Timebase_Token_Executing(void){
  return Timebase->ActiveTokens;
}

void Timebase_Token_Add(void){ 
  if( Timebase_Token_Executing() < 63){
    Timebase->ActiveTokens += 1;
  }
}

void Timebase_Token_Remove(void){
  if( Timebase_Token_Executing() > 0){
    Timebase->ActiveTokens -= 1;
  }
}

void Timebase_Token_Remove_All(void){
  Timebase->ActiveTokens = 0;
}
#endif

/*********************************Token Functions End********************************/









/*****************************Base Timer Functions Start*****************************/

uint16_t Timebase_Timer_Get_SubSecondsShadow(void){
  return Timebase->Time.SubSecondsShadow;
}

int32_t Timebase_Timer_Get_SecondsShadow(void){
  return Timebase->Time.SecondsShadow;
}


uint16_t Timebase_Timer_Get_SubSeconds(void){
  return Timebase->Time.SubSeconds;
}


int32_t Timebase_Timer_Get_Seconds(void){
  return Timebase->Time.Seconds;
}



void Timebase_Timer_Set_SubSeconds(uint16_t value){
  Timebase->Time.SubSeconds = value;
}

void Timebase_Timer_Set_Seconds(int32_t value){
  Timebase->Time.Seconds = value;
}

void Timebase_Timer_Sync_With_Shadow_Variables(void){
  if(Timebase->Time.VariablesSync == TIMEBASE_FALSE){
  
	#ifdef TIMEBASE_COUNT_ATOMIC_OPERATION
    Timebase_Atomic_Operation_Start();
    Timebase->Time.SubSeconds = Timebase->Time.SubSecondsShadow;
    Timebase->Time.Seconds    = Timebase->Time.SecondsShadow;
    Timebase_Atomic_Operation_End();
    #else
    #warning Shadow Variables Atomic Sync Turned Off
    Timebase->Time.SubSeconds = Timebase->Time.SubSecondsShadow;
    Timebase->Time.Seconds    = Timebase->Time.SecondsShadow;
    #endif
	
	Timebase->Time.VariablesSync = TIMEBASE_TRUE;
  }
}

void Timebase_Timer_Delay_SubSeconds(uint16_t value){
  
  int32_t smpl_val = 0, curr_val = 0;
  int32_t smpl_ss  = 0, smpl_s   = 0;
  int32_t curr_ss  = 0, curr_s   = 0;
  
  Timebase_Atomic_Operation_Start();
  smpl_ss  = Timebase_Timer_Get_SubSecondsShadow();
  smpl_s   = Timebase_Timer_Get_SecondsShadow();
  Timebase_Atomic_Operation_End();
  
  smpl_val  = smpl_s;
  smpl_val *= Timebase->Config.UpdateRate;
  smpl_val += smpl_ss;
  smpl_val += value;
  
  while(curr_val<smpl_val){
    Timebase_Atomic_Operation_Start();
    curr_ss   = Timebase_Timer_Get_SubSecondsShadow();
	curr_s    = Timebase_Timer_Get_SecondsShadow();
	Timebase_Atomic_Operation_End();
    curr_val  = curr_s;
	curr_val *= Timebase->Config.UpdateRate;
	curr_val += curr_ss;
  }
  
}


void Timebase_Timer_Await_SubSeconds(uint16_t value){
  uint16_t temp=0;
  Timebase_Atomic_Operation_Start();
  temp = Timebase_Timer_Get_SubSecondsShadow();
  Timebase_Atomic_Operation_End();
  while(temp != Timebase->Time.LastSample){
    Timebase_Atomic_Operation_Start();
    temp = Timebase_Timer_Get_SubSecondsShadow();
    Timebase_Atomic_Operation_End();
  }
  Timebase_Atomic_Operation_Start();
  temp = Timebase_Timer_Get_SubSecondsShadow();
  Timebase_Atomic_Operation_End();
  Timebase->Time.LastSample = temp;
  Timebase->Time.LastSample += value;
  if(Timebase->Time.LastSample >= Timebase->Config.UpdateRate){
    Timebase->Time.LastSample -= Timebase->Config.UpdateRate;
  }
}


void Timebase_Timer_Delay_Seconds(uint16_t value){
  int32_t curr_s = 0, target_s = 0;
  Timebase_Atomic_Operation_Start();
  target_s = Timebase_Timer_Get_SecondsShadow();
  Timebase_Atomic_Operation_End();
  target_s += value;
  while(target_s > curr_s){
    Timebase_Atomic_Operation_Start();
    curr_s = Timebase_Timer_Get_SecondsShadow();
	Timebase_Atomic_Operation_End();
  }
}

/******************************Base Timer Functions End******************************/









/****************************Time Window Functions Start*****************************/

#ifdef TIMEBASE_TIME_WINDOW_CALCULATION
void Timebase_Window_Timer_Reset(void){
  Timebase->Time.StartTimeSeconds = 0;
  Timebase->Time.StartTimeSubSeconds = 0;
  Timebase->Time.Status = COUNTER_STATE_RESET;
}

void Timebase_Window_Timer_Start(void){
  if(Timebase->Time.Status == COUNTER_STATE_RESET){
    Timebase_Atomic_Operation_Start();
    Timebase->Time.StartTimeSeconds = Timebase_Timer_Get_SecondsShadow();
    Timebase->Time.StartTimeSubSeconds = Timebase_Timer_Get_SubSecondsShadow();
	Timebase_Atomic_Operation_End();
    Timebase->Time.Status = COUNTER_STATE_STARTED;
  }
}


int32_t Timebase_Window_Timer_Get_Interval(void){
  int32_t curr_ss = 0, curr_s = 0;
  if(Timebase->Time.Status == COUNTER_STATE_STARTED){
    Timebase_Atomic_Operation_Start();
    curr_s = Timebase_Timer_Get_SecondsShadow();
    curr_ss = Timebase_Timer_Get_SubSecondsShadow();
	Timebase_Atomic_Operation_End();
	curr_s -= Timebase->Time.StartTimeSeconds;
	curr_ss -= Timebase->Time.StartTimeSubSeconds;
	curr_s *= Timebase->Config.UpdateRate;
	curr_s += curr_ss;
	return curr_s;
  }else{
    return 0;
  }
}

int32_t Timebase_Window_Timer_Get_Interval_Reset(void){
  int32_t val=Timebase_Window_Timer_Get_Interval();
  if(Timebase->Time.Status == COUNTER_STATE_STARTED){
    Timebase_Window_Timer_Reset();
  }
  return val;
}

#endif

/*****************************Time Window Functions End******************************/









/****************************UpCounter SS Functions Start****************************/

#ifdef TIMEBASE_UPCOUNTER_SUBSECONDS
uint8_t Timebase_UpCounter_SS_Get_Status(uint8_t window){
  return Timebase->UpCounterSS[window].Status.Value;
}

void Timebase_UpCounter_SS_Set_Status(uint8_t window, uint8_t value){
  Timebase->UpCounterSS[window].Status.Value = value;
}

int32_t Timebase_UpCounter_SS_Get_Value(uint8_t window){
  return Timebase->UpCounterSS[window].Value;
}

void Timebase_UpCounter_SS_Set_Value(uint8_t window, int32_t value){
  if(value < 0){
    value = 0;
  }
  Timebase->UpCounterSS[window].Value = value;
}

int32_t Timebase_UpCounter_SS_Get_EndValueSec(uint8_t window){
  return Timebase->UpCounterSS[window].EndValueSec;
}

int32_t Timebase_UpCounter_SS_Get_EndValueSubSec(uint8_t window){
  return Timebase->UpCounterSS[window].EndValueSubSec;
}

void Timebase_UpCounter_SS_Set_EndValueSec(uint8_t window, int32_t value){
  Timebase->UpCounterSS[window].EndValueSec = value;
}

void Timebase_UpCounter_SS_Set_EndValueSubSec(uint8_t window, int32_t value){
  Timebase->UpCounterSS[window].EndValueSubSec = value;
}

int32_t Timebase_UpCounter_SS_Get_TargetValue(uint8_t window){
  return Timebase->UpCounterSS[window].Target;
}

void Timebase_UpCounter_SS_Set_TargetValue(uint8_t window, int32_t value){
  Timebase->UpCounterSS[window].Target = value;
}

int32_t Timebase_UpCounter_SS_Get_TemporaryValue(uint8_t window){
  return Timebase->UpCounterSS[window].Temporary;
}

void Timebase_UpCounter_SS_Set_TemporaryValue(uint8_t window, int32_t value){
  if(value<0){
    value = 0;
  }
  Timebase->UpCounterSS[window].Temporary = value;
}

int32_t Timebase_UpCounter_SS_Get_PeriodValue(uint8_t window){
  return Timebase->UpCounterSS[window].PeriodValue;
}

void Timebase_UpCounter_SS_Set_PeriodValue(uint8_t window, int32_t value){
  Timebase->UpCounterSS[window].PeriodValue = value;
}

uint8_t Timebase_UpCounter_SS_Get_Period_Flag(uint8_t window){
  return Timebase->UpCounterSS[window].Status.PeriodFlag;
}

void Timebase_UpCounter_SS_Set_Period_Flag(uint8_t window){
  Timebase->UpCounterSS[window].Status.PeriodFlag = FLAG_STATE_SET;
}

void Timebase_UpCounter_SS_Clear_Period_Flag(uint8_t window){
  Timebase->UpCounterSS[window].Status.PeriodFlag = FLAG_STATE_RESET;
}

void Timebase_UpCounter_SS_Reset(uint8_t window){
  Timebase_UpCounter_SS_Set_Status(window, COUNTER_STATE_RESET);
  Timebase_UpCounter_SS_Set_Value(window, 0);
  Timebase_UpCounter_SS_Set_EndValueSec(window, 0);
  Timebase_UpCounter_SS_Set_EndValueSubSec(window, 0);
  Timebase_UpCounter_SS_Set_TargetValue(window, 0);
  Timebase_UpCounter_SS_Set_TemporaryValue(window, 0);
  Timebase_UpCounter_SS_Set_PeriodValue(window, 0);
  Timebase_UpCounter_SS_Clear_Period_Flag(window);
} 

void Timebase_UpCounter_SS_Clear_All_Flags(uint8_t window){
  Timebase_UpCounter_SS_Reset( window );
}

void Timebase_UpCounter_SS_Start(uint8_t window){
  if(Timebase_UpCounter_SS_Get_Status(window) != COUNTER_STATE_STARTED){
    Timebase_UpCounter_SS_Set_Status(window , COUNTER_STATE_START);
  }
}

void Timebase_UpCounter_SS_Stop(uint8_t window){
  if(Timebase_UpCounter_SS_Get_Status(window) != COUNTER_STATE_STOPPED){
    Timebase_UpCounter_SS_Set_Status(window , COUNTER_STATE_STOP);
  }
}


void Timebase_UpCounter_SS_Set_Securely(uint8_t window, int32_t value){
  int32_t curr_s, curr_ss;
  if( Timebase_UpCounter_SS_Get_Status( window ) == COUNTER_STATE_RESET ){
    Timebase_UpCounter_SS_Set_Value(window, 0);
    Timebase_UpCounter_SS_Set_TemporaryValue(window, 0);
    Timebase_UpCounter_SS_Set_TargetValue(window, value);
	curr_ss = Timebase_Timer_Get_SubSeconds();
	curr_s  = Timebase_Timer_Get_Seconds();
	int32_t subsec_val = value % Timebase->Config.UpdateRate;
	int32_t sec_val    = value / Timebase->Config.UpdateRate;
	subsec_val += curr_ss;
	if(subsec_val >= Timebase->Config.UpdateRate){
	  sec_val += 1;
	  subsec_val %= Timebase->Config.UpdateRate;
	}
	sec_val += curr_s;
    Timebase_UpCounter_SS_Set_EndValueSec(window, sec_val);
	Timebase_UpCounter_SS_Set_EndValueSubSec(window, subsec_val);
    Timebase_UpCounter_SS_Start(window);
  }
}

void Timebase_UpCounter_SS_Set_Forcefully(uint8_t window, int32_t value){
  Timebase_UpCounter_SS_Reset( window );
  Timebase_UpCounter_SS_Set_Securely( window, value );
} 

void Timebase_UpCounter_SS_Update(uint8_t window){
  int32_t curr_s, curr_ss;
  if( Timebase_UpCounter_SS_Get_Status( window ) == COUNTER_STATE_STARTED ){
	curr_s  = Timebase_Timer_Get_Seconds();
	curr_ss = Timebase_Timer_Get_SubSeconds();
	curr_s  = Timebase_UpCounter_SS_Get_EndValueSec(window) - curr_s;
	if(curr_s < 0){
	  curr_s = 0;
	}
	curr_s *= Timebase->Config.UpdateRate;
	curr_ss = Timebase_UpCounter_SS_Get_EndValueSubSec(window) - curr_ss;
	curr_ss += curr_s;
    Timebase_UpCounter_SS_Set_TemporaryValue(window, curr_ss );
    Timebase_UpCounter_SS_Set_Value(window, Timebase_UpCounter_SS_Get_TargetValue(window) - Timebase_UpCounter_SS_Get_TemporaryValue(window) );
    if(Timebase_UpCounter_SS_Get_TemporaryValue(window) <= 0){
      Timebase_UpCounter_SS_Set_EndValueSec(window, 0);
	  Timebase_UpCounter_SS_Set_EndValueSubSec(window, 0);
      Timebase_UpCounter_SS_Set_TemporaryValue(window, 0);
      Timebase_UpCounter_SS_Set_Value(window, Timebase_UpCounter_SS_Get_TargetValue(window));
      Timebase_UpCounter_SS_Set_Status(window, COUNTER_STATE_EXPIRED);
    }
  } else if (Timebase_UpCounter_SS_Get_Status( window ) == COUNTER_STATE_STOPPED){
    curr_s  = Timebase_Timer_Get_Seconds();
	curr_ss = Timebase_Timer_Get_SubSeconds();
	int32_t value = Timebase_UpCounter_SS_Get_TemporaryValue(window);
	int32_t subsec_val = value % Timebase->Config.UpdateRate;
	int32_t sec_val    = value / Timebase->Config.UpdateRate;
	subsec_val += curr_ss;
	if(subsec_val >= Timebase->Config.UpdateRate){
	  sec_val += 1;
	  subsec_val %= Timebase->Config.UpdateRate;
	}
	sec_val += curr_s;
	Timebase_UpCounter_SS_Set_EndValueSec(window, sec_val);
	Timebase_UpCounter_SS_Set_EndValueSubSec(window, subsec_val);
    Timebase_UpCounter_SS_Set_Value(window, Timebase_UpCounter_SS_Get_TargetValue(window) - Timebase_UpCounter_SS_Get_TemporaryValue(window));
  }
}


uint8_t Timebase_UpCounter_SS_Expired(uint8_t window){
  if(Timebase_UpCounter_SS_Get_Status( window ) == COUNTER_STATE_EXPIRED){
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

uint8_t Timebase_UpCounter_SS_Expired_Event(uint8_t window){
  if(Timebase_UpCounter_SS_Get_Status( window ) == COUNTER_STATE_EXPIRED){
    Timebase_UpCounter_SS_Clear_All_Flags( window );
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

void Timebase_UpCounter_SS_Set_Period_Value_Securely(uint8_t window, int32_t value){
  if(Timebase_UpCounter_SS_Get_Period_Flag( window ) == FLAG_STATE_RESET){
    Timebase_UpCounter_SS_Set_PeriodValue( window, Timebase_UpCounter_SS_Get_Value( window ) + value);
    Timebase_UpCounter_SS_Set_Period_Flag( window);
  }
}

int32_t Timebase_UpCounter_SS_Get_Remaining_Period_Value(uint8_t window){
  int32_t temp = Timebase_UpCounter_SS_Get_PeriodValue( window );
  temp -= Timebase_UpCounter_SS_Get_Value( window );
  if(temp < 0){
    temp = 0;
  }
  return temp;
}

uint8_t Timebase_UpCounter_SS_Period_Value_Expired(uint8_t window){
  if( (Timebase_UpCounter_SS_Get_Remaining_Period_Value( window ) == 0) && (Timebase_UpCounter_SS_Get_Period_Flag( window ) == FLAG_STATE_SET) ){
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

uint8_t Timebase_UpCounter_SS_Period_Value_Expired_Event(uint8_t window){
  if(Timebase_UpCounter_SS_Period_Value_Expired( window ) == TIMEBASE_TRUE){
    Timebase_UpCounter_SS_Clear_Period_Flag( window );
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

void Timebase_UpCounter_SS_Update_All(void){
  for(uint8_t i=0; i<TIMEBASE_UPCOUNTER; i++){
    Timebase_UpCounter_SS_Update(i);
  }
}

void Timebase_UpCounter_SS_Reset_All(void){
  for(uint8_t i=0; i<TIMEBASE_UPCOUNTER; i++){
    Timebase_UpCounter_SS_Reset(i);
  }
}
#endif

/*****************************UpCounter SS Functions End*****************************/









/******************************UpCounter Functions Start*****************************/

#ifdef TIMEBASE_UPCOUNTER
uint8_t Timebase_UpCounter_Get_Status(uint8_t window){
  return Timebase->UpCounter[window].Status.Value;
}

void Timebase_UpCounter_Set_Status(uint8_t window, uint8_t value){
  Timebase->UpCounter[window].Status.Value = value;
}

int32_t Timebase_UpCounter_Get_Value(uint8_t window){
  return Timebase->UpCounter[window].Value;
}

void Timebase_UpCounter_Set_Value(uint8_t window, int32_t value){
  if(value < 0){
    value = 0;
  }
  Timebase->UpCounter[window].Value = value;
}

int32_t Timebase_UpCounter_Get_EndValueSec(uint8_t window){
  return Timebase->UpCounter[window].EndValue;
}

int32_t Timebase_UpCounter_Get_EndValueSubSec(uint8_t window){
  return Timebase->UpCounter[window].EndValue;
}

void Timebase_UpCounter_Set_EndValueSec(uint8_t window, int32_t value){
  Timebase->UpCounter[window].EndValue = value;
}

void Timebase_UpCounter_Set_EndValueSubSec(uint8_t window, int32_t value){
  Timebase->UpCounter[window].EndValue = value;
}

int32_t Timebase_UpCounter_Get_TargetValue(uint8_t window){
  return Timebase->UpCounter[window].Target;
}

void Timebase_UpCounter_Set_TargetValue(uint8_t window, int32_t value){
  Timebase->UpCounter[window].Target = value;
}

int32_t Timebase_UpCounter_Get_TemporaryValue(uint8_t window){
  return Timebase->UpCounter[window].Temporary;
}

void Timebase_UpCounter_Set_TemporaryValue(uint8_t window, int32_t value){
  if(value < 0){
    value = 0;
  }
  Timebase->UpCounter[window].Temporary = value;
}

int32_t Timebase_UpCounter_Get_PeriodValue(uint8_t window){
  return Timebase->UpCounter[window].PeriodValue;
}

void Timebase_UpCounter_Set_PeriodValue(uint8_t window, int32_t value){
  Timebase->UpCounter[window].PeriodValue = value;
}

uint8_t Timebase_UpCounter_Get_Period_Flag(uint8_t window){
  return Timebase->UpCounter[window].Status.PeriodFlag;
}

void Timebase_UpCounter_Set_Period_Flag(uint8_t window){
  Timebase->UpCounter[window].Status.PeriodFlag = FLAG_STATE_SET;
}

void Timebase_UpCounter_Clear_Period_Flag(uint8_t window){
  Timebase->UpCounter[window].Status.PeriodFlag = FLAG_STATE_RESET;
}

void Timebase_UpCounter_Reset(uint8_t window){
  Timebase_UpCounter_Set_Status(window, COUNTER_STATE_RESET);
  Timebase_UpCounter_Set_Value(window, 0);
  Timebase_UpCounter_Set_EndValueSec(window, 0);
  Timebase_UpCounter_Set_EndValueSubSec(window, 0);
  Timebase_UpCounter_Set_TargetValue(window, 0);
  Timebase_UpCounter_Set_TemporaryValue(window, 0);
  Timebase_UpCounter_Set_PeriodValue(window, 0);
  Timebase_UpCounter_Clear_Period_Flag(window);
} 

void Timebase_UpCounter_Clear_All_Flags(uint8_t window){
  Timebase_UpCounter_Reset( window );
}

void Timebase_UpCounter_Start(uint8_t window){
  if(Timebase_UpCounter_Get_Status(window) != COUNTER_STATE_STARTED){
    Timebase_UpCounter_Set_Status(window , COUNTER_STATE_START);    
  }
}

void Timebase_UpCounter_Stop(uint8_t window){
  if(Timebase_UpCounter_Get_Status(window) != COUNTER_STATE_STOPPED){
    Timebase_UpCounter_Set_Status(window , COUNTER_STATE_STOP);
  }
}


void Timebase_UpCounter_Set_Securely(uint8_t window, int32_t value){
  int32_t curr_s;
  if( Timebase_UpCounter_Get_Status( window ) == COUNTER_STATE_RESET ){
    Timebase_UpCounter_Set_Value(window, 0);
    Timebase_UpCounter_Set_TemporaryValue(window, 0);
    Timebase_UpCounter_Set_TargetValue(window, value);
	curr_s = Timebase_Timer_Get_Seconds();
    Timebase_UpCounter_Set_EndValueSec(window, curr_s + value);
    Timebase_UpCounter_Start(window);
  }
}

void Timebase_UpCounter_Set_Forcefully(uint8_t window, int32_t value){
  Timebase_UpCounter_Reset( window );
  Timebase_UpCounter_Set_Securely( window, value );
} 

void Timebase_UpCounter_Update(uint8_t window){
  int32_t curr_s = 0;
  if( Timebase_UpCounter_Get_Status( window ) == COUNTER_STATE_STARTED ){
	curr_s = Timebase_Timer_Get_Seconds();
    Timebase_UpCounter_Set_TemporaryValue(window, Timebase_UpCounter_Get_EndValueSec(window) - curr_s );
    Timebase_UpCounter_Set_Value(window, Timebase_UpCounter_Get_TargetValue(window) - Timebase_UpCounter_Get_TemporaryValue(window) );
    if(Timebase_UpCounter_Get_TemporaryValue(window) <= 0){
      Timebase_UpCounter_Set_EndValueSec(window, 0);
      Timebase_UpCounter_Set_TemporaryValue(window, 0);
      Timebase_UpCounter_Set_Value(window, Timebase_UpCounter_Get_TargetValue(window));
      Timebase_UpCounter_Set_Status(window, COUNTER_STATE_EXPIRED);
    }
  } else if (Timebase_UpCounter_Get_Status( window ) == COUNTER_STATE_STOPPED){
    curr_s = Timebase_Timer_Get_Seconds();
    Timebase_UpCounter_Set_EndValueSec(window, Timebase_UpCounter_Get_TemporaryValue(window) + curr_s);
    Timebase_UpCounter_Set_Value(window, Timebase_UpCounter_Get_TargetValue(window) - Timebase_UpCounter_Get_TemporaryValue(window));
  }
}


uint8_t Timebase_UpCounter_Expired(uint8_t window){
  if(Timebase_UpCounter_Get_Status( window ) == COUNTER_STATE_EXPIRED){
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

uint8_t Timebase_UpCounter_Expired_Event(uint8_t window){
  if(Timebase_UpCounter_Get_Status( window ) == COUNTER_STATE_EXPIRED){
    Timebase_UpCounter_Clear_All_Flags( window );
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

void Timebase_UpCounter_Set_Period_Value_Securely(uint8_t window, int32_t value){
  if(Timebase_UpCounter_Get_Period_Flag( window ) == FLAG_STATE_RESET){
    Timebase_UpCounter_Set_PeriodValue( window, Timebase_UpCounter_Get_Value( window ) + value);
    Timebase_UpCounter_Set_Period_Flag( window);
  }
}

int32_t Timebase_UpCounter_Get_Remaining_Period_Value(uint8_t window){
  int32_t temp = Timebase_UpCounter_Get_PeriodValue( window ) - Timebase_UpCounter_Get_Value( window );
  if(temp < 0){
    temp = 0;
  }
  return temp;
}

uint8_t Timebase_UpCounter_Period_Value_Expired(uint8_t window){
  if( (Timebase_UpCounter_Get_Remaining_Period_Value( window ) == 0) && (Timebase_UpCounter_Get_Period_Flag( window ) == FLAG_STATE_SET) ){
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

uint8_t Timebase_UpCounter_Period_Value_Expired_Event(uint8_t window){
  if(Timebase_UpCounter_Period_Value_Expired( window ) == TIMEBASE_TRUE){
    Timebase_UpCounter_Clear_Period_Flag( window );
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

void Timebase_UpCounter_Update_All(void){
  for(uint8_t i=0; i<TIMEBASE_UPCOUNTER; i++){
    Timebase_UpCounter_Update(i);
  }
}

void Timebase_UpCounter_Reset_All(void){
  for(uint8_t i=0; i<TIMEBASE_UPCOUNTER; i++){
    Timebase_UpCounter_Reset(i);
  }
}
#endif

/*******************************UpCounter Functions End******************************/









/***************************DownCounter SS Functions Start**************************/

#ifdef TIMEBASE_DOWNCOUNTER_SUBSECONDS
uint8_t Timebase_DownCounter_SS_Get_Status(uint8_t window){
  return Timebase->DownCounterSS[window].Status.Value;
}

void Timebase_DownCounter_SS_Set_Status(uint8_t window, uint8_t value){
  Timebase->DownCounterSS[window].Status.Value = value;
}

int32_t Timebase_DownCounter_SS_Get_Value(uint8_t window){
  return Timebase->DownCounterSS[window].Value;
}

void Timebase_DownCounter_SS_Set_Value(uint8_t window, int32_t value){
  Timebase->DownCounterSS[window].Value = value;
}

int32_t Timebase_DownCounter_SS_Get_EndValueSec(uint8_t window){
  return Timebase->DownCounterSS[window].EndValueSec;
}

int32_t Timebase_DownCounter_SS_Get_EndValueSubSec(uint8_t window){
  return Timebase->DownCounterSS[window].EndValueSubSec;
}

void Timebase_DownCounter_SS_Set_EndValueSec(uint8_t window, int32_t value){
  Timebase->DownCounterSS[window].EndValueSec = value;
}

void Timebase_DownCounter_SS_Set_EndValueSubSec(uint8_t window, int32_t value){
  Timebase->DownCounterSS[window].EndValueSubSec = value;
}


int32_t Timebase_DownCounter_SS_Get_PeriodValue(uint8_t window){
  return Timebase->DownCounterSS[window].PeriodValue;
}

void Timebase_DownCounter_SS_Set_PeriodValue(uint8_t window, int32_t value){
  if(value < 0){
    Timebase->DownCounterSS[window].PeriodValue = 0;
  }else{
    Timebase->DownCounterSS[window].PeriodValue = value;
  }
  
}

uint8_t Timebase_DownCounter_SS_Get_Period_Flag(uint8_t window){
  return Timebase->DownCounterSS[window].Status.PeriodFlag;
}

void Timebase_DownCounter_SS_Set_Period_Flag(uint8_t window){
  Timebase->DownCounterSS[window].Status.PeriodFlag = FLAG_STATE_SET;
}

void Timebase_DownCounter_SS_Clear_Period_Flag(uint8_t window){
  Timebase->DownCounterSS[window].Status.PeriodFlag = FLAG_STATE_RESET;
}


void Timebase_DownCounter_SS_Reset(uint8_t window){
  Timebase_DownCounter_SS_Set_EndValueSubSec(window, 0);
  Timebase_DownCounter_SS_Set_EndValueSec(window, 0);
  Timebase_DownCounter_SS_Set_Value(window, 0);
  Timebase_DownCounter_SS_Set_Status(window, COUNTER_STATE_RESET);
  Timebase_DownCounter_SS_Clear_Period_Flag(window);
} 

void Timebase_DownCounter_SS_Clear_All_Flags(uint8_t window){
  Timebase_DownCounter_SS_Reset( window );
}

void Timebase_DownCounter_SS_Start(uint8_t window){
  if(Timebase_DownCounter_SS_Get_Status(window) != COUNTER_STATE_STARTED){
    Timebase_DownCounter_SS_Set_Status(window, COUNTER_STATE_START); 
  }
}

void Timebase_DownCounter_SS_Stop(uint8_t window){
  if(Timebase_DownCounter_SS_Get_Status(window) != COUNTER_STATE_STOPPED){
    Timebase_DownCounter_SS_Set_Status(window, COUNTER_STATE_STOP);    
  }
}


void Timebase_DownCounter_SS_Set_Securely(uint8_t window, int32_t value){
  if( Timebase_DownCounter_SS_Get_Status( window ) == COUNTER_STATE_RESET ){
    Timebase_DownCounter_SS_Set_Value(window, value);
	int32_t temp_ss = Timebase_Timer_Get_SubSeconds();
	int32_t temp_s  = Timebase_Timer_Get_Seconds();
	int32_t subsec_val = value % Timebase->Config.UpdateRate;
	int32_t sec_val    = value / Timebase->Config.UpdateRate;
	subsec_val += temp_ss;
	if(subsec_val >= Timebase->Config.UpdateRate){
	  sec_val += 1;
	  subsec_val = subsec_val % Timebase->Config.UpdateRate;
	}
	sec_val += temp_s;
    Timebase_DownCounter_SS_Set_EndValueSec(window, sec_val);
	Timebase_DownCounter_SS_Set_EndValueSubSec(window, subsec_val);
    Timebase_DownCounter_SS_Start(window);
  }
}

void Timebase_DownCounter_SS_Set_Forcefully(uint8_t window, int32_t value){
  Timebase_DownCounter_SS_Reset( window );
  Timebase_DownCounter_SS_Set_Securely( window, value );
} 

void Timebase_DownCounter_SS_Update(uint8_t window){
  if( Timebase_DownCounter_SS_Get_Status( window ) == COUNTER_STATE_STARTED ){
    int32_t temp_ss = Timebase_Timer_Get_SubSeconds();
	int32_t temp_s  = Timebase_Timer_Get_Seconds();
	temp_ss = Timebase_DownCounter_SS_Get_EndValueSubSec(window) - temp_ss;
	temp_s = Timebase_DownCounter_SS_Get_EndValueSec(window) - temp_s;
	temp_s *= Timebase->Config.UpdateRate;
	temp_s += temp_ss;
	Timebase_DownCounter_SS_Set_Value(window, temp_s);
    if(Timebase_DownCounter_SS_Get_Value(window) <= 0){
      Timebase_DownCounter_SS_Reset(window);
      Timebase_DownCounter_SS_Set_Status(window, COUNTER_STATE_EXPIRED);
    }
  } else if (Timebase_DownCounter_SS_Get_Status( window ) == COUNTER_STATE_STOPPED){
    int32_t temp_ss  = Timebase_Timer_Get_SubSeconds();
	int32_t temp_s   = Timebase_Timer_Get_Seconds();
	int32_t curr_val = Timebase_DownCounter_SS_Get_Value(window);
	int32_t curr_s   = curr_val / Timebase->Config.UpdateRate;
	int32_t curr_ss  = curr_val % Timebase->Config.UpdateRate;
	Timebase_DownCounter_SS_Set_EndValueSec(window, curr_s + temp_s );
	Timebase_DownCounter_SS_Set_EndValueSubSec(window, curr_ss + temp_ss);
  }
}

uint8_t Timebase_DownCounter_SS_Expired(uint8_t window){
  if(Timebase_DownCounter_SS_Get_Status( window ) == COUNTER_STATE_EXPIRED){
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

uint8_t Timebase_DownCounter_SS_Expired_Event(uint8_t window){
  if(Timebase_DownCounter_SS_Get_Status( window ) == COUNTER_STATE_EXPIRED){
    Timebase_DownCounter_SS_Clear_All_Flags( window );
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}


void Timebase_DownCounter_SS_Set_Period_Value_Securely(uint8_t window, int32_t value){
  if(Timebase_DownCounter_SS_Get_Period_Flag( window ) == FLAG_STATE_RESET){
    Timebase_DownCounter_SS_Set_PeriodValue(window, Timebase_DownCounter_SS_Get_Value(window) - value);
    Timebase_DownCounter_SS_Set_Period_Flag(window);
  }
}

int32_t Timebase_DownCounter_SS_Get_Remaining_Period_Value(uint8_t window){
  int32_t temp = Timebase_DownCounter_SS_Get_Value(window) - Timebase_DownCounter_SS_Get_PeriodValue(window) ;
  if(temp < 0){
    temp = 0;
  }
  return temp;
}

uint8_t Timebase_DownCounter_SS_Period_Value_Expired(uint8_t window){
  if( (Timebase_DownCounter_SS_Get_Remaining_Period_Value( window ) == 0) && (Timebase_DownCounter_SS_Get_Period_Flag( window ) == FLAG_STATE_SET) ){
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

uint8_t Timebase_DownCounter_SS_Period_Value_Expired_Event(uint8_t window){
  if(Timebase_DownCounter_SS_Period_Value_Expired( window ) == TIMEBASE_TRUE){
    Timebase_DownCounter_SS_Clear_Period_Flag( window );
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}


void Timebase_DownCounter_SS_Update_All(void){
  for(uint8_t i=0; i<TIMEBASE_DOWNCOUNTER_SUBSECONDS; i++){
    Timebase_DownCounter_SS_Update(i);
  }
}

void Timebase_DownCounter_SS_Reset_All(void){
  for(uint8_t i=0; i<TIMEBASE_DOWNCOUNTER_SUBSECONDS; i++){
    Timebase_DownCounter_SS_Reset(i);
  }
}
#endif

/****************************DownCounter SS Functions End***************************/









/*****************************DownCounter Functions Start****************************/

#ifdef TIMEBASE_DOWNCOUNTER
uint8_t Timebase_DownCounter_Get_Status(uint8_t window){
  return Timebase->DownCounter[window].Status.Value;
}

void Timebase_DownCounter_Set_Status(uint8_t window, uint8_t value){
  Timebase->DownCounter[window].Status.Value = value;
}

int32_t Timebase_DownCounter_Get_Value(uint8_t window){
  return Timebase->DownCounter[window].Value;
}

void Timebase_DownCounter_Set_Value(uint8_t window, int32_t value){
  Timebase->DownCounter[window].Value = value;
}

int32_t Timebase_DownCounter_Get_EndValue(uint8_t window){
  return Timebase->DownCounter[window].EndValue;
}

void Timebase_DownCounter_Set_EndValue(uint8_t window, int32_t value){
  Timebase->DownCounter[window].EndValue = value;
}


int32_t Timebase_DownCounter_Get_PeriodValue(uint8_t window){
  return Timebase->DownCounter[window].PeriodValue;
}

void Timebase_DownCounter_Set_PeriodValue(uint8_t window, int32_t value){
  if(value < 0){
    Timebase->DownCounter[window].PeriodValue = 0;
  }else{
    Timebase->DownCounter[window].PeriodValue = value;
  }
  
}

uint8_t Timebase_DownCounter_Get_Period_Flag(uint8_t window){
  return Timebase->DownCounter[window].Status.PeriodFlag;
}

void Timebase_DownCounter_Set_Period_Flag(uint8_t window){
  Timebase->DownCounter[window].Status.PeriodFlag = FLAG_STATE_SET;
}

void Timebase_DownCounter_Clear_Period_Flag(uint8_t window){
  Timebase->DownCounter[window].Status.PeriodFlag = FLAG_STATE_RESET;
}


void Timebase_DownCounter_Reset(uint8_t window){
  Timebase_DownCounter_Set_EndValue(window, 0);
  Timebase_DownCounter_Set_Value(window, 0);
  Timebase_DownCounter_Set_Status(window, COUNTER_STATE_RESET);
  Timebase_DownCounter_Clear_Period_Flag(window);
} 

void Timebase_DownCounter_Clear_All_Flags(uint8_t window){
  Timebase_DownCounter_Reset( window );
}

void Timebase_DownCounter_Start(uint8_t window){
  if(Timebase_DownCounter_Get_Status(window) != COUNTER_STATE_STARTED){
    Timebase_DownCounter_Set_Status(window, COUNTER_STATE_START); 
  }
}

void Timebase_DownCounter_Stop(uint8_t window){
  if(Timebase_DownCounter_Get_Status(window) != COUNTER_STATE_STOPPED){
    Timebase_DownCounter_Set_Status(window, COUNTER_STATE_STOP);    
  }
}


void Timebase_DownCounter_Set_Securely(uint8_t window, int32_t value){
  if( Timebase_DownCounter_Get_Status( window ) == COUNTER_STATE_RESET ){
    Timebase_DownCounter_Set_Value(window, value);
    Timebase_DownCounter_Set_EndValue(window, Timebase_Timer_Get_Seconds() + value);
    Timebase_DownCounter_Start(window);
  }
}

void Timebase_DownCounter_Set_Forcefully(uint8_t window, int32_t value){
  Timebase_DownCounter_Reset( window );
  Timebase_DownCounter_Set_Securely( window, value );
} 

void Timebase_DownCounter_Update(uint8_t window){
  if( Timebase_DownCounter_Get_Status( window ) == COUNTER_STATE_STARTED ){ 
    Timebase_DownCounter_Set_Value(window, Timebase_DownCounter_Get_EndValue(window) - Timebase_Timer_Get_Seconds());
    if(Timebase_DownCounter_Get_Value(window) <= 0){
      Timebase_DownCounter_Reset(window);
      Timebase_DownCounter_Set_Status(window, COUNTER_STATE_EXPIRED);
    }
  } else if (Timebase_DownCounter_Get_Status( window ) == COUNTER_STATE_STOPPED){
    Timebase_DownCounter_Set_EndValue(window, Timebase_DownCounter_Get_Value(window) + Timebase_Timer_Get_Seconds());
  }
}

uint8_t Timebase_DownCounter_Expired(uint8_t window){
  if(Timebase_DownCounter_Get_Status( window ) == COUNTER_STATE_EXPIRED){
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

uint8_t Timebase_DownCounter_Expired_Event(uint8_t window){
  if(Timebase_DownCounter_Get_Status( window ) == COUNTER_STATE_EXPIRED){
    Timebase_DownCounter_Clear_All_Flags( window );
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}


void Timebase_DownCounter_Set_Period_Value_Securely(uint8_t window, int32_t value){
  if(Timebase_DownCounter_Get_Period_Flag( window ) == FLAG_STATE_RESET){
    Timebase_DownCounter_Set_PeriodValue(window, Timebase_DownCounter_Get_Value(window) - value);
    Timebase_DownCounter_Set_Period_Flag(window);
  }
}

int32_t Timebase_DownCounter_Get_Remaining_Period_Value(uint8_t window){
  int32_t temp = Timebase_DownCounter_Get_Value(window) - Timebase_DownCounter_Get_PeriodValue(window) ;
  if(temp < 0){
    temp = 0;
  }
  return temp;
}

uint8_t Timebase_DownCounter_Period_Value_Expired(uint8_t window){
  if( (Timebase_DownCounter_Get_Remaining_Period_Value( window ) == 0) && (Timebase_DownCounter_Get_Period_Flag( window ) == FLAG_STATE_SET) ){
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}

uint8_t Timebase_DownCounter_Period_Value_Expired_Event(uint8_t window){
  if(Timebase_DownCounter_Period_Value_Expired( window ) == TIMEBASE_TRUE){
    Timebase_DownCounter_Clear_Period_Flag( window );
    return TIMEBASE_TRUE;
  }else{
    return TIMEBASE_FALSE;
  }
}


void Timebase_DownCounter_Update_All(void){
  for(uint8_t i=0; i<TIMEBASE_DOWNCOUNTER; i++){
    Timebase_DownCounter_Update(i);
  }
}

void Timebase_DownCounter_Reset_All(void){
  for(uint8_t i=0; i<TIMEBASE_DOWNCOUNTER; i++){
    Timebase_DownCounter_Reset(i);
  }
}
#endif

/******************************DownCounter Functions End*****************************/









/*******************************Common Functions Start******************************/

void Timebase_Reset(void){
  #ifdef TIMEBASE_UPCOUNTER
  Timebase_UpCounter_Reset_All();
  #endif
  #ifdef TIMEBASE_DOWNCOUNTER_SUBSECONDS
  Timebase_DownCounter_SS_Reset_All();
  #endif
  #ifdef TIMEBASE_DOWNCOUNTER
  Timebase_DownCounter_Reset_All();
  #endif
  
}

void Timebase_Init(uint16_t UpdateRateHz){
  Timebase_Struct_Init();
  Timebase_Timer_Config(UpdateRateHz);
  Timebase_Reset();
}

void Timebase_Main_Loop_Executables(void){
  
  #ifdef TIMEBASE_UPCOUNTER_SUBSECONDS
  if(Timebase->UpdateRequest & UPCOUNTER_SS_UPDATE_REQ){
    Timebase_Timer_Sync_With_Shadow_Variables();
    Timebase_UpCounter_SS_Update_All();
	Timebase->UpdateRequest &=~ UPCOUNTER_SS_UPDATE_REQ;
  }
  #endif

  #ifdef TIMEBASE_UPCOUNTER
  if(Timebase->UpdateRequest & UPCOUNTER_UPDATE_REQ){
    Timebase_Timer_Sync_With_Shadow_Variables();
    Timebase_UpCounter_Update_All();
	Timebase->UpdateRequest &=~ UPCOUNTER_UPDATE_REQ;
  }
  #endif
  
  
  
  #ifdef TIMEBASE_DOWNCOUNTER_SUBSECONDS
  if(Timebase->UpdateRequest & DOWNCOUNTER_SS_UPDATE_REQ){
    Timebase_Timer_Sync_With_Shadow_Variables();
    Timebase_DownCounter_SS_Update_All();
	Timebase->UpdateRequest &=~ DOWNCOUNTER_SS_UPDATE_REQ;
  }
  #endif
  
  #ifdef TIMEBASE_DOWNCOUNTER
  if(Timebase->UpdateRequest & DOWNCOUNTER_UPDATE_REQ){
    Timebase_Timer_Sync_With_Shadow_Variables();
    Timebase_DownCounter_Update_All();
	Timebase->UpdateRequest &=~ DOWNCOUNTER_UPDATE_REQ;
  }
  #endif
  Timebase->Time.VariablesSync = TIMEBASE_FALSE;
}

void Timebase_ISR_Executables(void){
  Timebase->Time.SubSecondsShadow++;
  
  #ifdef TIMEBASE_UPCOUNTER_SUBSECONDS
  Timebase->UpdateRequest |= UPCOUNTER_SS_UPDATE_REQ;
  #endif
  
  #ifdef TIMEBASE_DOWNCOUNTER_SUBSECONDS
  Timebase->UpdateRequest |= DOWNCOUNTER_SS_UPDATE_REQ;
  #endif
  
  if(Timebase->Time.SubSecondsShadow >= Timebase->Config.UpdateRate){
    Timebase->Time.SecondsShadow++;
    Timebase->Time.SubSecondsShadow = 0;
	
	#ifdef TIMEBASE_UPCOUNTER
    Timebase->UpdateRequest |= UPCOUNTER_UPDATE_REQ;
    #endif
	
	#ifdef TIMEBASE_DOWNCOUNTER
    Timebase->UpdateRequest |= DOWNCOUNTER_UPDATE_REQ;
    #endif
  }
}

/********************************Common Functions End*******************************/









/*************************************ISR Start************************************/
void TIM3_IRQHandler(void){
    if(TIM3->SR & TIM_SR_UIF){
        Timebase_ISR_Executables();
        TIM3->SR &=~TIM_SR_UIF;
    }
}

/**************************************ISR End************************************/



/* Index of timer used in code:
Milli-second:
window-00: DISP_COMM_HEARTBEAT_INTERVAL_TIMER_WINDOW    0
window-01: WATER_DISPENSER_LED_BLINK_TIMER_WINDOW       1
window-02: For Uart to print debug data; loc: main.c
window-03: For detection of init ccw end Ice_Maker_Timebase_CCW_End_Detection()
window-04: For delay at CCW end


Second:
window-00: WATER_DISPENSER_MID_WATER_FILL_TIMER_WINDOW  0
window-01: WATER_DISPENSER_LOW_WATER_FILL_TIMER_WINDOW  1
window-02:
window-03: CONF_WINDOW_IM_ROTATE
*/


