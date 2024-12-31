#include "stm32g070xx.h"
#include "disp_comm.h"
#include "timebase.h"
#include "disp.h"
#include "crc.h"
#include "Uart.h"
#include "Icemaker.h"
#include "glvars.h"


typedef struct master_data_t{
	uint8_t  RawBuf[DISP_COMM_RAW_BUF_SIZE];
	uint16_t RawBufLen;
	uint8_t  Header;
  uint8_t  Length;
  uint8_t  CMD;
  uint8_t  Mode;
  uint8_t  WaterDispenser;
  uint8_t  IceMaker;
  uint8_t  IMValveOnTime;
	uint8_t  IMValveOnCycle;
	uint16_t CalculatedCrc;
	uint16_t ReceivedCrc;
	uint8_t  CrcCheckOk;
}master_data_t;

typedef struct heart_beat_t{
	uint8_t  Header;
  uint8_t  Length;
  uint8_t  CMD;
	uint8_t  PacketID;
	int8_t   Temperature;
  uint8_t  Mode;
  uint8_t  WaterDispenserFlags;
  uint8_t  CubeIceDispenserFlags;
  uint8_t  CrushedIceDispenserFlags;
	uint8_t  IceMakingFlags;
	uint8_t  Feedback;
	uint16_t Crc;
}heart_beat_t;


master_data_t MasterData;
heart_beat_t  HeartBeat;





/*----------------------Heart beat Start------------------------*/
uint8_t HB_update_packet_id;
void Disp_Comm_Variable_Init(void){
		HB_update_packet_id = 0;
}

void Disp_Comm_HeartBeat_Struct_Init(void){
  HeartBeat.Header = 0;
	HeartBeat.Length = 0;
	HeartBeat.CMD = 0;
	HeartBeat.PacketID = 0;
	HeartBeat.Temperature = 0;
	HeartBeat.WaterDispenserFlags = 0;
	HeartBeat.CubeIceDispenserFlags = 0;
	HeartBeat.CrushedIceDispenserFlags = 0;
	HeartBeat.IceMakingFlags = 0;
	HeartBeat.Feedback = 0;
	HeartBeat.Crc = 0;
}


void Disp_Comm_HeartBeat_Set_Fixed_Fields(void){
	HeartBeat.Header = 0x55;
	HeartBeat.Length = 7;
	HeartBeat.CMD = 0x03;
}

void Disp_Comm_HeartBeat_Set_PacketID(uint8_t val){
	HeartBeat.PacketID = val;
}

void Disp_Comm_HeartBeat_Set_Temperature(int16_t val){
	HeartBeat.Temperature = val;
}

void Disp_Comm_HeartBeat_Set_WaterDispenserFlags(uint8_t val){
	HeartBeat.WaterDispenserFlags = val;
}

void Disp_Comm_HeartBeat_Set_CubeIceDispenserFlags(uint8_t val){
	HeartBeat.CubeIceDispenserFlags = val;
}

void Disp_Comm_HeartBeat_Set_CrushedIceDispenserFlags(uint8_t val){
	HeartBeat.CrushedIceDispenserFlags = val;
}

void Disp_Comm_HeartBeat_Set_IceMakingFlags(uint8_t val){
	HeartBeat.IceMakingFlags = val;
}


void Disp_Comm_HeartBeat_Set_Feedback(uint8_t val){
	HeartBeat.Feedback = val;
}

void Disp_Comm_HeartBeat_Set_Crc(uint16_t val){
	HeartBeat.Crc = val;
}


