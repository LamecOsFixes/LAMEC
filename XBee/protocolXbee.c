#include "protocolXbee.h"



uint32_t	MyGlobalTime=0;
uint8_t		RssiValue=0;

typedef struct
{
  uint8_t 		inbufCommandState;
  uint16_t		inbufLength;			//guarda tamanho da trama
	uint16_t		inbufCommandNumbytesLeft;	//controla numero d bytes que falta ler para completara a trama
	uint8_t			checksumVerify;
  uint8_t 		inbufCommandIndex;		//apontador de posiçao do buffer
  uint8_t 		inbufCommandBuf[256];
} S_Protocol_Machine;

void Analyse_Frame(S_Protocol_Machine *Protocol_Motor);
uint8_t Process_Frame(S_Protocol_Machine *Protocol_Machine, uint8_t *tempbyte);

S_Protocol_Machine Protocol_Machine;


S_XBee_ATResponse * xbeeAtResp;
S_XBee_ZigbeeExplicitRXIndicator * zigBeeRx;
S_XBee_RemoteATResponse * xbeeRemoteAtResp;

/**
  * @brief  Reinicia estrutura com valores default
  * @param  *Protocol_Machine: Estrutura para reiniciar.
  * @return None
	* @todo		TESTAR
  */
void ResetStateMachine(S_Protocol_Machine * Protocol_Machine){
		Protocol_Machine->checksumVerify=0;
		Protocol_Machine->inbufCommandIndex=0;
		Protocol_Machine->inbufCommandIndex=0;
		Protocol_Machine->inbufCommandNumbytesLeft=0;
		Protocol_Machine->inbufCommandState=0;
}

/**
  * @brief  Verifica se chegou trama valida, desencapsula e faz tratamento.
  * @return None
  */
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
			if(Process_Frame(&Protocol_Machine, &tempbyte)){	//verifica se foi encontrada trama válida
				Analyse_Frame(&Protocol_Machine);	//tratamento de trama válida
			}
  }		
}

/**
  * @brief  Maquina de estados para desencapsulamento da trama
  * @param  *Protocol_Machine: Estrutura para preencher com a trama e outras informacoes.
	* @param  *tempbyte: novo byte recebido da trama
  * @return 1 - sucesso ; 0 - erro
	* @todo		TESTAR
  */
uint8_t Process_Frame(S_Protocol_Machine *Protocol_Machine, uint8_t *tempbyte)
{
	uint8_t ok =0;
	switch(Protocol_Machine->inbufCommandState)
	{
	case 0:  
				MyGlobalTime = HAL_GetTick();
				if (*tempbyte == 0x7E )
				{
					Protocol_Machine->inbufCommandState++;        // header detect from API Frame
				}else if (*tempbyte == 0x4F )
				{
					Protocol_Machine->inbufCommandState=5;        // header detect from AT command mode
				}
				
				break;
				 
	case 1:																																	
				Protocol_Machine->inbufCommandIndex = 0;
				Protocol_Machine->inbufLength = ((*tempbyte)<<8)&0xFFFF;
				Protocol_Machine->inbufCommandState++;
				break;            

	case 2:																							
				Protocol_Machine->inbufLength |= (*tempbyte & 0xFF);// Size MSB+LSB
 
				if(Protocol_Machine->inbufLength > 256)
					{
//						ResetStateMachine(Protocol_Machine);
//						break;
					}
				Protocol_Machine->inbufCommandNumbytesLeft = Protocol_Machine->inbufLength;
				Protocol_Machine->inbufCommandState++;
				break;
				 
	case 3:																							
				Protocol_Machine->inbufCommandBuf[Protocol_Machine->inbufCommandIndex] = *tempbyte;
				Protocol_Machine->inbufCommandNumbytesLeft--;
				Protocol_Machine->checksumVerify = (Protocol_Machine->checksumVerify + *tempbyte)&0xFF; 
	
				if (Protocol_Machine->inbufCommandNumbytesLeft == 0){
					Protocol_Machine->inbufCommandState++;
				} else {
					Protocol_Machine->inbufCommandIndex++;
				}
				break;
				 
	case 4:
		
				if (Protocol_Machine->checksumVerify + *tempbyte == 0xFF) {
					ok = 1;
				}else {
					ResetStateMachine(Protocol_Machine);
				}
				break;
				
	case 5:
				if (*tempbyte == 0x4B) {
					Protocol_Machine->inbufCommandState++;
				}else {
					ResetStateMachine(Protocol_Machine);
				}
				break;
	
	case 6:
			 if (*tempbyte == 0x0D) {
					ok = 2;
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





/**
  * @brief  Faz o tratamento da trama recebida
  * @param  *Protocol_Motor: Estrutura preenchida com trama válida
  * @return 1 - sucesso ; 0 - erro
	* @todo		TESTAR
  */
void Analyse_Frame(S_Protocol_Machine *Protocol_Motor)
{
//  uint8_t bufferRead[128]; //512
	uint8_t bufferParams[64];
	uint8_t bufferMAC[8];
  uint8_t TypeOper;
	
  TypeOper = Protocol_Motor->inbufCommandBuf[0];
  
  //XBee_AtCommand(0x01,Cmd_DB,bufferParams,0);
  switch(TypeOper)
  {
		case RemoteATComandResponse:
			xbeeRemoteAtResp = XBee_RemoteAtResponse(&(Protocol_Motor->inbufCommandBuf[1]), Protocol_Motor->inbufLength);
			if(xbeeRemoteAtResp->status == 0x00){	//0x00 - OK
				if(xbeeRemoteAtResp->ATCommand == Cmd_DB){
						for(int i=0; i<8 ; i++) //write source MAC of remote device
						{
							bufferParams[i] = zigBeeRx->SrcAddr64[i];
						}
						bufferParams[8] = xbeeRemoteAtResp->Params[0];	//write RSSI value
					 for(int i=0;i<8;i++){
						bufferMAC[i]=0;
					 }
					 XBee_ZigBeeTransmitRequest(0x01,bufferMAC,0x0000,0x00,0x00,bufferParams,10);

				}
			}
			break;
		
//  case AT_RESPONSE:
//     xbeeAtResp = XBee_AtResponse(&(Protocol_Motor->inbufCommandBuf[1]) , Protocol_Motor->inbufLength);
//		 if(xbeeAtResp->ATCommand==Cmd_DB){
//				 for(int i=0; i<8 ; i++) //write source MAC of remote device
//				 {
//					bufferParams[i] = zigBeeRx->SrcAddr64[i];
//				 }
//				 bufferParams[8] = xbeeAtResp->Params[0];	//write RSSI value
//				 for(int i=0;i<8;i++){
//					bufferMAC[i]=0;
//				 }
//				 XBee_ZigBeeTransmitRequest(0x01,bufferMAC,0x0000,0x00,0x00,bufferParams,10);
//		 }
//     break;

	case ZigbeeExplicitRXIndicator:
		//XBee_AtCommand(0x01,Cmd_DB,bufferParams,0);
		
		//zigBeeRx = XBee_ZigbeeExpRXInd(&(Protocol_Motor->inbufCommandBuf[1]), Protocol_Motor->inbufLength);
		//XBee_AtCommand(0x01,Cmd_DB,bufferParams,0);
	break;
	
  default:
		break;
  }
	
//	if(TypeOper==ZigbeeExplicitRXIndicator)
//	{
//		XBee_AtCommand(0x01,Cmd_DB,bufferParams,0);
//	}
}
