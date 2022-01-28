#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "drivers.h"

#if 1
   



//#define DMA_AND_INTERRUPT_ENABLE

uint32_t UART_RX_buff_size = 0;

void (*tx_ready_func)(void) = 0;
void (*rx_ready_func)(void) = 0;

uint8_t UART_TX_buffer[200];

void UART_Init(USART_TypeDef* huart, uint32_t baud, uint8_t* RX_buff, uint8_t RX_size)
{
   // DMA1 and DMA2 clock enable
#ifdef DMA_AND_INTERRUPT_ENABLE
   RCC->AHB1ENR |= (1 << 22) | (1 << 21);
   DMA_InitVals DMA_UART_RX_Init;
   DMA_InitVals DMA_UART_TX_Init;

#endif
   uint32_t pclk1;
   uint32_t div;
   uint8_t carry;
   // DMA1 clock enable
   //RCC->AHB1ENR |= (1 << 21);
   // UART4 clock enable
   RCC->APB1ENR |= (1 << 19);
   // GPIOC clock enable
   RCC->AHB1ENR |= (1 << 2);
   // PC10 -> TX, PC11 -> RX
   GPIOC->MODER |= (1 << 23) | (1 << 21);
   GPIOC->OSPEEDR |= (1 << 23) | (1 << 22) | (1 << 21) | (1 << 20);
   GPIOC->AFR[1] |= (8 << 12) | (8 << 8);
   // Calculate division factor
   pclk1 = HAL_RCC_GetPCLK1Freq();// This gives the actual freq divided by two, why?
   //pclk1 = 42000000U;
   //pclk1 = RCC_get_APB1_freq();
   div = pclk1 << 2;
   div = div/baud;
   carry = (uint8_t)(div & 0x01);
   div >>= 1;
   div += carry;
   huart->BRR |= (0x0000ffff & div);

   // These two also work
   //huart->BRR |= (42000000L)/baud;
   //huart->BRR |= (pclk1)/baud;

#ifdef DMA_AND_INTERRUPT_ENABLE
   /* Idle line interrupt enable */
   huart->CR1 |= (1 << 4);
#endif

   /* UART enable, transmitter & receiver enable */
   huart->CR1 |= (1 << 13) | (1 << 3) | (1 << 2);

#ifdef DMA_AND_INTERRUPT_ENABLE
   /* UART4 RX DMA is on channel 4, stream 2 on DMA1 */
   /* UART DMA receiver */
   huart->CR3 |= (1 << 6);

   DMA_UART_RX_Init.channel = 4;
   DMA_UART_RX_Init.direction = 0; /* Peripheral to memory */
   DMA_UART_RX_Init.bufferSize = RX_size;
   DMA_UART_RX_Init.memAddr = (uint32_t)RX_buff;
   DMA_UART_RX_Init.periphAddr = (uint32_t)(&huart->DR);
   DMA_UART_RX_Init.memDataSize = SIZE_BYTE;
   DMA_UART_RX_Init.periphDataSize = SIZE_BYTE;
   DMA_UART_RX_Init.mode = 1;   /* Circular mode */
   DMA_UART_RX_Init.compIntEnable = 1;
   DMA_UART_RX_Init.halfIntEnable = 0;
   DMA_UART_RX_Init.dma_enable = 1;

   DMA_init(DMA1_Stream2, DMA_UART_RX_Init);

   UART_RX_buff_size = RX_size;

   /* UART4 TX DMA is on channel 4, stream 4 on DMA1 */
   /* UART DMA transmitter */
   huart->CR3 |= (1 << 7);

   DMA_UART_TX_Init.channel = 4;
   DMA_UART_TX_Init.direction = 1; /* Memory to peripheral */
   DMA_UART_TX_Init.bufferSize = 0;
   DMA_UART_TX_Init.memAddr = (uint32_t)UART_TX_buffer;
   DMA_UART_TX_Init.periphAddr = (uint32_t)(&huart->DR);
   DMA_UART_TX_Init.memDataSize = SIZE_BYTE;
   DMA_UART_TX_Init.periphDataSize = SIZE_BYTE;
   DMA_UART_TX_Init.mode = 0;   /* Circular mode */
   DMA_UART_TX_Init.compIntEnable = 1;
   DMA_UART_TX_Init.halfIntEnable = 0;
   DMA_UART_TX_Init.dma_enable = 0;

   DMA_init(DMA1_Stream4, DMA_UART_TX_Init);


   NVIC_SetPriorityGrouping(2);
   NVIC_SetPriority (DMA1_Stream2_IRQn, 8);
   NVIC_EnableIRQ(DMA1_Stream2_IRQn);

   NVIC_SetPriorityGrouping(2);
   NVIC_SetPriority (DMA1_Stream4_IRQn, 8);
   NVIC_EnableIRQ(DMA1_Stream4_IRQn);

   NVIC_SetPriorityGrouping(2);
   NVIC_SetPriority (UART4_IRQn, 7);
   NVIC_EnableIRQ(UART4_IRQn);
#endif
}

