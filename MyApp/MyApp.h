
// ---------------------------------------------------------------------------------------------------
#define WaitOnBusy()           while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1) == 1){printf("Chip is Busy \r\n");}
#define WaitOnCounter()        for(uint8_t counter=0; counter < 15; counter++) {   __nop();  }

#define SX1262_CMD_NOP         0x00

//-------------------------------- enums ------------------------------------------------------------
typedef enum{
	
	STDBY_RC                            = 0x00,
	STDBY_XOSC                          = 0x01,
	
}RadioStandbyModes_t;


typedef enum RadioCommands_e{
	
	RADIO_GET_STATUS                 = 0xC0,
	RADIO_SET_STANDBY                = 0x80,
	
	
}RadioCommands_t;

// ------------------------------ Function Prototype -------------------------------------------------
void SX1262_Reset(void);
void MyApp(void);

