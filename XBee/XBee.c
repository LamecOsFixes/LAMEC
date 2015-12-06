#include "XBee.h"

//S_XBee_ATResponse ATResponseStruct;
S_XBee_ATResponse ATResp;
S_XBee_RemoteATResponse		RemoteATResponse;
S_XBee_ZigbeeExplicitRXIndicator zigbeeRx;
	
uint8_t FormatFrame(uint8_t * data, uint16_t size);
//////////////Specific/////////	
/**
  * @brief  Calcula valor percentual de RSSI a partir de 
	*					resposta ao AT command DB
  * @param  frameRssi: valor de rssi recebido na trama.
	* @retval *resultValue: valor percentual de RSSI com casa decimal na forma inteira (46,8% = 468)
  * @return 1 - sucesso ; 0 - erro
	* @todo		!!!!!!FAZER E TESTAR!!!!!!
  */
uint8_t calcRSSI(uint8_t frameRssi, uint16_t *resultValue)
{
	if(frameRssi < 0x1A || frameRssi > 0x5C)
	{
		return 0;
	}
//	uint32_t Result = 0;
//	
//	Result = (41 * (frameRssi)) - 5928;
	
	return 1;
}
////////////////////Receive///////////////////
/**
  * @brief  Processa AT Response
  * @param  frame: trama recebida
	* @param  size: tamanho da trama recebida
  * @return Estrutura da trama recebida
	* @note   trama e size inclui o identificador 0x88(AT Response)
	* @todo		TESTAR
  */
S_XBee_ATResponse * XBee_AtResponse(uint8_t * frame , uint8_t size)
{
		ATResp.FrameId   = frame[0];
		ATResp.ATCommand = frame[1];
		ATResp.ATCommand = (ATResp.ATCommand<<8) | frame[2];
		ATResp.status    = frame[3];
		ATResp.SizeParams= size - 5;
		
		for (int i = 0 ; i < size-5 ; i++){
			ATResp.Params[i] = frame[i+4];
		}
		
		return &ATResp;
}

/**
  * @brief  Processa Remote AT Response
  * @param  frame: trama recebida
	* @param  size: tamanho da trama recebida
  * @return Estrutura da trama recebida
	* @note   Size inclui o identificador 0x97(AT Response)
	* @todo		TESTAR
  */
S_XBee_RemoteATResponse * XBee_RemoteAtResponse(uint8_t * frame , uint8_t size)
{
		RemoteATResponse.FrameId   = frame[0];
		for (int i = 0 ; i < 8 ; i++){
			RemoteATResponse.Addr64[i] = frame[1+i];
		}
		RemoteATResponse.Addr16 = ((frame[9]<<8) | frame[10]);
		RemoteATResponse.ATCommand = ((frame[11]<<8) | frame[12]);
		RemoteATResponse.status = frame[13];
		RemoteATResponse.SizeParams = size - 14;
		
		for (int i = 0 ; i < RemoteATResponse.SizeParams ; i++){
			ATResp.Params[i] = frame[i+14];
		}
		
		return &RemoteATResponse;
}

/**
  * @brief  Processa trama com Zigbee Explicit RX Indicator
  * @param  frame: trama recebida
	* @param  size: tamanho da trama recebida
  * @return Estrutura da trama recebida
	* @note   trama e size inclui o identificador 0x91(Zigbee Explicit RX Indicator)
	* @todo		TESTAR
  */
S_XBee_ZigbeeExplicitRXIndicator * XBee_ZigbeeExpRXInd (uint8_t * frame, uint8_t size)
{	
	for (int i = 0 ; i < 8 ; i++){
		zigbeeRx.SrcAddr64[i] = frame[i];
	}
	zigbeeRx.SrcAddr16 = ((frame[8]<<8)|frame[9]);
	zigbeeRx.SrcEndpoint = frame[10];
	zigbeeRx.DstEndpoint = frame[11];
	zigbeeRx.ClusterID = ((frame[12]<<8)|frame[13]);
	zigbeeRx.ProfileID = ((frame[14]<<8)|frame[15]);
	zigbeeRx.RcvOpt = frame[16];
	for (int i = 0 ; i < size-17 ; i++){
		zigbeeRx.Data[i] = frame[17+i];
	}
	zigbeeRx.SizeData = size-17;
	
	return &zigbeeRx;
}


/////////////////////SEND///////////////////////
/**
  * @brief  Entra em modo AT
  * @return 1 - sucesso ; 0 - erro
	*	@note Not tested
  */
uint8_t XBee_enterAtMode(){
	uint8_t buffer[3];
	
	for (int i = 0 ; i < 3 ; i++){
		buffer[i] = '+';
	}
	Lib_SetUARTOutBufBytes(buffer, 3);
	Lib_UART_Transmit_wRetry_IT(&huart2);
	
	HAL_Delay(1100);
	return 1;
}

/**
  * @brief  Entra em modo AT
  * @return 1 - sucesso ; 0 - erro
  */
uint8_t XBee_exitAtMode(){
	uint8_t buffer[5];
	
	buffer[0] = 'A';
	buffer[1] = 'T';
	buffer[2] = 'C';
	buffer[3] = 'N';
	buffer[4] = '\r';

	Lib_SetUARTOutBufBytes(buffer, 5);
	Lib_UART_Transmit_wRetry_IT(&huart2);

	return 1;
}