void UART_Transmit_DMA(USART_TypeDef* huart, uint8_t* data_buffer, uint32_t size)
{
   memcpy(UART_TX_buffer, data_buffer, size);

   DMA_start_TX(DMA1_Stream4, size);
}

// Data should be a null terminated string if size == 0.
// if size > 0 then it will send size number of elements
void UART_Transmit(USART_TypeDef* huart, uint8_t* data, uint8_t size)
{
   if(size == 0)
   {
      for(uint8_t i=0; data[i] != '\0'; i++)
      {

         huart->DR = data[i];
         while(!(huart->SR & (1 << 6)));
      }
   }
   else
   {
      for(uint8_t i=0; i<size; i++)
      {

         huart->DR = data[i];
         while(!(huart->SR & (1 << 6)));
      }
   }
}


void UART_register_rx_complete_func(void (*func)(void))
{
   rx_ready_func = func;
}

void UART_register_tx_complete_func(void (*func)(void))
{
   tx_ready_func = func;
}



void DMA1_Stream2_IRQHandler(void)
{
   // Transfer complete interrupt?
   if(DMA1->LISR & (1 << 21))
   {
      // Clear interrupt flag
      DMA1->LIFCR |= 1 << 21;

      if (rx_ready_func != 0)
      {
         rx_ready_func();
      }

      // Enable DMA stream again
      DMA1_Stream2->NDTR = UART_RX_buff_size;
      DMA1_Stream2->CR |= (1 << 0);
   }
}

void DMA1_Stream4_IRQHandler(void)
{
   // Transfer complete interrupt?
   if(DMA1->HISR & (1 << 5))
   {
      // Clear interrupt flag
      DMA1->HIFCR |= 1 << 5;

      if(tx_ready_func != 0)
      {
         tx_ready_func();
      }
   }
}


void UART4_IRQHandler(void)
{
   // Idle line detect interrupt?
   if(UART4->SR & (1 << 4))
   {
      // Disable DMA stream to force transfer complete interrupt
      DMA1_Stream2->CR &= ~(1 << 0);
      uint8_t dummy = UART4->DR;
      (void)dummy; // Prevent compiler warnings
   }
}



#if 0

uint32_t UART_config(uart_reg_t* uart_dev, uart_config_t* uart_conf)
{
   uint32_t ret = 0;
   uint32_t hclk;
   uint32_t uart_div;

   /* Set OVER8 bit to over sample by 8 */
   uart_dev->CR1 |= 1 << 15;

   /* UART 2, 3, 4 and 5 is part of APB1 and uses
    * UART 1 and 6 is part of APB2 */

   hclk = RCC_get_APB1_freq();
   uart_div = (2000000 * hclk) / uart_conf->baud_rate;
   uart_dev->BRR = uart_div;

   if (DATA_BITS_8 == uart_conf->data_bits)
   {
      uart_dev->CR1 &= ~(1 << 12);
   }
   else
   {
      uart_dev->CR1 |= 1 << 12;
   }

   uart_dev->CR2 &= ~(0x3 << 12);
   if (STOP_BITS_2 == uart_conf->stop_bits)
   {
      uart_dev->CR2 |= 1 << 13;
   }

   if (enable == uart_conf->RX_enable)
   {
      uart_dev->CR1 |= 1 << 2;
   }

   if (enable == uart_conf->TX_enable)
   {
      uart_dev->CR1 |= 1 << 3;
   }



   /* Enable UART */
   uart_dev->CR1 |= 1 << 13;

   return(ret);
}

