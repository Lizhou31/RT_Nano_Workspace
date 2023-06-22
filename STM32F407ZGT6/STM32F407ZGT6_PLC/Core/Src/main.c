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
#include "rtc.h"
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
#define EMERGENCY_THREAD_PRIORITY 3
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
SBUS_Handler sbushandler;

struct rt_event control_event;
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

int check_5s_RC_signal(void)
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
MSH_CMD_EXPORT(check_5s_RC_signal, check RC signal);

/* Decode Thread */
ALIGN(RT_ALIGN_SIZE)
static char rc_decode_stack[512];
static struct rt_thread rc_decode_thread;
static void rc_decode(void *param)
{
  rt_uint32_t e;
  while (1)
  {
    if (rt_event_recv(&control_event, (rc_interrupt_event | rc_output_completed_event),
                      (RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR),
                      RT_WAITING_FOREVER, &e) == RT_EOK)
    {
      sbushandler.sbus_decode(&sbushandler);
    }
  }
}

/* Periodic Output Thread */
// #define DEBUG_PERIODIC_OUTPUT
ALIGN(RT_ALIGN_SIZE)
static char periodic_output_stack[512];
static struct rt_thread rc_periodic_output_thread;
static void periodic_output(void *param)
{
  uint8_t channel = 0;
  while (1)
  {
    /*******************************************************************************/
    /*
     * Channel 0 ~ 7 Output Channel Values. Channel 8 ~ 10 Output Channel info.
     *
     * For example : output Channel is 3, value is 128
     *
     *         LSB {            128               }{     3      } MSB
     * Channel     | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
     * Output        0   0   0   0   0   0   0   1   1   1    0
     *
     */
    /*******************************************************************************/

    uint8_t value = sbushandler.rx_data.channel_decoded[channel];
    rt_enter_critical();
    HAL_GPIO_WritePin(RC_channel_8_GPIO_Port, RC_channel_8_Pin, channel % 2);
    HAL_GPIO_WritePin(RC_channel_9_GPIO_Port, RC_channel_9_Pin, (channel >> 1) % 2);
    HAL_GPIO_WritePin(RC_channel_10_GPIO_Port, RC_channel_10_Pin, (channel >> 2) % 2);
    HAL_GPIO_WritePin(RC_channel_0_GPIO_Port, RC_channel_0_Pin, value % 2);
    HAL_GPIO_WritePin(RC_channel_1_GPIO_Port, RC_channel_1_Pin, (value >> 1) % 2);
    HAL_GPIO_WritePin(RC_channel_2_GPIO_Port, RC_channel_2_Pin, (value >> 2) % 2);
    HAL_GPIO_WritePin(RC_channel_3_GPIO_Port, RC_channel_3_Pin, (value >> 3) % 2);
    HAL_GPIO_WritePin(RC_channel_4_GPIO_Port, RC_channel_4_Pin, (value >> 4) % 2);
    HAL_GPIO_WritePin(RC_channel_5_GPIO_Port, RC_channel_5_Pin, (value >> 5) % 2);
    HAL_GPIO_WritePin(RC_channel_6_GPIO_Port, RC_channel_6_Pin, (value >> 6) % 2);
    HAL_GPIO_WritePin(RC_channel_7_GPIO_Port, RC_channel_7_Pin, (value >> 7) % 2);
    rt_exit_critical();
#ifdef DEBUG_PERIODIC_OUTPUT
    rt_kprintf("%d ", value);
#endif
    channel++;
    if (channel >= 8)
    {
      channel = 0;
#ifdef DEBUG_PERIODIC_OUTPUT
      rt_kprintf("\n");
#endif
      rt_event_send(&control_event, rc_output_completed_event);
    }
    rt_thread_mdelay(32);
  }
}

ALIGN(RT_ALIGN_SIZE)
static char emergency_interrupt_stack[512];
static struct rt_thread emergency_interrupt_thread;
static void emergency_process(void *param)
{
  rt_uint32_t e;
  while (1)
  {
    if (rt_event_recv(&control_event,
                      emergency_trigger_event | emergency_clear_event,
                      RT_EVENT_FLAG_OR,
                      RT_WAITING_FOREVER, &e) == RT_EOK)
    {
      if (control_event.set & emergency_trigger_event)
      {
        rt_enter_critical();
        HAL_GPIO_WritePin(Brake_GPIO_Port, Brake_Pin, SET);
        control_event.set &= ~emergency_trigger_event;
        rt_exit_critical();
      }
      else if (control_event.set & emergency_clear_event)
      {
        rt_enter_critical();
        HAL_GPIO_WritePin(Brake_GPIO_Port, Brake_Pin, RESET);
        control_event.set &= ~emergency_clear_event;
        rt_exit_critical();
      }
    }
  }
}

int main(void)
{
  sbushandler.sbus_get_tick = rt_tick_get;
  sbushandler.sbus_raw_fetch = sbus_fetch;
  SBUS_DEFINE(sbus, &sbushandler);
  sbushandler.sbus_raw_fetch(&sbushandler);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

  /* initialize event object */
  rt_err_t result;
  result = rt_event_init(&control_event, "control_event", RT_IPC_FLAG_PRIO);
  if (result != RT_EOK)
  {
    rt_kprintf("init event failed. \n");
    return RT_ERROR;
  }

  /* Initialize Thread */
  /* Decode Thread */
  rt_thread_init(&rc_decode_thread,
                 "RCdecode",
                 rc_decode,
                 RT_NULL,
                 &rc_decode_stack[0],
                 sizeof(rc_decode_stack),
                 DECODE_THREAD_PRIORITY, THREAD_TIMESLICE);
  rt_thread_startup(&rc_decode_thread);

  /* Periodic Output Thread */
  rt_thread_init(&rc_periodic_output_thread,
                 "RCoutput",
                 periodic_output,
                 RT_NULL,
                 &periodic_output_stack[0],
                 sizeof(periodic_output_stack),
                 PERIODIC_OUTPUT_THREAD_PRIORITY, THREAD_TIMESLICE);
  rt_thread_startup(&rc_periodic_output_thread);

  /* Emergency Process */
  rt_thread_init(&emergency_interrupt_thread,
                 "Emergency",
                 emergency_process,
                 RT_NULL,
                 &emergency_interrupt_stack[0],
                 sizeof(emergency_interrupt_stack),
                 EMERGENCY_THREAD_PRIORITY, THREAD_TIMESLICE);
  rt_thread_startup(&emergency_interrupt_thread);
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

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
