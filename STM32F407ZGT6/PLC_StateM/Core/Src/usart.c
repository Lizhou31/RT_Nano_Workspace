/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#define UART_RX_BUF_LEN 16
rt_uint8_t uart_rx_buf[UART_RX_BUF_LEN] = {0};
struct rt_ringbuffer uart_rxcb;
static struct rt_semaphore shell_rx_sem;
/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* initialize ringbuffer */
  rt_ringbuffer_init(&uart_rxcb, uart_rx_buf, UART_RX_BUF_LEN);

  /* initialize rx semaphore*/
  rt_sem_init(&(shell_rx_sem), "shell_rx", 0, 0);

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE); //启动串口中断
  /* USER CODE END USART1_Init 2 */
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (uartHandle->Instance == USART1)
  {
    /* USER CODE BEGIN USART1_MspInit 0 */

    /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    /* USER CODE BEGIN USART1_MspInit 1 */

    /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{

  if (uartHandle->Instance == USART1)
  {
    /* USER CODE BEGIN USART1_MspDeInit 0 */

    /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    /* USER CODE BEGIN USART1_MspDeInit 1 */

    /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
INIT_BOARD_EXPORT(MX_USART1_UART_Init);

#ifdef RT_USING_CONSOLE
void rt_hw_console_output(const char *str)
{
  rt_size_t i = 0, size = 0;
  char a = '\r';

  // __HAL_UNLOCK(&huart1);

  size = rt_strlen(str);

  for (i = 0; i < size; i++)
  {
    if (*(str + i) == '\n')
    {
      HAL_UART_Transmit(&huart1, (uint8_t *)&a, 1, 1);
    }
    HAL_UART_Transmit(&huart1, (uint8_t *)(str + i), 1, 1);
  }
}
#endif

#ifdef RT_USING_FINSH
char rt_hw_console_getchar(void)
{
  /* Note: the initial value of ch must < 0 */
  char ch = -1;

  /* 从 ringbuffer 中拿出数据 */
  while (rt_ringbuffer_getchar(&uart_rxcb, (rt_uint8_t *)&ch) != 1)
  {
    rt_sem_take(&shell_rx_sem, RT_WAITING_FOREVER);
  }
  return ch;
}

void USART1_IRQHandler(void)
{
  /* Note: the initial value of ch must < 0 */
  int ch = -1;
  /* enter interrupt */
  rt_interrupt_enter(); //在中断中一定要调用这对函数，进入中断

  if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET) &&
      (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE) != RESET))
  {
    while (1)
    {
      ch = -1;
      if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
      {
        ch = huart1.Instance->DR & 0xff;
      }
      if (ch == -1)
      {
        break;
      }
      /* 读取到数据，将数据存入 ringbuffer */
      rt_ringbuffer_putchar(&uart_rxcb, ch);
    }
    rt_sem_release(&shell_rx_sem);
  }

  /* leave interrupt */
  rt_interrupt_leave(); //在中断中一定要调用这对函数，离开中断
}
#endif
/* USER CODE END 1 */