void UART_tx(uart_reg_t* uart_dev, uint8_t* data, uint32_t size)
{
   uint32_t i;
   for (i=0; i<size; i++)
   {
      uart_dev->DR = data[i];
      while(!(uart_dev->SR & (1 << 6)));
   }

}
#endif
#else

#define TX_BUFF_SIZE 1000

uint8_t uart_tx_buffer[TX_BUFF_SIZE];
uint32_t uart_rx_buff_size = 0;
/* Callback for receive ready */
void (*uart_rx_callback)(void) = NULL;
volatile bool uart_tx_ready = true;

void UART_register_rx_callback(void (*callback)(void))
{
   uart_rx_callback = callback;
}

static uint32_t calculate_baud_divider(uint32_t baud, uint32_t pclk)
{
	uint32_t div;
	uint8_t carry;

	div = pclk << 1;
	div = div/baud;
	carry = (uint8_t)(div & 0x01);
	div >>= 1;
	div += carry;

	// This two also "work"
	//div |= pclk/baud;

   return div;
}

void UART5_init(void)
{
	/************************ Enable peripheral clock *************************/
	/* UART5 clock enable */
	RCC->APB1ENR |= (1 << 20);
	/* GPIOC clock enable */
	RCC->AHB1ENR |= (1 << 2);
	/**************************************************************************/

	/*********************** Configure GPIOs for UART *************************/
	/* TX -> PC12 */
	GPIOC->MODER |= (1 << 25);
	/* UART5 is on AF8 */
	GPIOC->AFR[1] |= (8 << 16);
	/**************************************************************************/

	/**************************** Configure UART ******************************/
	//UART5->BRR = calculate_baud_divider(9600, HAL_RCC_GetPCLK1Freq());
	//UART5->BRR = calculate_baud_divider(9600, 42000000);
	UART5->BRR |= (273 << 4) | 0b0111;


	/* Transmitter enable */
	UART5->CR1 |= (1 << 3);

	/* UART enable */
	UART5->CR1 |= (1 << 13);

	/**************************************************************************/
}

void UART5_transmitt(uint8_t* data, uint32_t size)
{
	uint32_t i;

	for (i = 0; i < size; ++i)
	{
		UART5->DR = data[i];
		/* Wait for TXE to be set */
		while(!(UART5->SR & (1 << 7)));
	}
}


