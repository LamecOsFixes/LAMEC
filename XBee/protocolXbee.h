#include "stm32f4xx_hal.h"
#include "XBee.h"


#define RemoteATComandResponse			0x97
#define AT_RESPONSE 								0x88
#define ZigbeeExplicitRXIndicator 	0x91



void Run_Frame(void);

