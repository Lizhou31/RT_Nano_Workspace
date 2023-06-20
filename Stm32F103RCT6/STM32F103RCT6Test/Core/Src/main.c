/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DECODE_THREAD_PRIORITY 9
#define PERIODIC_OUTPUT_THREAD_PRIORITY 10
#define THREAD_TIMESLICE 5

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
SBUS_Handler sbushandler;

struct rt_event io_event;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void sbus_fetch(SBUS_Handler *handler)
{
  HAL_UART_Receive_IT(&huart2, (uint8_t *)(handler->rx_data.raw_data), 25);
}

int check_5s_RC_Signal(void)
{
  int count = 0;
  while (count++ < 5)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      rt_kprintf("%d ", sbushandler.rx_data.channel_decoded[i]);
    }
    rt_kprintf("\n");
    rt_thread_mdelay(1000);
  }
  return RT_EOK;
}
MSH_CMD_EXPORT(check_5s_RC_Signal, check RC signal);

ALIGN(RT_ALIGN_SIZE)
static char rc_decode_stack[1024];
static struct rt_thread rc_decode_thread;
static void rc_decode(void *param)
{
  rt_uint32_t e;
  while (1)
  {
    if (rt_event_recv(&io_event, (rc_interrupt_event | rc_output_completed_event),
                      RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER, &e) == RT_EOK)
    {
      sbushandler.sbus_decode(&sbushandler);
    }
  }
}

// #define OUTPUT_DEBUG
ALIGN(RT_ALIGN_SIZE)
static char periodic_output_stack[1024];
static struct rt_thread rc_p_output_thread;
static void periodic_output(void *param)
{
  int count = 0;
  while (1)
  {
#ifdef OUTPUT_DEBUG
    rt_kprintf("%d ", sbushandler.rx_data.channel_decoded[count]);
#endif
    count++;
    if (count >= 8)
    {
      count = 0;
#ifdef OUTPUT_DEBUG
      rt_kprintf("\n");
#endif
      rt_event_send(&io_event, rc_output_completed_event);
    }
    rt_thread_mdelay(32);
  }
}

int main(void)
{
  sbushandler.sbus_get_tick = rt_tick_get;
  sbushandler.sbus_raw_fetch = sbus_fetch;
  SBUS_DEFINE(sbus, &sbushandler);
  sbushandler.sbus_raw_fetch(&sbushandler);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

  /* 初始化事件对象 */
  rt_err_t result;
  result = rt_event_init(&io_event, "event", RT_IPC_FLAG_PRIO);
  if (result != RT_EOK)
  {
    rt_kprintf("init event failed.\n");
    return -1;
  }

  rt_thread_init(&rc_decode_thread,
                 "rc_decode",
                 rc_decode,
                 RT_NULL,
                 &rc_decode_stack[0],
                 sizeof(rc_decode_stack),
                 DECODE_THREAD_PRIORITY, THREAD_TIMESLICE);
  rt_thread_startup(&rc_decode_thread);

  rt_thread_init(&rc_p_output_thread,
                 "rc_output",
                 periodic_output,
                 RT_NULL,
                 &periodic_output_stack[0],
                 sizeof(periodic_output_stack),
                 PERIODIC_OUTPUT_THREAD_PRIORITY, THREAD_TIMESLICE);
  rt_thread_startup(&rc_p_output_thread);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
