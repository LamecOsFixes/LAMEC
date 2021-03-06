/**
		\file 	UART_Handler.c
		\brief	UART interface functions.
	*/
	
/**
		\addtogroup UART UART Handler
		\{
	*/
#include "UART_Handler.h"


/************Constantes****************/
/// \brief Tamanho alocado para buffer UARTInBuf.
const uint8_t UARTINBUFSIZE  = 250;

/// \brief Tamanho alocado para buffer UARTOutBuf.
const uint8_t UARTOUTBUFSIZE = 250;

/**************Variaveis***************/
/// \brief Buffer recep�ao
uint8_t 	UARTInBuf[UARTINBUFSIZE];		

/// \brief Buffer envio
uint8_t 	UARTOutBuf[UARTOUTBUFSIZE];	

//uint32_t	UARTInBufNumbytes=0;
//uint32_t	UARTOutBufNumbytes =0;

/// \brief Apontador para proxima leitura do buffer UARTInBuf.
uint16_t		UARTInBufNextRead=0;

/// \brief Apontador para proxima escrita do buffer UARTInBuf.
uint16_t   	UARTInBufNextWrite=0;

/// \brief Apontador para proxima leitura do buffer UARTOutBuf.
uint16_t		UARTOutBufNextRead=0;

/// \brief Apontador para proxima escrita do buffer UARTOutBuf.
uint16_t		UARTOutBufNextWrite=0;

//uint8_t sendSuccessful = 1;

/***************Fun�oes****************/
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
  * @brief  Retorna 1 byte do buffer circular de recep�ao
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
	* @param	numbytes: tamanho de informa�ao
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
    //UARTOutBufNumbytes++;   // increase numbytes by 1
  }
}

////////////TRASMISSION///////////////////
/**
  * @brief  Coloca informa�ao recebida num buffer circular 
  * @param  huart: apontador para estrutura UART_HandleTypeDef que tem informa�ao sobre as 
	*                configura�oes do modulo de UART
  * @param  data: apontador para buffer de informa�ao a enviar
  * @param  Size: tamanho de informa��o a enviar
  * @retval HAL status (HAL_OK , HAL_ERROR , HAL_BUSY)
  */
void Lib_UART_Receive_IT (uint8_t * data, uint8_t size)
{	
	uint16_t i=0;
	
	for( i=0 ; i< size	; i++)
	{
		UARTInBuf[UARTInBufNextWrite] = data[i];
//    UARTInBufNumbytes++;

		if(UARTInBufNextWrite == UARTINBUFSIZE - 1) 
       UARTInBufNextWrite = 0;  // wrap-around
    else 
       UARTInBufNextWrite++;	
	}
}

/**
  * @brief  Transmite informa�ao pela UART com IT
  * @param  huart: apontador para estrutura UART_HandleTypeDef que tem informa�ao sobre as 
	*                configura�oes do modulo de UART
  * @param  data: apontador para buffer de informa�ao a enviar
  * @param  Size: tamanho de informa�ao a enviar
  * @retval HAL status (HAL_OK , HAL_ERROR , HAL_BUSY)
  */
HAL_StatusTypeDef Lib_UART_Transmit_IT(UART_HandleTypeDef *huart, uint8_t * data, uint16_t Size)
{
	HAL_StatusTypeDef result;
  result = HAL_UART_Transmit_IT(huart, data , Size );
	
  return result;
}
/**
  * @brief  Transmite informa�ao pela UART com IT com retry ate conseguir enviar
  * @param  huart: apontador para estrutura UART_HandleTypeDef que tem informa�ao sobre as 
	*                configura�oes do modulo de UART
  * @param  data: apontador para buffer de informa�ao a enviar
  * @param  Size: tamanho de informa�ao a enviar
  * @retval HAL status (HAL_OK , HAL_ERROR , HAL_BUSY)
  */
uint8_t Lib_UART_Transmit_wRetry_IT(UART_HandleTypeDef *huart)
{
	uint32_t timout=0;
	uint8_t send_char = 0x00;
	while(Lib_GetUARTOutBufByte(&send_char)){	//enquanto houver caracteres para transmitir
		timout=HAL_GetTick();
			while(Lib_UART_Transmit_IT(huart,&send_char,1)!=HAL_OK)	//tenta enviar ate conseguir
			{
				if(HAL_GetTick()>(timout+20)){
					return 0;
				}
			}
	}
	return 1;
}

/// \}