void UART_init(USART_TypeDef* huart, uint32_t baud, uint8_t* dst_buffer, uint32_t rx_buffer_size)
{
	DMA_InitVals DMA_Init;

	/************************ Enable peripheral clock *************************/
	/* DMA1 clock enable */
	RCC->AHB1ENR |= (1 << 21);
	/* UART4 clock enable */
	RCC->APB1ENR |= (1 << 19);
	/* GPIOC clock enable */
	RCC->AHB1ENR |= (1 << 2);
	/**************************************************************************/

	/*********************** Configure GPIOs for UART *************************/
	/* PC10 -> TX, PC11 -> RX */
	GPIOC->MODER |= (1 << 23) | (1 << 21);
   /* UART4 is AF8 */
	GPIOC->AFR[1] |= (8 << 12) | (8 << 8);
	/**************************************************************************/


	/**************************** Configure UART ******************************/
   //huart->BRR |= 0x0000ffff & calculate_baud_divider(baud, HAL_RCC_GetPCLK1Freq());
	huart->BRR |= (273 << 4) | 0b0111;
	// Idle line interrupt enable
	//huart->CR1 |= (1 << 4);
   /* UART4 RX DMA is on channel 4, stream 2 on DMA1 */
	/* UART DMA receiver */
	//huart->CR3 |= (1 << 6);
   /* UART DMA transmitter */
   //huart->CR3 |= (1 << 7);
	/* UART enable, transmitter & receiver enable */
	huart->CR1 |= (1 << 13) | (1 << 3) | (1 << 2);
   /* TXE interrupt */
   //huart->CR1 |= (1 << 7);
	/**************************************************************************/

	/***************************** Configure DMA ******************************/
	DMA_Init.channel = 4;
	DMA_Init.direction = 0; // Peripheral to memory
	DMA_Init.bufferSize = rx_buffer_size;
	DMA_Init.memAddr = (uint32_t)dst_buffer;
	DMA_Init.periphAddr = (uint32_t)(&huart->DR);
	DMA_Init.memDataSize = SIZE_BYTE;
	DMA_Init.periphDataSize = SIZE_BYTE;
	DMA_Init.mode = 1;	// Circular mode
	DMA_Init.compIntEnable = 1;
	DMA_Init.halfIntEnable = 0;
   DMA_Init.dma_enable = 1;

	DMA_init(DMA1_Stream2, DMA_Init);

   uart_rx_buff_size = rx_buffer_size;

   /* Init DMA for UART4 TX */
   DMA_Init.channel = 4;
   DMA_Init.direction = 1; /* Memory to peripheral */
   DMA_Init.bufferSize = 0;
   DMA_Init.memAddr = (uint32_t)uart_tx_buffer;
   DMA_Init.periphAddr = (uint32_t)(&huart->DR);
   DMA_Init.memDataSize = SIZE_BYTE;
   DMA_Init.periphDataSize = SIZE_BYTE;
   DMA_Init.mode = 0;   /* Circular mode */
   DMA_Init.compIntEnable = 1;
   DMA_Init.halfIntEnable = 0;
   DMA_Init.dma_enable = 0;

   DMA_init(DMA1_Stream4, DMA_Init);
	/**************************************************************************/

	/************************** Configure interrupt ***************************/
   /* DMA transfer complete interrupt for UART TX */
   NVIC_SetPriorityGrouping(2);
	NVIC_SetPriority(DMA1_Stream4_IRQn, 8);
	NVIC_EnableIRQ(DMA1_Stream4_IRQn);

   /* DMA transfer complete interrupt for UART RX */
	NVIC_SetPriorityGrouping(2);
	NVIC_SetPriority(DMA1_Stream2_IRQn, 8);
	NVIC_EnableIRQ(DMA1_Stream2_IRQn);

   /* UART idle line interrupt */
	NVIC_SetPriorityGrouping(2);
	NVIC_SetPriority(UART4_IRQn, 7);
	NVIC_EnableIRQ(UART4_IRQn);
	/**************************************************************************/
}

/* Send data with DMA */
void UART_transmit(uint8_t* data_buffer, uint32_t data_size)
{
#if 0
   while(uart_tx_ready == false);
   uart_tx_ready = false;
   memcpy(uart_tx_buffer, data_buffer, data_size);

   /* Clear TC bit */
   UART4->SR &= ~(1 << 6);

   /* Set size of buffer to be sent */
   DMA1_Stream4->NDTR = data_size;
   /* Enable DMA to start transmit */
   DMA1_Stream4->CR |= (1 << 0);
#else
   uint32_t i = 0;
   for (i = 0; i < data_size; ++i)
   {
      while(!(UART4->SR & (1 << 7)));
      UART4->DR = data_buffer[i];
   }
#endif
}

void DMA1_Stream4_IRQHandler(void)
{
   // Transfer complete interrupt?
	if(DMA1->HISR & (1 << 5))
	{

      uart_tx_ready = true;
		// Clear interrupt flag
		DMA1->HIFCR |= 1 << 5;
	}
}

void DMA1_Stream2_IRQHandler(void)
{
	// Transfer complete interrupt?
	if(DMA1->LISR & (1 << 21))
	{
      /* Disable DMA reception for UART */
      UART4->CR3 &= ~(1 << 6);

		// Clear interrupt flag
		DMA1->LIFCR |= 1 << 21;
		// Enable DMA stream again
		DMA1_Stream2->NDTR = uart_rx_buff_size;
		DMA1_Stream2->CR |= (1 << 0);

      if (uart_rx_callback != NULL)
      {
         uart_rx_callback();
      }
      /* Enable DMA reception for UART */
      UART4->CR3 |= (1 << 6);
	}
}

void UART4_IRQHandler(void)
{
   volatile uint32_t test_cnt = 0;
	// Idle line detect interrupt?
	if(UART4->SR & (1 << 4))
	{
      while(test_cnt < 10000)
      {
         test_cnt++;
      }
		// Disable DMA stream to force transfer complete interrupt
		DMA1_Stream2->CR &= ~(1 << 0);
		uint8_t dummy = UART4->DR;
		(void)dummy; // Prevent compiler warnings
	}
}
#endif