void Disp_Comm_Heartbeat_Set_Data(uint8_t Byte_pos ,uint16_t Data, uint8_t Data_Pos){
	if(Byte_pos == 3){
		HeartBeat.Temperature = Data;
	}
	
	else if(Byte_pos == 5){
		if(Data == 1){
			HeartBeat.WaterDispenserFlags |= (1<<Data_Pos);
		}
		else if(Data == 0){
			HeartBeat.WaterDispenserFlags &=~ (1<<Data_Pos);
		}
	}
	
	else if(Byte_pos == 6){
		if(Data == 1){
			HeartBeat.CubeIceDispenserFlags |= (1<<Data_Pos);
		}
		else if(Data == 0){
			HeartBeat.CubeIceDispenserFlags &=~ (1<<Data_Pos);
		}
	}
	
	else if(Byte_pos == 7){
		if(Data == 1){
			HeartBeat.CrushedIceDispenserFlags |= (1<<Data_Pos);
		}
		else if(Data == 0){
			HeartBeat.CrushedIceDispenserFlags &=~ (1<<Data_Pos);
		}
	}
	
	else if(Byte_pos == 8){
		if(Data == 1){
			HeartBeat.IceMakingFlags |= (1<<Data_Pos);
		}
		else if(Data == 0){
			HeartBeat.IceMakingFlags &=~ (1<<Data_Pos);
		}
	}
	
	else if(Byte_pos == 9){
		if(Data == 1){
			HeartBeat.Feedback |= (1<<Data_Pos);
		}
		else if(Data == 0){
			HeartBeat.Feedback &=~ (1<<Data_Pos);
		}
	}
}




void Disp_Comm_Water_Disp_Set_Lever_Stat(uint8_t Wat_disp_lever_bit){
	if(Wat_disp_lever_bit == 1){
		HeartBeat.WaterDispenserFlags |= (1 << 0);
	}
	else if(Wat_disp_lever_bit == 0){
		HeartBeat.WaterDispenserFlags &=~ (1 << 0);
	}
}






uint8_t  heartbeat_data[14];
/*
void Disp_Comm_Print_Heartbeat_Send_Data(void){
	PrintfD_BIN("S>M Header:",heartbeat_data[0]);
	PrintfNL(1);
	PrintfD_BIN("S>M Length:",heartbeat_data[1]);
	PrintfNL(1);
	PrintfD_BIN("S>M CMD:",heartbeat_data[2]);
	PrintfNL(1);
	PrintfD_BIN("S>M NTC:",heartbeat_data[3]);
	PrintfNL(1);
	PrintfD_BIN("S>M NTC:",heartbeat_data[4]);
	PrintfNL(1);
	PrintfD_BIN("S>M WaterDispFlag:",heartbeat_data[5]);
	PrintfNL(1);
	PrintfD_BIN("S>M CubeIceDispFlag:",heartbeat_data[6]);
	PrintfNL(1);
	PrintfD_BIN("S>M CrushedIceDispFlag::",heartbeat_data[7]);
	PrintfNL(1);
	PrintfD_BIN("S>M IceMakingFlag:",heartbeat_data[8]);
	PrintfNL(1);
	PrintfD_BIN("S>M Feedback:",heartbeat_data[9]);
	//PrintfNL(1);
}*/

void Disp_Comm_Print_Heartbeat_Send_Data(void){
	if(GlobalVars_Get_Start_Flag() && (GlobalVars_Get_Thread_Function() == 2)){
	  if(GlobalVars_Get_Curr_Thread() == 23){
			UART_PrintfD_NL("SM_Header ",HeartBeat.Header);
		}
		else if(GlobalVars_Get_Curr_Thread() == 24){
			UART_PrintfD_NL("SM_Length ",HeartBeat.Length);
		}
		else if(GlobalVars_Get_Curr_Thread() == 25){
			UART_PrintfD_NL("SM_CMD ",HeartBeat.CMD);
		}
		else if(GlobalVars_Get_Curr_Thread() == 26){
			UART_PrintfD_NL("SM_PacketID ",HeartBeat.PacketID);
		}
		else if(GlobalVars_Get_Curr_Thread() == 27){
			UART_PrintfD_NL("SM_NTC ",HeartBeat.Temperature);
		}
		else if(GlobalVars_Get_Curr_Thread() == 28){
			UART_PrintfD_NL("SM_WaterDispFlag ",HeartBeat.WaterDispenserFlags);
		}
		else if(GlobalVars_Get_Curr_Thread() == 29){
			UART_PrintfD_NL("SM_CubeIceDispFlag ",HeartBeat.CubeIceDispenserFlags);
		}
		else if(GlobalVars_Get_Curr_Thread() == 30){
			UART_PrintfD_NL("SM_CrushedIceDispFlag ",HeartBeat.CrushedIceDispenserFlags);
		}
		else if(GlobalVars_Get_Curr_Thread() == 31){
			UART_PrintfD_NL("SM_IceMakingFlag ",HeartBeat.IceMakingFlags);
		}
		else if(GlobalVars_Get_Curr_Thread() == 32){
			UART_PrintfD_NL("SM_Feedback ",HeartBeat.Feedback);
			GlobalVars_Inc_Thread_Function();
		}
		GlobalVars_Inc_Curr_Thread();
	  //PrintfNL(1);
	}
}






