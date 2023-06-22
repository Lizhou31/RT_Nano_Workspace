/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rtthread.h"
#include "RC.h"
  /* USER CODE END Includes */

  /* Exported types ------------------------------------------------------------*/
  /* USER CODE BEGIN ET */

  /* USER CODE END ET */

  /* Exported constants --------------------------------------------------------*/
  /* USER CODE BEGIN EC */

  /* USER CODE END EC */

  /* Exported macro ------------------------------------------------------------*/
  /* USER CODE BEGIN EM */

  /* USER CODE END EM */

  /* Exported functions prototypes ---------------------------------------------*/
  void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Emergency_Rising_Pin GPIO_PIN_4
#define Emergency_Rising_GPIO_Port GPIOE
#define Emergency_Rising_EXTI_IRQn EXTI4_IRQn
#define Emergency_Falling_Pin GPIO_PIN_0
#define Emergency_Falling_GPIO_Port GPIOF
#define Emergency_Falling_EXTI_IRQn EXTI0_IRQn
#define RC_channel_0_Pin GPIO_PIN_5
#define RC_channel_0_GPIO_Port GPIOG
#define RC_channel_1_Pin GPIO_PIN_6
#define RC_channel_1_GPIO_Port GPIOG
#define RC_channel_2_Pin GPIO_PIN_7
#define RC_channel_2_GPIO_Port GPIOG
#define RC_channel_3_Pin GPIO_PIN_8
#define RC_channel_3_GPIO_Port GPIOG
#define RC_channel_4_Pin GPIO_PIN_8
#define RC_channel_4_GPIO_Port GPIOA
#define RC_channel_5_Pin GPIO_PIN_3
#define RC_channel_5_GPIO_Port GPIOD
#define RC_channel_6_Pin GPIO_PIN_4
#define RC_channel_6_GPIO_Port GPIOD
#define RC_channel_7_Pin GPIO_PIN_7
#define RC_channel_7_GPIO_Port GPIOD
#define RC_channel_8_Pin GPIO_PIN_9
#define RC_channel_8_GPIO_Port GPIOG
#define RC_channel_9_Pin GPIO_PIN_10
#define RC_channel_9_GPIO_Port GPIOG
#define RC_channel_10_Pin GPIO_PIN_12
#define RC_channel_10_GPIO_Port GPIOG
#define Brake_Pin GPIO_PIN_15
#define Brake_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */
#define rc_interrupt_event (1 << 2)
#define rc_output_completed_event (1 << 3)
#define emergency_trigger_event (1 << 4)
#define emergency_clear_event (1 << 5)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
