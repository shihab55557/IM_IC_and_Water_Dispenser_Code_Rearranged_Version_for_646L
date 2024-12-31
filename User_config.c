#include "stm32g070xx.h"
#include "User_config.h"
#include "disp_comm.h"
#include "Icemaker.h"
#include "Uart.h"

uint8_t User_config_im_reset_flag = 0;
uint8_t Disp_conf_im_on_disp_conf_rotation = 0;    // Will be used in User_Config_Disp_Ice_Maker_Ice_Tray_Rotation() function 

uint8_t Disp_Config_Ice_Tray_on_User_Config_Rotation(void){
	return Disp_conf_im_on_disp_conf_rotation;
}

void User_Config_Disp_Ice_Maker_Ice_Tray_Rotation(void){
	if((Disp_Comm_Master_Data_Get_Mode() & (1<<4)) == (1<<4)){
		if((Disp_Comm_Master_Data_Get_IceMaker() & (1<<6)) == (1<<6)){
			if((!Ice_Maker_Ice_Tray_On_Operation()) && (!User_Config_Valve_on_User_Config())){
				Disp_Comm_Heartbeat_Set_Data(9,1,7);
				if(User_config_im_reset_flag == 0){
					Ice_Maker_Reset_All_Parameter();
					User_config_im_reset_flag = 1;
				}
				//Ice_Maker_Config_Rotation();
				Disp_conf_im_on_disp_conf_rotation = 1;
				//UART_Transmit_Text("Conf rotation");
			}
		}
	}
	
	if(Disp_conf_im_on_disp_conf_rotation == 1){
		//UART_Transmit_Text("Conf rotation");
		Ice_Maker_Config_Rotation();            // Making Ice_Maker_Ice_Tray_On_Config_Rotation() = 0 at the end of rotation
	}
	
	if(!Ice_Maker_Ice_Tray_On_Config_Rotation()){
		Disp_conf_im_on_disp_conf_rotation = 0;           // Making the flag Disp_conf_im_on_rotation = 0 at the end of the rotation to stop the rotation by Disp Config
		
	}
	
	if(Disp_conf_im_on_disp_conf_rotation == 0){
		Disp_Comm_Heartbeat_Set_Data(9,0,7);
		//UART_Transmit_Text("!Conf rotation");
		User_config_im_reset_flag = 0;
	}
}

void User_Config_Send_IM_Motor_Feedback(void){
	//if(Ice_Maker_Ice_Tray_On_Operation() || (Ice_Maker_Valve_On_Operation()) || Ice_Maker_Ice_Tray_On_Config_Rotation() || (User_Config_Valve_on_User_Config())){
	//if(Ice_Maker_Ice_Tray_On_Operation() || (Ice_Maker_Valve_On_Operation()) || Ice_Maker_Ice_Tray_On_Config_Rotation()){
	if(Ice_Maker_Ice_Tray_On_Operation() || Ice_Maker_Ice_Tray_On_Config_Rotation()){

		Disp_Comm_Heartbeat_Set_Data(8,1,7);
		//UART_Transmit_Text("On rotation");
	}
	else{
		Disp_Comm_Heartbeat_Set_Data(8,0,7);
		//UART_Transmit_Text("Not on rotation");
	}
}


uint8_t Valve_on_user_config = 0;    // Valve is being used by user for configuration
uint8_t UC_valve_turned_off = 0;
void User_Config_Disp_Ice_Maker_Valve_On_Off(void){
	//if((!Ice_Maker_Valve_On_Operation()) && (!Ice_Maker_Ice_Tray_On_Operation() && (!Ice_Maker_Ice_Tray_On_Config_Rotation()))){
	if((!Ice_Maker_Ice_Tray_On_Operation() && (!Ice_Maker_Ice_Tray_On_Config_Rotation()))){
		if((Disp_Comm_Master_Data_Get_Mode() & (1<<4)) == (1<<4)){
			if((Disp_Comm_Master_Data_Get_IceMaker() & (1<<4)) == (1<<4)){
				UC_valve_turned_off = 1;
				Ice_Maker_Valve_On();
				Valve_on_user_config = 1;
				Disp_Comm_Heartbeat_Set_Data(9,1,6);
			}
			else{
				Ice_Maker_Valve_Off();
				Valve_on_user_config = 0;
				Disp_Comm_Heartbeat_Set_Data(9,0,6);
			}
		}
		else{
			if(UC_valve_turned_off != 0){
				Ice_Maker_Valve_Off();
				Ice_Maker_Valve_Off();
				UC_valve_turned_off = 0;
			}
			Valve_on_user_config = 0;
			Disp_Comm_Heartbeat_Set_Data(9,0,6);
		}
	}
}


uint8_t User_Config_Valve_on_User_Config(void){
	return Valve_on_user_config;
}

uint8_t User_Config_User_in_Configuration_Window(void){
	uint8_t Conf_window = 0;
	if((Disp_Comm_Master_Data_Get_Mode() & (1<<4)) == (1<<4)){
		Conf_window = 1;
	}
	else{
		Conf_window = 0;
	}
	return Conf_window;
}

void User_Config_Configure_Window_Feedback(void){
	if((Disp_Comm_Master_Data_Get_Mode() & (1<<4)) == (1<<4)){
		Disp_Comm_Heartbeat_Set_Data(9,1,5);
	}
	else{
		Disp_Comm_Heartbeat_Set_Data(9,0,5);
	}
}

uint8_t User_Config_User_In_Configuration_Mode(void){
	uint8_t Configuration_mode = 0;
	if((Disp_Config_Ice_Tray_on_User_Config_Rotation()) || (User_Config_Valve_on_User_Config()) || (User_Config_User_in_Configuration_Window())){
		Configuration_mode = 1;
	}
	else{
		Configuration_mode = 0;
	}
	return Configuration_mode;
}

void User_Config_Ice_Maker_Handler(void){
	//User_Config_Disp_Configure_Window();
	//User_Config_Ice_Maker_Test_Button_Rotation();
	User_Config_Configure_Window_Feedback();
	User_Config_Send_IM_Motor_Feedback();
	User_Config_Disp_Ice_Maker_Ice_Tray_Rotation();
	User_Config_Disp_Ice_Maker_Valve_On_Off();
}


