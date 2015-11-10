#include "stm32f4xx_hal.h"


/*******************   UART IT   *******************/
//verificar e obter byte
uint8_t Lib_GetUARTInBufByte(uint8_t *inbyte);

void Lib_SetUARTOutBufBytes(unsigned char *msg, char numbytes);

//transmitir informaçao por IT
HAL_StatusTypeDef Lib_UART_Transmit_IT(UART_HandleTypeDef *huart, uint8_t * data, uint16_t Size);

uint8_t Lib_UART_Transmit_wRetry_IT(UART_HandleTypeDef *huart);

//receber informaçao e coloca no buffer circular
void Lib_UART_Receive_IT (uint8_t * data, uint8_t size);
