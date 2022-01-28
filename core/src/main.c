#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "stm32f407xx.h"
#include "drivers.h"
#include "wave_tables.h"
#include "midi.h"
#include "oscillator.h"

#define DAC_BUFF_SIZE 256
#define UART_BUFF_SIZE 256

volatile uint8_t uart_buff[UART_BUFF_SIZE];

volatile uint32_t intr_cnt = 1;
volatile uint32_t intr_cnt_comp = 1;
volatile uint32_t intr_cnt_half = 1;
volatile uint16_t dac_buff1[DAC_BUFF_SIZE];
volatile uint16_t dac_buff2[DAC_BUFF_SIZE];
volatile uint16_t* volatile dac_ch1_ptr;
volatile uint16_t* volatile dac_ch2_ptr;

volatile bool half_buff_int = false;
volatile bool full_buff_int = false;
volatile bool dac_buff_ready = false;


oscillator_t osc1;

void SystemClock_Config(void);

int main(void)
{
  int i = 0;
  unsigned int shifter = 0;
  uint32_t buff_idx = 0;
  uint32_t freq = 0;
  volatile uint32_t cnt = 0;
  bool note_on = true;
  uint32_t buff_offset = 0;
  int16_t* table = sine_table;
  uint8_t uart_buff[UART_BUFF_SIZE];
 
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  UART_Init(UART4, 9600, uart_buff, UART_BUFF_SIZE);

  memset((void*)dac_buff1, 0, sizeof(uint16_t)*DAC_BUFF_SIZE);
  memset((void*)dac_buff2, 0, sizeof(uint16_t)*DAC_BUFF_SIZE);

  DAC_init((uint16_t*)dac_buff1, (uint16_t*)dac_buff2, DAC_BUFF_SIZE);
  oscillator_init(&osc1);
  
  /* Init PD12, PD13, PD14 and PD15 */
  RCC->AHB1ENR |= 1 << 3;
  GPIOD->MODER = (1 << 30) | (1 << 28) | (1 << 26) | (1 << 24); 

  shifter = 0x1000;
  while (1)
  {
    uint16_t sample;
    int16_t amp = 0;
    freq = 20;

    while(dac_buff_ready == false);
    dac_buff_ready = false;
    buff_offset = half_buff_int ? 0 : DAC_BUFF_SIZE/2;
    for(i = 0; i < DAC_BUFF_SIZE/2; ++i)
    {
      dac_buff1[buff_offset + i] = (uint16_t)(((table[buff_idx] * amp) >> 11) + 2048);
      dac_buff2[buff_offset + i] = (uint16_t)(((table[buff_idx] * amp) >> 11) + 2048);
      buff_idx += freq;
      buff_idx &= 0x7ff;
    }





    if ((cnt % 10) == 0)
    {
      
      GPIOD->ODR = shifter;
      shifter <<= 1;
      if (shifter == 0x10000)
      {
        UART_Transmit(UART4, "hejhej", 6);
        shifter = 0x1000;
      }
    }
    cnt++;
  }
}

void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks
    */
#if 0
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16; // 1MHz clock input to PLL
  RCC_OscInitStruct.PLL.PLLN = 360; // PLL multiplier, output of PLL = 1*360 = 360MHz
  RCC_OscInitStruct.PLL.PLLP = 2; // System clock = output of PLL / 2 = 180MHz
  RCC_OscInitStruct.PLL.PLLQ = 7;
#else
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  //RCC_OscInitStruct.HSECalibrationValue = 8;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8; // 1MHz clock input to PLL
  RCC_OscInitStruct.PLL.PLLN = 336; // PLL multiplier, output of PLL = 1*336 = 336MHz
  RCC_OscInitStruct.PLL.PLLP = 2; // System clock = output of PLL / 2 = 168MHz
  RCC_OscInitStruct.PLL.PLLQ = 7;
#endif
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
  }

  /**Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK |
		                        RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
  }

  SystemCoreClockUpdate();


  /**Configure the Systick
  */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  /**Configure the Systick interrupt time
  */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

#if 0
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}
#endif


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

}
#endif /* USE_FULL_ASSERT */



// Synchronization for DAC buffers
#if 1
void DMA1_Stream6_IRQHandler(void)
{
  intr_cnt++;
	// Half transfer complete interrupt?
	if(DMA1->HISR & (1 << 20))
	{
    //dac_ch1_ptr = &dac_buff1[0];
    //dac_ch2_ptr = &dac_buff2[0];
    dac_buff_ready = true;
    half_buff_int = true;
    full_buff_int = false;
    intr_cnt_half++;
		// Clear interrupt flag
		DMA1->HIFCR |= 1 << 20;
	}
	// Transfer complete interrupt?
	else if(DMA1->HISR & (1 << 21))
	{
    //dac_ch1_ptr = &dac_buff1[DAC_BUFF_SIZE/2];
    //dac_ch2_ptr = &dac_buff2[DAC_BUFF_SIZE/2];
    dac_buff_ready = true;
    half_buff_int = false;
    full_buff_int = true;
    intr_cnt_comp++;
		// Clear interrupt flag
		DMA1->HIFCR |= 1 << 21;
	}
}
#endif