void Disp_Comm_HeartBeat_Send(void){
	//uint8_t  heartbeat_data[14];
	uint16_t crc_temp = 0;

	Disp_Comm_HeartBeat_Set_Fixed_Fields();
	
  heartbeat_data[0]  = HeartBeat.Header;
	heartbeat_data[1]  = HeartBeat.Length;
	heartbeat_data[2]  = HeartBeat.CMD;
	heartbeat_data[3]  = HeartBeat.PacketID; //Temperature>>8;
	heartbeat_data[4]  = HeartBeat.Temperature; // & 0xFF;
	heartbeat_data[5]  = HeartBeat.WaterDispenserFlags;
	heartbeat_data[6]  = HeartBeat.CubeIceDispenserFlags;
	heartbeat_data[7]  = HeartBeat.CrushedIceDispenserFlags;
	heartbeat_data[8]  = HeartBeat.IceMakingFlags;
	heartbeat_data[9]  = HeartBeat.Feedback;
	
	#ifdef DISP_COMM_USE_CRC_CVTE
	crc_temp = CRC_CVTE_Block(heartbeat_data, 10);
	#endif
	#ifdef DISP_COMM_USE_CRC_X_MODEM
	crc_temp = CRC_X_Modem_Block(heartbeat_data, 10);
	#endif
	
	Disp_Comm_HeartBeat_Set_Crc(crc_temp);
	heartbeat_data[10] = HeartBeat.Crc>>8;
	heartbeat_data[11] = HeartBeat.Crc & 0xFF;
  Disp_UART_Tx_Buf(heartbeat_data, 12);
}

/*----------------------Heart beat End--------------------------*/






/*----------------------Master Data Start---------------------*/

void Disp_Comm_Master_Data_Struct_Init(void){
	for(uint16_t i=0;i<DISP_COMM_RAW_BUF_SIZE; i++){
		MasterData.RawBuf[i]=0;
	}
	MasterData.RawBufLen = 0;
	MasterData.Header = 0;
	MasterData.Length = 0;
	MasterData.CMD = 0;
	MasterData.Mode = 0;
	MasterData.WaterDispenser = 0;
	MasterData.IceMaker = 0;
	MasterData.IMValveOnTime = 0;
	MasterData.IMValveOnCycle = 0;
	MasterData.CalculatedCrc = 0;
	MasterData.ReceivedCrc = 0;
	MasterData.CrcCheckOk = 0;
}



uint8_t Disp_Comm_Master_Data_CRC_Verify(void){
	uint8_t sts = 0;
	//UART_Transmit_Text("0");
	if( Disp_UART_Data_Available() ){
		Disp_UART_Clear_Data_Available_Flag();
		//UART_Transmit_Text("1");
		Disp_UART_Data_Copy_Buf( MasterData.RawBuf );
		//UART_Transmit_Text("2");
		uint16_t calculated_crc = CRC_CVTE_Block( MasterData.RawBuf, Disp_UART_Get_Data_Len()-2);
		//UART_Transmit_Text("3");
		MasterData.CalculatedCrc = calculated_crc;
    uint16_t received_crc   = MasterData.RawBuf[Disp_UART_Get_Data_Len()-2];
		received_crc          <<= 8;
		received_crc           |= MasterData.RawBuf[Disp_UART_Get_Data_Len()-1];
		MasterData.ReceivedCrc  =  received_crc;
		if(calculated_crc == received_crc){
			sts = 1;
		}
		MasterData.RawBufLen = Disp_UART_Get_Data_Len();
		Disp_UART_Flush_Buf();
		//UART_Transmit_Text("DataFlushed");
	}
	return sts;
}


