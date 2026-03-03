/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define J1_LED0_Pin GPIO_PIN_0
#define J1_LED0_GPIO_Port GPIOC
#define J1_LED1_Pin GPIO_PIN_1
#define J1_LED1_GPIO_Port GPIOC
#define J1_LED2_Pin GPIO_PIN_2
#define J1_LED2_GPIO_Port GPIOC
#define J1_LED3_Pin GPIO_PIN_3
#define J1_LED3_GPIO_Port GPIOC
#define J1_PB_Pin GPIO_PIN_0
#define J1_PB_GPIO_Port GPIOA
#define J1_PB_EXTI_IRQn EXTI0_IRQn
#define J2_PB_Pin GPIO_PIN_1
#define J2_PB_GPIO_Port GPIOA
#define J2_PB_EXTI_IRQn EXTI1_IRQn
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define Start_PB_Pin GPIO_PIN_4
#define Start_PB_GPIO_Port GPIOA
#define Start_PB_EXTI_IRQn EXTI4_IRQn
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define J2_LED0_Pin GPIO_PIN_4
#define J2_LED0_GPIO_Port GPIOC
#define J2_LED1_Pin GPIO_PIN_5
#define J2_LED1_GPIO_Port GPIOC
#define Disp_a_Pin GPIO_PIN_0
#define Disp_a_GPIO_Port GPIOB
#define Disp_b_Pin GPIO_PIN_1
#define Disp_b_GPIO_Port GPIOB
#define Disp_c_Pin GPIO_PIN_2
#define Disp_c_GPIO_Port GPIOB
#define J2_LED2_Pin GPIO_PIN_6
#define J2_LED2_GPIO_Port GPIOC
#define J2_LED3_Pin GPIO_PIN_7
#define J2_LED3_GPIO_Port GPIOC
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define Disp_e_Pin GPIO_PIN_4
#define Disp_e_GPIO_Port GPIOB
#define Disp_f_Pin GPIO_PIN_5
#define Disp_f_GPIO_Port GPIOB
#define Disp_g_Pin GPIO_PIN_6
#define Disp_g_GPIO_Port GPIOB
#define Disp_pt_Pin GPIO_PIN_7
#define Disp_pt_GPIO_Port GPIOB
#define Disp_d_Pin GPIO_PIN_8
#define Disp_d_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
