#include "stm32g070xx.h"
#include "Uart.h"
#include "timer.h"
#include "Icemaker.h"
#include "disp.h"
#include "disp_comm.h"
#include "timebase.h"
#include "water_dispenser.h"
#include "debug_parameter.h"
#include "glvars.h"

//Disp_UART_Print_Buf_Data();
//Disp_UART_Flush_Buf();




int main(void) {
	

	  
	  Timebase_Init(1000);
    UART_Init(38400);    // Initialize UART with desired baud rate
	  Disp_UART_Init(9600);
		timer_Init(0xFFF, 0xF44);
		Ice_Maker_Init();
	  Disp_Comm_Init();
		Water_Dispenser_Init();
	  
	  
	  Timebase_DownCounter_SS_Set_Forcefully(2, 1000);
	
    while (1) {
			
			Disp_Comm_Handler();
			Ice_Maker_Disp_Comm_Based_Ice_Maker_Handler();
			Water_Dispenser_Handler();
			if( Timebase_DownCounter_SS_Expired_Event(2) ){
				Timebase_DownCounter_SS_Set_Forcefully(2,1000);
				UART_Transmit_Text("UpTime ");
				UART_Transmit_Number((Timebase_Timer_Get_Seconds()));
				UART_Transmit_Text("\r\n");
				GlobalVars_Set_Start_Flag();
			}
			
			Print_All_Debug_Data();
			//UART_Transmit_Number(Timebase_Window_Timer_Get_Interval_Reset());
			//UART_Transmit_Text("\r\n");
			Timebase_Window_Timer_Start();
			Timebase_Main_Loop_Executables();
    }
		
}