uint8_t Disp_Comm_Master_Data_Extract(void){
	uint8_t sts = 0;
	if(Disp_Comm_Master_Data_CRC_Verify()){
		//UART_Transmit_Text("5");
	  MasterData.Header         = MasterData.RawBuf[0];
		MasterData.Length         = MasterData.RawBuf[1];
		MasterData.CMD            = MasterData.RawBuf[2];
		MasterData.Mode           = MasterData.RawBuf[3];
		MasterData.WaterDispenser = MasterData.RawBuf[4];
		MasterData.IceMaker       = MasterData.RawBuf[5]; 
		MasterData.IMValveOnTime  = MasterData.RawBuf[6];
		MasterData.IMValveOnCycle = MasterData.RawBuf[7]; 
		for(uint16_t i=0; i<MasterData.RawBufLen; i++){
		  MasterData.RawBuf[i]=0;
	  }
		//UART_Transmit_Text("6");
		MasterData.RawBufLen = 0;
    sts = 1;
		MasterData.CrcCheckOk = 1;
	}
	else{
		MasterData.CrcCheckOk = 0;
	}
	return sts;
}

uint8_t Disp_Comm_Master_Data_CRC_Stat(void){
	uint8_t sts = 0;
	if((Disp_Comm_Master_Data_Get_Calculated_Crc() == Disp_Comm_Master_Data_Get_Received_Crc()) && (Disp_Comm_Master_Data_Get_Calculated_Crc() != 0)){
		sts = 1;
	}
	return sts;
}



void Disp_Comm_Master_Data_Extract_Buffer_Reset(uint8_t Byte_pos, uint8_t Bit_pos){
	if(Byte_pos == 4){
		MasterData.WaterDispenser &=~ (1<<Bit_pos);
	}
	
	else if(Byte_pos == 5){
		MasterData.IceMaker &=~ (1<<Bit_pos);
	}
	
	else if(Byte_pos == 6){
		MasterData.IMValveOnTime &=~ (1<<Bit_pos);
	}
	
	else if(Byte_pos == 7){
		MasterData.IMValveOnCycle &=~ (1<<Bit_pos);
	}
}




uint8_t Disp_Comm_Master_Data_Get_RawBuf(uint8_t index){
	return MasterData.RawBuf[index];
}

uint8_t Disp_Comm_Master_Data_Get_RawBufLen(void){
	return MasterData.RawBufLen;
}

uint8_t Disp_Comm_Master_Data_Get_Header(void){
  return MasterData.Header;
}

uint8_t Disp_Comm_Master_Data_Get_Length(void){
  return MasterData.Length;
}

uint8_t Disp_Comm_Master_Data_Get_Mode(void){
  return MasterData.Mode;
}

uint8_t Disp_Comm_Master_Data_Get_WaterDispenser(void){
  return MasterData.WaterDispenser;
}

uint8_t Disp_Comm_Master_Data_Get_IceMaker(void){
  return MasterData.IceMaker;
}

uint8_t Disp_Comm_Master_Data_Get_IMValveOnTime(void){
  return MasterData.IMValveOnTime;
}

uint8_t Disp_Comm_Master_Data_Get_IMValveOnCycle(void){
  return MasterData.IMValveOnCycle;
}

uint16_t Disp_Comm_Master_Data_Get_Calculated_Crc(void){
  return MasterData.CalculatedCrc;
}

uint16_t Disp_Comm_Master_Data_Get_Received_Crc(void){
  return MasterData.ReceivedCrc;
}

