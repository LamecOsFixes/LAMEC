#include "UART_Handler.h"

/************Constantes****************/
const uint8_t UARTINBUFSIZE  = 250;
const uint8_t UARTOUTBUFSIZE = 250;
/**************Variaveis***************/
uint8_t 	UARTInBuf[UARTINBUFSIZE];		//Buffer recepçao
uint8_t 	UARTOutBuf[UARTOUTBUFSIZE];	//Buffer envio
uint32_t	UARTInBufNumbytes=0;
uint32_t	UARTOutBufNumbytes =0;

uint16_t		UARTInBufNextRead=0;
uint16_t   	UARTInBufNextWrite=0;
uint16_t		UARTOutBufNextRead=0;
uint16_t		UARTOutBufNextWrite=0;

//uint8_t sendSuccessful = 1;
/***************Funçoes****************/
///////////////BUFFER IN/////////////////
/**
  * @brief  Verifica se existem bytes para ler no buffer de entrada
  * @retval 1 se houver bytes novos; 0 se nao houver
  */
uint8_t Lib_GetUARTInBufNumbytes(void)
{
	if(UARTInBufNextWrite == UARTInBufNextRead)
		return 0;
	else 
		return 1;
}


/**
  * @brief  Retorna 1 byte do buffer circular de recepçao
  * @param  inbyte: caracter a retornar
  * @retval 1 se houver caracter novo ; 0 se falhar ao obter um caracter novo
  */
uint8_t Lib_GetUARTInBufByte(uint8_t *inbyte)
{
  if(Lib_GetUARTInBufNumbytes()){
			*inbyte = UARTInBuf[UARTInBufNextRead];
			
			if(UARTInBufNextRead == UARTINBUFSIZE - 1) {
				UARTInBufNextRead = 0; // wrap-around
			} else { 
				UARTInBufNextRead++;
			}
			
			return 1;
	} else {
			return 0;
	}
}


///////////////BUFFER OUT/////////////////
/**
  * @brief  Verifica se existem bytes para ler no buffer de saida
  * @retval 1 se houver bytes novos; 0 se nao houver
  */
uint8_t Lib_GetUARTOutBufNumbytes(void)
{
	if(UARTOutBufNextWrite == UARTOutBufNextRead)
		return 0;
	else 
		return 1;
}

/**
  * @brief  Retorna 1 byte do buffer circular de envio
  * @param  inbyte: caracter a retornar
  * @retval 1 se houver caracter novo ; 0 se falhar ao obter um caracter novo
  */
uint8_t Lib_GetUARTOutBufByte(uint8_t *inbyte)
{
  if(Lib_GetUARTOutBufNumbytes()){
			*inbyte = UARTOutBuf[UARTOutBufNextRead];
			
			if(UARTOutBufNextRead == UARTOUTBUFSIZE - 1) {
				UARTOutBufNextRead = 0; // wrap-around
			} else { 
				UARTOutBufNextRead++;
			}
			
			return 1;
	} else {
			return 0;
	}
}

/**
  * @brief  carrega buffer circular de envio
  * @param  msg: caracteres a carregar
	* @param	numbytes: tamanho de informaçao
  * @retval 1 se houver caracter novo ; 0 se falhar ao obter um caracter novo
  */
void Lib_SetUARTOutBufBytes(unsigned char *msg, char numbytes)
{
  char i;
  
  for(i=0; i<numbytes; i++)
  {
    UARTOutBuf[UARTOutBufNextWrite] = msg[i];
    if(UARTOutBufNextWrite == UARTOUTBUFSIZE - 1) {
			UARTOutBufNextWrite = 0; // wrap-around
    }else { 
			UARTOutBufNextWrite++;
		}
    UARTOutBufNumbytes++;   // increase numbytes by 1
  }
}

////////////TRASMISSION///////////////////
/**
  * @brief  Coloca informaçao recebida num buffer circular 
  * @param  huart: apontador para estrutura UART_HandleTypeDef que tem informaçao sobre as 
	*                configuraçoes do modulo de UART
  * @param  data: apontador para buffer de informaçao a enviar
  * @param  Size: tamanho de informaçºao a enviar
  * @retval HAL status (HAL_OK , HAL_ERROR , HAL_BUSY)
  */
void Lib_UART_Receive_IT (uint8_t * data, uint8_t size)
{	
	uint16_t i=0;
	
	for( i=0 ; i< size	; i++)
	{
		UARTInBuf[UARTInBufNextWrite] = data[i];
    UARTInBufNumbytes++;

		if(UARTInBufNextWrite == UARTINBUFSIZE - 1) 
       UARTInBufNextWrite = 0;  // wrap-around
    else 
       UARTInBufNextWrite++;	
	}
}

/**
  * @brief  Transmite informaçao pela UART com IT
  * @param  huart: apontador para estrutura UART_HandleTypeDef que tem informaçºao sobre as 
	*                configuraçoes do modulo de UART
  * @param  data: apontador para buffer de informaçao a enviar
  * @param  Size: tamanho de informaçºao a enviar
  * @retval HAL status (HAL_OK , HAL_ERROR , HAL_BUSY)
  */
HAL_StatusTypeDef Lib_UART_Transmit_IT(UART_HandleTypeDef *huart, uint8_t * data, uint16_t Size)
{
	HAL_StatusTypeDef result;
  result = HAL_UART_Transmit_IT(huart, data , Size );
	
  return result;
}
/**
  * @brief  Transmite informaçao pela UART com IT com retry ate conseguir enviar
  * @param  huart: apontador para estrutura UART_HandleTypeDef que tem informaçºao sobre as 
	*                configuraçoes do modulo de UART
  * @param  data: apontador para buffer de informaçao a enviar
  * @param  Size: tamanho de informaçºao a enviar
  * @retval HAL status (HAL_OK , HAL_ERROR , HAL_BUSY)
  */
uint8_t Lib_UART_Transmit_wRetry_IT(UART_HandleTypeDef *huart)
{
	uint8_t send_char = 0x00;
	while(Lib_GetUARTOutBufByte(&send_char)){	//enquanto houver caracteres para transmitir
		
			while(Lib_UART_Transmit_IT(huart,&send_char,1)!=HAL_OK)	//tenta enviar ate conseguir
			{
			}
	}
	return 1;
}