/**
  * @brief  Envia comando AT
  * @param  FrameID: valor FrameID
	* @param  ATCommand: comando AT
	* @param  param: buffer com parametros do comando AT
	* @param  sizeParams: tamanho ocupado pelos parametros
  * @return 1 - sucesso ; 0 - erro
	*	@attencion	!!!!!NAO FUNCIONA!!!!!!
  */
uint8_t XBee_AtCommand(uint8_t FrameID, uint16_t ATCommand, uint8_t * param , uint8_t sizeParams){
	uint8_t buffer[128];
	
	XBee_enterAtMode();
	
	uint16_t totalSize = sizeParams + 4; //tamanho de parametros util mais 3 (0x08 , FrameID , ATCommand)
	
	buffer[0] = 0x08;
	buffer[1] = FrameID;
	buffer[2] = (ATCommand >> 8) & 0xFF;
	buffer[3] = (ATCommand & 0xFF);
	
	for (int i = 0 ; i < sizeParams ; i++){
		buffer[4+i] = param[i];
	}
	
	FormatFrame(buffer, totalSize);
	XBee_exitAtMode();
	return 1;
}

/**
  * @brief  Envia comando AT remotamente
  * @param  FrameID: valor FrameID
	* @param  ATCommand: comando AT
	* @param  param: buffer com parametros do comando AT
	* @param  sizeParams: tamanho ocupado pelos parametros
  * @return 1 - sucesso ; 0 - erro
	*	@attencion	Not tested 
  */
uint8_t XBee_RemoteAtCommand(uint8_t FrameID, uint8_t * Addr64, uint16_t Addr16, uint8_t RemoteCmdOptions ,uint16_t ATCommand, uint8_t * param , uint8_t sizeParams){
	uint8_t buffer[128];
	
	XBee_enterAtMode();
	
	uint16_t totalSize = sizeParams + 14; 
	
	buffer[0] = RemoteATComandRequest;
	buffer[1] = FrameID;
	for (int i = 0 ; i < 8 ; i++){
		buffer[2+i] = Addr64[i];
	}
	buffer[10] = (Addr16 >> 8) & 0xFF;
	buffer[11] = (Addr16 & 0xFF);
	buffer[12] = RemoteCmdOptions;
	buffer[13] = (ATCommand >> 8) & 0xFF;
	buffer[14] = (ATCommand & 0xFF);
	for (int i = 0 ; i < sizeParams ; i++){
		buffer[15+i] = param[i];
	}
	
	FormatFrame(buffer, totalSize);
	XBee_exitAtMode();
	return 1;
}
/**
  * @brief  Envia ZigBee Transmit Request
  * @param  FrameID: valor FrameID
	* @param  Dest64: endereço 64bits de destino
	* @param  Dest16: endereço 16bits de destino
	* @param  BC_radius: numero maximo de saltos quando se envia broadcast
	* @param  Opt: Options
	* @param  RFData: buffer com informaçao a enviar
	* @param  sizeRFData : tamanho da informaçao a enviar
  * @return 1 - sucesso ; 0 - erro
	* @todo		TESTAR
  */
uint8_t XBee_ZigBeeTransmitRequest(uint8_t FrameID, uint8_t * Dest64, uint8_t * Dest16 , uint8_t BC_radius , uint8_t Opt , uint8_t * RFData, uint8_t sizeRFData){
	uint8_t buffer[128];
	
	uint16_t totalSize = sizeRFData + 13; //tamanho de parametros util mais 3 (0x08 , FrameID , ATCommand)
	
	buffer[0] = 0x10;
	buffer[1] = FrameID;
	for( int i=0 ; i<8 ; i++ ){
		buffer[2+i] = Dest64[i];
	}
	buffer[10]= Dest16[0];
	buffer[11]= Dest16[1];
	
	buffer[12]= BC_radius;
	
	buffer[13]= Opt;
	
	for (int i = 0 ; i < sizeRFData ; i++){
		buffer[14+i] = RFData[i];
	}
	
	FormatFrame(buffer, totalSize);
	return 1;
}

/////////////////Base de envio/////////////////////
/**
  * @brief  Encapsula uma trama e envia
  * @param  data: buffer com a trama util
	* @param  size: tamanho da trama util
  * @return 1 - sucesso ; 0 - erro
  */
uint8_t FormatFrame(uint8_t * data, uint16_t size){
	uint8_t buffer[128];
	uint16_t totalSize = size + 4; //tamanho da trama util mais 4 de encapsulamento
	uint16_t checksum = 0 ;
	
	buffer[0] = 0x7E;
	buffer[1] = (size>>8) & 0xFF ;
	buffer[2] = size & 0xFF ;
	
	for (int i = 0 ; i < size ; i++){
		buffer[3+i] = data[i];
		checksum = (checksum + data[i])&0xFF;
	}
	checksum = (0xFF - checksum);
	
	buffer [3+size] = checksum & 0xFF;
	
	Lib_SetUARTOutBufBytes(buffer, totalSize);
	Lib_UART_Transmit_wRetry_IT(&huart2);
	return 1;
}

