#include "stm32g070xx.h"

#define  DISP_COMM_RAW_BUF_SIZE 128U

#define  DISP_COMM_USE_CRC_CVTE
//#define  DISP_COMM_USE_CRC_X_MODEM
#define  DISP_COMM_DEBUG_EN


#define  DISP_COMM_HEARTBEAT_INTERVAL_TIMER_WINDOW            0
#define  DISP_COMM_HEARTBEAT_INTERVAL                         500

#define  DISP_COMM_MASTER_DATA_MODE_ICE_MAKER_ON_OFF_BP       0
#define  DISP_COMM_MASTER_DATA_MODE_WATER_DISPENSER_ON_OFF_BP 2




void     Disp_Comm_HeartBeat_Struct_Init(void);
void     Disp_Comm_Variable_Init(void);
void     Disp_Comm_HeartBeat_Set_Fixed_Fields(void);
void     Disp_Comm_HeartBeat_Set_PacketID(uint8_t val);
void     Disp_Comm_HeartBeat_Set_Temperature(int16_t val);
void     Disp_Comm_HeartBeat_Set_WaterDispenserFlags(uint8_t val);
void     Disp_Comm_HeartBeat_Set_CubeIceDispenserFlags(uint8_t val);
void     Disp_Comm_HeartBeat_Set_CrushedIceDispenserFlags(uint8_t val);
void     Disp_Comm_HeartBeat_Set_IceMakingFlags(uint8_t val);
void     Disp_Comm_HeartBeat_Set_Feedback(uint8_t val);
void     Disp_Comm_HeartBeat_Set_Crc(uint16_t val);
uint16_t Disp_Comm_GetCRC16(uint8_t *buffer, uint16_t len);
void     Disp_Comm_HeartBeat_Send(void);


void     Disp_Comm_Master_Data_Struct_Init(void);
uint8_t  Disp_Comm_Master_Data_CRC_Verify(void);
uint8_t  Disp_Comm_Master_Data_Extract(void);
uint8_t  Disp_Comm_Master_Data_CRC_Stat(void);
void     Disp_Comm_Master_Data_Extract_Buffer_Reset(uint8_t Byte_pos, uint8_t Bit_pos);
uint8_t  Disp_Comm_Master_Data_Get_RawBuf(uint8_t index);
uint8_t  Disp_Comm_Master_Data_Get_RawBufLen(void);
uint8_t  Disp_Comm_Master_Data_Get_Header(void);
uint8_t  Disp_Comm_Master_Data_Get_Length(void);
uint8_t  Disp_Comm_Master_Data_Get_Mode(void);
uint8_t  Disp_Comm_Master_Data_Get_WaterDispenser(void);
uint8_t  Disp_Comm_Master_Data_Get_IceMaker(void);
uint8_t  Disp_Comm_Master_Data_Get_IMValveOnTime(void);
uint8_t  Disp_Comm_Master_Data_Get_IMValveOnCycle(void);
uint16_t Disp_Comm_Master_Data_Get_Calculated_Crc(void);
uint16_t Disp_Comm_Master_Data_Get_Received_Crc(void);
uint8_t  Disp_Comm_Master_Data_Get_Crc_Check_Status(void);
uint8_t  Disp_Comm_Master_Data_Get_Bit_Val(uint8_t reg, uint8_t bit_pos);
uint8_t  Disp_Comm_Master_Data_Get_Mode_Ice_Maker_Status(void);
uint8_t  Disp_Comm_Master_Data_Get_Mode_Water_Dispenser_Status(void);

void     Disp_Comm_Handler(void);

int8_t   Disp_Comm_IM_Control(void);
int8_t   Disp_Comm_IM_Control_2(void);

void     Disp_Comm_Init(void);

void Disp_Comm_Water_Disp_Set_Lever_Stat(uint8_t Wat_disp_lever_bit);
void Disp_Comm_Heartbeat_Set_Data(uint8_t Byte_pos ,uint16_t Data, uint8_t Data_Pos);
void Disp_Comm_Print_Heartbeat_Send_Data(void);
void Disp_Comm_Print_Master_Rec_Data(void);




