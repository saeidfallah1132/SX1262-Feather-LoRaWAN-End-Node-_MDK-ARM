#include "main.h"
#include "MyApp.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


//------------------------------ Pinout Information ---------------------------------
// DEBUG (USART1) ------------------------> PA9,PA10
/* SX1262(SPI1) --------------------------> MISO:PA5, MOSI:PA7, SCK:PA5
									
								-NRESET_SX1262 -> PA4 --> OUTPUT
								-NSS_SX1262    -> PA0 --> OUTPUT
								-ANT_SW_SX1262 -> PC4 --> OUTPUT
								-BUSY_SX1262   -> PA1 --> INPUT
								-DIO1_SX1262   -> PC2 --> INTERRUPT
								
								-DIO2_SX1262   -> PC3  --> N/A
								-DIO3_SX1262   -> PA11 --> N/A
								
							
-----------------------------------------------------------------------------------------------------------*/

#define ADV_DEBUG
extern UART_HandleTypeDef huart1;
extern SPI_HandleTypeDef hspi1;
HAL_StatusTypeDef Status;

//----------------------------- Assign printf function to UART1 --------------------
#ifdef __GNUC__

#define PUTCHAR_PROTOTYPE int __io__putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

#endif

PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch,1,0XFFFF);
	return 0;
}

//------------------------------------ SX1262_reset function (Review Done) -------------------------------------------

void SX1262_Reset(void){
	
	#ifdef ADV_DEBUG 
	printf("----------------------- Reset Chip ---------------------------- \r\n");
	#endif
	
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET); // PA4 --> 0
	HAL_Delay(100); // wait 100 ms
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);  // PA4 --> 1
	HAL_Delay(100); // wait 100 ms
	
	#ifdef ADV_DEBUG
	printf(" LoRa Chipset Reset Done \r\n");
	#endif
	
}
//---------------------------------- SX1262_WriteCommand function ---------------------------
void SX1262_WriteCommand(RadioCommands_t command, uint8_t *buffer, uint16_t size){
	
		#ifdef ADV_DEBUG
			printf("cmd: 0x%02x", command);
			
			for(uint8_t i=0; i<size; i++){
					printf("-%02x",buffer[i]);
			}
			printf("\r\n");
		#endif 
			
		WaitOnBusy();
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);   // NSS=0 enable LoRa chipset
			
		HAL_SPI_Transmit(&hspi1,(uint8_t*)&command,1,1000);
		while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
			
		for(uint16_t i=0; i<size; i++){
			
				HAL_SPI_Transmit(&hspi1,(uint8_t*)&buffer[i],1,1000);
				while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
		}
		
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);   // NSS=1 disable LoRa chipset
		
		WaitOnCounter();

	
	
}

//---------------------------------- SX1262_ReadCommand function ---------------------------
uint8_t  SX1262_ReadCommand(RadioCommands_t command, uint8_t *buffer, uint16_t size){
	
			uint8_t status = 0x00;
			uint8_t nop = SX1262_CMD_NOP;
			
			WaitOnBusy();
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);   // NSS=0 enable LoRa chipset
			
			HAL_SPI_Transmit(&hspi1,(uint8_t*)&command,1,1000);
			while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
			
			#ifdef ADV_DEBUG
					printf("command: -%02x \r\n", command);
			#endif 
			
			HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&nop, (uint8_t*)&status,1,1000); // Reading LoRa Chipset status
			while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
			
			for(uint16_t i=0; i<size; i++){
				
					HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&nop, &buffer[i],1,1000); 
					while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
				
			}
			
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);   // NSS=1 disable LoRa chipset
			
			#ifdef ADV_DEBUG
					printf("Status: -%02x \r\n", status);
			#endif 
			
			return status;
			
}

//--------------------------------- SX1262_Standby function --------------------------------
void SX1262_Standby(RadioStandbyModes_t standbyConfig){
			
	    #ifdef ADV_DEBUG
					printf("--- Standby function ---\r\n");
			#endif 
			
			SX1262_WriteCommand(RADIO_SET_STANDBY, (uint8_t*)&standbyConfig, 1);
	
			if(standbyConfig == STDBY_RC){
				
					#ifdef ADV_DEBUG
						printf("Operation Mode= MODE_STDBY_RC \r\n");
					#endif			
			}
			else {
				  #ifdef ADV_DEBUG
						printf("Operation Mode= MODE_STDBY_XOSC \r\n");
					#endif			
			}
	
}

//--------------------------------- SX1262_GetStatus function ------------------------------
uint8_t SX1262_GetStatus(void){
	
		uint8_t status;
	
		status = SX1262_ReadCommand(RADIO_GET_STATUS, NULL, 0);
	
	  #ifdef ADV_DEBUG
			printf("---- GetStatus function ----- \r\n");
  		printf("Status: -%02x \r\n", status);

	  #endif	

	  return status;

}

//--------------------------------- Initialize LoRa Chipset ---------------------------------
void Init_LoRa(void){
	
		SX1262_Reset();
	  SX1262_Standby(STDBY_XOSC);
	  SX1262_GetStatus();

	
}
//--------------------------------- Initialize GPIO -----------------------------------------
void Init_GPIO (void){
	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   // NRESET_SX1262 -> PA4=1
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);   // NSS_SX1262    -> PA0=1
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET); // ANT_SW_SX1262 -> PC4=0
}
//--------------------------------- Application function ------------------------------------

void MyApp(void){
	

	
	#ifdef ADV_DEBUG
	printf("---------------------------- START PROGRAM ----------------------- \r\n");
	#endif
  
	Init_GPIO();
	Init_LoRa();
  	
		
	
	
}

