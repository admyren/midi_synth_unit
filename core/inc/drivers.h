#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <stdint.h>
#include <stdlib.h>

#include "stm32f407xx.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#define SAMPLING_FREQUENCY	50000

typedef enum
{
	SIZE_BYTE = 0,
	SIZE_HALF_WORD = 1,
	SIZE_WORD = 2
}DMA_DataSize;

typedef struct
{
	uint32_t channel;				// DMA channel
	DMA_DataSize periphDataSize;	// Peripheral data size
	DMA_DataSize memDataSize;		// Memory data size
	uint32_t direction;				// Direction, memory->peripheral(1) or peripheral->memory(0)
	uint32_t bufferSize;			// Number of element in the DMA buffer
	uint32_t periphAddr;			// Address to peripheral data register
	uint32_t memAddr;				// Address to first element of DMA buffer
	uint32_t halfIntEnable;			// DMA half transfer complete interrupt enable
	uint32_t compIntEnable;			// DMA transfer complete interrupt enable
	uint32_t mode;					// Circular mode or not
   uint32_t dma_enable;
}DMA_InitVals;


void DAC_init(uint16_t *bufferOut1, uint16_t *bufferOut2, uint16_t bufferSize);
//void UART_init(USART_TypeDef* huart, uint32_t baud, uint8_t* dst_buffer, uint32_t rx_buffer_size);
//void UART_transmit(uint8_t* data_buffer, uint32_t data_size);
void DMA_init(DMA_Stream_TypeDef* hdma, DMA_InitVals idma);
void DMA_start_TX(DMA_Stream_TypeDef* hdma, uint32_t buff_size);

//void UART5_init(void);
//void UART5_transmitt(uint8_t* data, uint32_t size);
void UART_Init(USART_TypeDef* huart, uint32_t baud, uint8_t* RX_buff, uint8_t RX_size);

void UART_Transmit_DMA(USART_TypeDef* huart, uint8_t* data_buffer, uint32_t size);
void UART_Transmit(USART_TypeDef* huart, uint8_t* data, uint8_t size);
void UART_register_rx_complete_func(void (*func)(void));
void UART_register_tx_complete_func(void (*func)(void));

#endif

