#include "XBee.h"



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
	checksum = (checksum - 0xFF);
	
	buffer [3+size] = checksum & 0xFF;
	
	Lib_SetUARTOutBufBytes(buffer, totalSize);
	Lib_UART_Transmit_wRetry_IT(&huart2);
}