uint8_t Disp_Comm_Master_Data_Get_Crc_Check_Status(void){
	return MasterData.CrcCheckOk;
}

/*
void Disp_Comm_Print_Master_Rec_Data(void){
	PrintfD_BIN("M>S Header:",Disp_Comm_Master_Data_Get_Header());
	UART_Transmit_Text("  ");
	PrintfD_BIN("Length:",Disp_Comm_Master_Data_Get_Length());
	UART_Transmit_Text("  ");
	PrintfD_BIN("Mode:",Disp_Comm_Master_Data_Get_Mode());
	UART_Transmit_Text("  ");
	PrintfD_BIN("WaterDispenser:",Disp_Comm_Master_Data_Get_WaterDispenser());
	UART_Transmit_Text("  ");
	PrintfD_BIN("IceTypeSelect:",Disp_Comm_Master_Data_Get_IceTypeSelect());
	UART_Transmit_Text("  ");
	PrintfD_BIN("SelfDiagnosis:",Disp_Comm_Master_Data_Get_SelfDiagnosis());
	UART_Transmit_Text("  ");
	PrintfD_BIN("SystemReset:",Disp_Comm_Master_Data_Get_SystemReset());
	UART_Transmit_Text("  ");
	PrintfD_BIN("Calculated_Crc:",Disp_Comm_Master_Data_Get_Calculated_Crc());
	UART_Transmit_Text("  ");
	PrintfD_BIN("Received_Crc:",Disp_Comm_Master_Data_Get_Received_Crc());
	UART_Transmit_Text("  ");
	PrintfNL(1);
}

*/


void Disp_Comm_Print_Master_Rec_Data(void){
	
	if(GlobalVars_Get_Start_Flag() && (GlobalVars_Get_Thread_Function() == 3)){
	  if(GlobalVars_Get_Curr_Thread() == 33){
			UART_PrintfD_NL("MS_Header ",Disp_Comm_Master_Data_Get_Header());
		}
		else if(GlobalVars_Get_Curr_Thread() == 34){
			UART_PrintfD_NL("MS_Length ",Disp_Comm_Master_Data_Get_Length());
		}
		else if(GlobalVars_Get_Curr_Thread() == 35){
			UART_PrintfD_NL("MS_Mode ",Disp_Comm_Master_Data_Get_Mode());
		}
		else if(GlobalVars_Get_Curr_Thread() == 36){
			UART_PrintfD_NL("MS_WaterDispenser ",Disp_Comm_Master_Data_Get_WaterDispenser());
		}
		else if(GlobalVars_Get_Curr_Thread() == 37){
			UART_PrintfD_NL("MS_IceMaker ",Disp_Comm_Master_Data_Get_IceMaker());
		}
		else if(GlobalVars_Get_Curr_Thread() == 38){
			UART_PrintfD_NL("MS_IMVOnTime ",Disp_Comm_Master_Data_Get_IMValveOnTime());
		}
		else if(GlobalVars_Get_Curr_Thread() == 39){
			UART_PrintfD_NL("MS_IMVOnCycle ",Disp_Comm_Master_Data_Get_IMValveOnCycle());
		}
		else if(GlobalVars_Get_Curr_Thread() == 40){
			//UART_PrintfD_HEX("MS_Calculated_Crc ",Disp_Comm_Master_Data_Get_Calculated_Crc());
		}
		else if(GlobalVars_Get_Curr_Thread() == 41){
			//UART_PrintfD_HEX("MS_Received_Crc ",Disp_Comm_Master_Data_Get_Received_Crc());
		}
		else if(GlobalVars_Get_Curr_Thread() == 42){
			UART_PrintfD_NL("CRC_Var ",Disp_Comm_Master_Data_CRC_Stat());
			GlobalVars_Inc_Thread_Function();
		}
		GlobalVars_Inc_Curr_Thread();
	}
}

uint8_t Disp_Comm_Master_Data_Get_Bit_Val(uint8_t reg, uint8_t bit_pos){
	if(reg & (1<<bit_pos)){
		return 1;
	}
	else{
		return 0;
	}
}


