#include "protocolXbee.h"
#include "XBee.h"


uint32_t MyGlobalTime;
typedef struct
{
  uint8_t 		inbufCommandState;
  uint16_t		inbufLength;			//guarda tamanho da trama
	uint16_t		inbufCommandNumbytesLeft;	//controla numero d bytes que falta ler para completara a trama
	uint8_t			checksumVerify;
  uint8_t 		inbufCommandIndex;		//apontador de posiçao do buffer
  uint8_t 		inbufCommandBuf[256];
} S_Protocol_Machine;

uint8_t Analyse_Frame(S_Protocol_Machine *Protocol_Motor);

S_Protocol_Machine Protocol_Machine;

void ResetStateMachine(S_Protocol_Machine * Protocol_Machine){
	Protocol_Machine->checksumVerify=0;
	Protocol_Machine->inbufCommandIndex=0;
	Protocol_Machine->inbufCommandIndex=0;
	Protocol_Machine->inbufCommandNumbytesLeft=0;
	Protocol_Machine->inbufCommandState=0;
}

uint8_t Process_Frame(S_Protocol_Machine *Protocol_Machine, uint8_t * tempbyte)
{
	uint8_t ok =0;
	switch(Protocol_Machine->inbufCommandState)
	{
	case 0:  
				if (*tempbyte == 0x7E)
				{
					MyGlobalTime = HAL_GetTick();
					Protocol_Machine->inbufCommandState++;        // header detect
				}  
				MyGlobalTime = HAL_GetTick();
				break;
				 
	case 2:																																	
				Protocol_Machine->inbufCommandIndex = 0;
				Protocol_Machine->inbufLength = ((*tempbyte)<<8)&0xFFFF;
				Protocol_Machine->inbufCommandState++;
				break;            

	case 3:																							
				Protocol_Machine->inbufLength |= (*tempbyte & 0xFF);// Size MSB+LSB
 
				if(Protocol_Machine->inbufLength > 256)
					{
				//  resetCommandStateMachine(Protocol_Motor);
				//  break;
					}
				Protocol_Machine->inbufCommandNumbytesLeft = Protocol_Machine->inbufLength;
				Protocol_Machine->inbufCommandState++;
				break;
				 
	case 4:																							
				Protocol_Machine->inbufCommandBuf[Protocol_Machine->inbufCommandIndex] = *tempbyte;
				Protocol_Machine->inbufCommandNumbytesLeft--;
				Protocol_Machine->checksumVerify = (Protocol_Machine->checksumVerify + *tempbyte)&0xFF; 
	
				if (Protocol_Machine->inbufCommandNumbytesLeft == 0){
					Protocol_Machine->inbufCommandState++;
				} else {
					Protocol_Machine->inbufCommandIndex++;
				}
				break;
				 
	case 5:
		
				if (Protocol_Machine->checksumVerify + *tempbyte == 0xFF) {
					ok = 1;
				}else {
					ResetStateMachine(Protocol_Machine);
				}
				break;
		 
	default:
				ResetStateMachine(Protocol_Machine); 
				break;
	}
  return ok;
}



void Run_Frame(void)
{
  uint8_t tempbyte;
  
  if(HAL_GetTick() > MyGlobalTime + 500)
  {
    ResetStateMachine(&Protocol_Machine);
    MyGlobalTime = HAL_GetTick();
  }
  
  if (Lib_GetUARTInBufByte(&tempbyte) != 0)
  {
		if(Process_Frame(&Protocol_Machine, &tempbyte)){			
      Analyse_Frame(&Protocol_Machine);
		}
  }		
}

uint8_t Analyse_Frame(S_Protocol_Machine *Protocol_Motor)
{
//  uint8_t bufferRead[128]; //512
  uint8_t TypeOper;
  
  TypeOper = Protocol_Motor->inbufCommandBuf[0];
  
  
  switch(TypeOper)
  {
  case AT_RESPONSE:
     XBee_AtResponse(&(Protocol_Motor->inbufCommandBuf[1]) , Protocol_Motor->inbufLength);
		 
     break;
    
  
  }
  return 0;
}
