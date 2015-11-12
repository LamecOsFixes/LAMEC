#include "XBee.h"

//S_XBee_ATResponse ATResponseStruct;

////////////////////Receive///////////////////

/**
  * @brief  Processa AT Response
  * @param  frame: trama recebida
	* @param  size: tamanho da trama recebida
  * @retval Estrutura da trama recebida
	* @note   trama e size inclui o identificador 0x88(AT Response)
	* @todo		TESTAR
  */
S_XBee_ATResponse * XBee_AtResponse(uint8_t * frame , uint8_t size){
	
	S_XBee_ATResponse * ATResp;
	
	
	ATResp->FrameId   = frame[1];
	ATResp->ATCommand = frame[2];
	ATResp->status    = frame[3];
	ATResp->SizeParams= size - 4;
	
	for (int i = 0 ; i < size-4 ; i++){
		ATResp->Params[i] = frame[i+4];
	}
	
	return ATResp;
}


/**
  * @brief  Processa trama com Zigbee Explicit RX Indicator
  * @param  frame: trama recebida
	* @param  size: tamanho da trama recebida
  * @retval Estrutura da trama recebida
	* @note   trama e size inclui o identificador 0x91(Zigbee Explicit RX Indicator)
	* @todo		TESTAR
  */
S_XBee_ZigbeeExplicitRXIndicator * XBee_ZigbeeExpRXInd (uint8_t * frame, uint8_t size){
	
	S_XBee_ZigbeeExplicitRXIndicator * zigbeeRx;
	
	for (int i = 0 ; i < 8 ; i++){
		zigbeeRx->SrcAddr64[i] = frame[i+1];
	}
	zigbeeRx->SrcAddr16 = ((frame[8]<<8)|frame[9]);
	zigbeeRx->SrcEndpoint = frame[10];
	zigbeeRx->DstEndpoint = frame[11];
	zigbeeRx->ClusterID = frame[12];
	zigbeeRx->ProfileID = frame[13];
	zigbeeRx->RcvOpt = frame[14];
	for (int i = 0 ; i < size-16 ; i++){
		zigbeeRx->Data[i] = frame[15+i];
	}
	zigbeeRx->SizeData = size-16;
	
	return zigbeeRx;
}

/////////////////////SEND///////////////////////
/**
  * @brief  Envia comando AT
  * @param  FrameID: valor FrameID
	* @param  ATCommand: comando AT
	* @param  param: buffer com parametros do comando AT
	* @param  sizeParams: tamanho ocupado pelos parametros
  * @retval 1 - sucesso ; 0 - erro
	* @todo		TESTAR
  */
uint8_t XBee_AtCommand(uint8_t FrameID, uint8_t ATCommand, uint8_t * param , uint8_t sizeParams){
	uint8_t buffer[128];
	
	uint16_t totalSize = sizeParams + 3; //tamanho de parametros util mais 3 (0x08 , FrameID , ATCommand)
	
	buffer[0] = 0x08;
	buffer[1] = FrameID;
	buffer[2] = ATCommand;
	
	for (int i = 0 ; i < sizeParams ; i++){
		buffer[3+i] = param[i];
	}
	
	Lib_SetUARTOutBufBytes(buffer, totalSize);
	Lib_UART_Transmit_wRetry_IT(&huart2);
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
  * @retval 1 - sucesso ; 0 - erro
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
	
	Lib_SetUARTOutBufBytes(buffer, totalSize);
	Lib_UART_Transmit_wRetry_IT(&huart2);
	return 1;
}

/////////////////Base de envio/////////////////////
/**
  * @brief  Encapsula uma trama e envia
  * @param  data: buffer com a trama util
	* @param  size: tamanho da trama util
  * @retval 1 - sucesso ; 0 - erro
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