uint8_t Disp_Comm_Master_Data_Get_Mode_Ice_Maker_Status(void){
	if(Disp_Comm_Master_Data_Get_Bit_Val( Disp_Comm_Master_Data_Get_Mode(), DISP_COMM_MASTER_DATA_MODE_ICE_MAKER_ON_OFF_BP)){
		return 1;
	}
	else{
		return 0;
	}
}


uint8_t Disp_Comm_Master_Data_Get_Mode_Water_Dispenser_Status(void){
	if(Disp_Comm_Master_Data_Get_Bit_Val( Disp_Comm_Master_Data_Get_Mode(), DISP_COMM_MASTER_DATA_MODE_WATER_DISPENSER_ON_OFF_BP)){
		return 1;
	}
	else{
		return 0;
	}
}






/*----------------------Master Data End-----------------------*/









/*-----------------Display Communication Handler--------------*/

void Disp_Comm_Update_HBUpdate_Byte(void){
	//uint8_t HBbyte = 0;
	if((Disp_Comm_Master_Data_Get_Mode() & (1<<5)) == (1<<5)){
		HB_update_packet_id++;
		if(HB_update_packet_id >= 125){
			HB_update_packet_id = 0;
		}
		//HBbyte = ((HB_update_packet_id<<1)|1);
		//Disp_Comm_HeartBeat_Set_PacketID(HBbyte);
		Disp_Comm_HeartBeat_Set_PacketID(((HB_update_packet_id<<1)|1));
	}
	else{
		Disp_Comm_HeartBeat_Set_PacketID(0);
	}
}

void Disp_Comm_Handler(void){
	Disp_Comm_Master_Data_Extract();
	if( Timebase_DownCounter_SS_Expired_Event(DISP_COMM_HEARTBEAT_INTERVAL_TIMER_WINDOW) ){
		Disp_Comm_Update_HBUpdate_Byte();
		Disp_Comm_HeartBeat_Send();
		Timebase_DownCounter_SS_Set_Forcefully(DISP_COMM_HEARTBEAT_INTERVAL_TIMER_WINDOW, DISP_COMM_HEARTBEAT_INTERVAL);
	}
}




/*
int8_t Im_state = 0;
int8_t Disp_Comm_IM_Control(void){
	if(Disp_UART_Data_Available()){
		if( Disp_UART_Get_Buf(1) == 65){
			Im_state = 1;
		}
		else if(Disp_UART_Get_Buf(1) == 66){
			Im_state = 0;
		}
		Disp_UART_Flush_Buf();
	}
	return Im_state ;
}

int8_t Im_State_2 = 0;
int8_t Disp_Comm_IM_Control_2(void){
	if(Disp_Comm_IM_Control() == 1){
		Im_State_2 = 1;
	}
	
	else if(Disp_Comm_IM_Control() == 0){
		Im_State_2 = 0;
	}
	return Im_State_2;
}
*/


void Disp_Comm_Init(void){
	Disp_Comm_Variable_Init();
	Disp_Comm_Master_Data_Struct_Init();
	Disp_Comm_HeartBeat_Struct_Init();
	Timebase_DownCounter_SS_Set_Forcefully(DISP_COMM_HEARTBEAT_INTERVAL_TIMER_WINDOW, DISP_COMM_HEARTBEAT_INTERVAL);
}



//--------------Disp_Comm based ice maker and water dispenser operation------------------//

/*
int8_t im_state = 0;
void Disp_Comm_Ice_Maker_Mode(void){
			if(Disp_Comm_Master_Data_Get_Mode() & (1<<0)){
				im_state = 1;
				//UART_Transmit_Text("ON\r\n");
			}
			
			else if((Ice_Maker_IM_On_Operation() == 0)){
				im_state = 0;
				//UART_Transmit_Text("OFF\r\n");
			}
			
			if(im_state == 1){
				Ice_Maker_Ice_Making_Operation();
			}
}
*/



