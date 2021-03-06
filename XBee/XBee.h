#include "stm32f4xx_hal.h"
#include "UART_Handler.h"

extern UART_HandleTypeDef huart2;

#define RemoteATComandRequest				0x17

#define Cmd_DB	0x4442

typedef struct
{
  uint8_t FrameId;
	uint8_t	Addr64[8];
	uint16_t	Addr16;
  uint16_t ATCommand;
	uint8_t status;	//0x00 - OK
  uint8_t Params[100];
	uint8_t SizeParams;
} S_XBee_RemoteATResponse;

typedef struct
{
  uint8_t FrameId;
  uint16_t ATCommand;
	uint8_t status;
  uint8_t Params[100];
	uint8_t SizeParams;
} S_XBee_ATResponse;

typedef struct
{
  uint8_t		SrcAddr64[8];
	uint16_t	SrcAddr16;
	uint8_t		SrcEndpoint;
	uint8_t		DstEndpoint;
	uint16_t	ClusterID;
	uint16_t	ProfileID;
  uint8_t 	RcvOpt;
  uint8_t 	Data[100];
	uint8_t 	SizeData;
} S_XBee_ZigbeeExplicitRXIndicator;


S_XBee_ATResponse * XBee_AtResponse(uint8_t * frame , uint8_t size);

uint8_t XBee_RemoteAtCommand(uint8_t FrameID, uint8_t * Addr64, uint16_t Addr16, uint8_t RemoteCmdOptions ,uint16_t ATCommand, uint8_t * param , uint8_t sizeParams);

S_XBee_RemoteATResponse * XBee_RemoteAtResponse(uint8_t * frame , uint8_t size);

S_XBee_ZigbeeExplicitRXIndicator * XBee_ZigbeeExpRXInd (uint8_t * frame, uint8_t size);

uint8_t XBee_AtCommand(uint8_t FrameID, uint16_t ATCommand, uint8_t * param , uint8_t sizeParams);

uint8_t XBee_ZigBeeTransmitRequest(uint8_t FrameID, uint8_t * Dest64, uint8_t * Dest16 , uint8_t BC_radius , uint8_t Opt , uint8_t * RFData, uint8_t sizeRFData);

