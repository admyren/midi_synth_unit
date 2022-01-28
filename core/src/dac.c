/**
 * The DAC on the STM32f407VGT is a dual channel DAC.
 * The configuration here uses both channels of the DAC and DMA
 * is used to feed data to both channels of the DAC from two buffers in RAM.
 * The DAC conversion is triggered by an update event on TIM8 (overflow).
*/

#include "drivers.h"



void DAC_init(uint16_t *bufferOut1, uint16_t *bufferOut2, uint16_t bufferSize)
{
	DMA_InitVals DMA_Init;

	/****** Clocks config ******/
	/* GPIOA clock enable */
	RCC->AHB1ENR |= (1 << 0);
	/* DAC clock enablefreq */
	RCC->APB1ENR |= (1 << 29);
	/* DMA1 clock enable */
	RCC->AHB1ENR |= (1 << 21);
	/* TIM8 clock enable */
	RCC->APB2ENR |= (1 << 1);

	/* Configure GPIO’s for analog mode
	   Outputs for DACs. DAC_OUT1 -> PA4, DAC_OUT2 -> PA5 */
	GPIOA->MODER |= (1 << 11) | (1 << 10) | (1 << 9) | (1 << 8);
   /**************************/

	/****** DAC config. ******/
	/* TIM8 TRGO as trigger source, trigger enable, enable channels */
	DAC->CR |= (1 << 19) | (1 << 3) | (1 << 18) | (1 << 2) | (1 << 16) | (1 << 0);
	/* DMA enable */
	DAC->CR |= (1 << 28) | (1 << 12);
	/******************************************************************************/

	/********************** DMA1 config. (memory -> periph.) **********************/
	DMA_Init.channel = 7;
	DMA_Init.direction = 1; /* MEM -> PERIPH */
	DMA_Init.bufferSize = bufferSize;
	DMA_Init.memAddr = (uint32_t)bufferOut1;
	DMA_Init.periphAddr = (uint32_t)&DAC->DHR12R1;
	DMA_Init.memDataSize = SIZE_HALF_WORD;
	DMA_Init.periphDataSize = SIZE_HALF_WORD;
	DMA_Init.mode = 1; /* Circular */
	DMA_Init.compIntEnable = 0;
	DMA_Init.halfIntEnable = 0;
	DMA_Init.dma_enable = 1;
	DMA_init(DMA1_Stream5, DMA_Init);

	DMA_Init.memAddr = (uint32_t)bufferOut2;
	DMA_Init.periphAddr = (uint32_t)&DAC->DHR12R2;
   DMA_Init.compIntEnable = 1;
	DMA_Init.halfIntEnable = 1;
	DMA_init(DMA1_Stream6, DMA_Init);

   /* Enable global interrupt for DMA1 stream 6 */
	NVIC_SetPriorityGrouping(2);
	NVIC_SetPriority (DMA1_Stream6_IRQn, 6);
	NVIC_EnableIRQ(DMA1_Stream6_IRQn);
   /**************************/

	/****** Timer 8 config. ******/
	/* Update event is selected as trigger output (TRGO) (Overflow in this case) */
	TIM8->CR2 |= (1 << 5);
	/* Prescaler */
	TIM8->PSC = 168-1; // Internal clock freq. is 168 MHz so this gives a tim freq. of 1MHz
	/* Auto reload */
	TIM8->ARR = 20-1;//1000000/SAMPLING_FREQUENCY;//20; // This gives a sampling freq. of 50kHz
	/* Enable counter */
	TIM8->CR1 |= (1 << 0);
	/**************************/

}

