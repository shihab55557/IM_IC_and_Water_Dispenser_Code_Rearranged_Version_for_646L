#include "stm32g070xx.h"

 void     NTC_Struct_Init(void);
 void     NTC_Sensor_GPIO_Init (void);
 void     NTC_Sensor_ADC_Init (void);
 uint16_t Read_ADC(uint8_t channel);
 //uint16_t NTC_Sensor_Reading(uint8_t channel,uint8_t i);
 uint16_t NTC_Sensor_Reading(uint8_t sensoor);
 //void     NTC_Calculate_Single_Sample(uint8_t channel,uint8_t i);
 void     NTC_Calculate_Single_Sample(uint8_t sensor);
 int      NTC_Get_RawADC(uint8_t i);
 int      NTC_Get_Temperature(uint8_t i);
 uint8_t  NTC_Get_Error(uint8_t i);
 void     NTC_Sensor_Init(void);
			


