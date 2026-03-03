/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdint.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t jugador_1;
uint8_t jugador_2;
uint8_t ganador;
volatile uint8_t pb_start;
volatile uint8_t start;
volatile uint8_t disp_counter;
volatile uint16_t counter;
volatile uint8_t pb_flag;
volatile uint8_t countdown_active = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void juego(uint8_t *jugador); // Función del juego
void show_display(uint8_t counter); // Función del display
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  jugador_1 = 0;
  jugador_2 = 0;
  ganador   = 0;
  pb_flag   = 0;
  counter	= 0;
  start 	= 0;
  pb_start	= 0;
  HAL_TIM_Base_Start_IT(&htim2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (counter == 1000)
	  {
		  disp_counter--;
		  counter = 0;
	  }
	  if (countdown_active == 1 && pb_start == 1)	// Si la bandera de acción de PB5 está encendida entra al if
	  {
		  pb_flag = 0;		// Apaga la bandera de acción de los botones
		  counter = 0;		// Cada vez que entra a la acción del botón 1 reinicia las variables para reiniciar el juego
		  disp_counter = 6;
		  start = 0;
		  ganador = 0;
		  jugador_1 = 0x00;
		  jugador_2 = 0x00;
		  pb_start = 0;
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);	// En PORTB se apagan los leds del jugador 1
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);	// En PORTC se encienden los leds del jugador 2
	  	}
	  else if (start == 1)	// Una vez termina la cuenta regresiva el juego comienza y es posible realizar la lectura de los botones
	  		{
			  if (pb_flag == 2)	// Se activa si PC0 se presiona y entra a la rutina de jugador 1
			  {
				  pb_flag = 0;		// Apaga la bandera de acción de los botones
				  switch(ganador)		// Detecta si hay un ganador antes de realizar la rutina del juego
				  {
				  	  case 0:			// Si todavía no hay ganador entra al juego
				  		  juego(&jugador_1);	// Llama la subrutina del juego
				  		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, (jugador_1 & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
				  		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, (jugador_1 & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
				  		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, (jugador_1 & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
				  		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, (jugador_1 & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);	// En PORTB se encienden los leds del jugador 1

				  		  if (ganador == 1)	// Si el jugador 1 ganó entonces enciende todos los leds de dicho jugador
				  		  {
				  			  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
				  			  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
				  			  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
				  			  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
				  			  show_display(1);
				  		  }
				  	  break;

				  	  case 1:
				  		  start = 0; // Si ya hay ganador esto impide que siga el juego
				  	  break;
				  }
			  }
			  else if (pb_flag == 3)	// Se activa si PC1 se presiona y entra a la rutina de jugador 2
			  {
				  pb_flag = 0;		// Apaga la bandera de acción de los botones
				  switch(ganador)		// Detecta si hay un ganador antes de realizar la rutina del juego
				  {
				  	  case 0:		// Si todavía no hay ganador entra al juego
				  		  juego(&jugador_2);	// Llama la subrutina del juego
				  		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, (jugador_2 & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
				  		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, (jugador_2 & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
				  		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, (jugador_2 & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
				  		  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, (jugador_2 & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);	// En PORTC se encienden los leds del jugador 2

				  		  if (ganador == 1)	// Si el jugador 2 ganó entonces enciende todos los leds de dicho jugador
				  		  {
				  			  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
				  			  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
				  			  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
				  			  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
				  			  show_display(2);
				  		  }
				  	  break;

				  	  case 1:
				  		  start = 0;	// Si el jugador 1 ganó entonces enciende todos los leds de dicho jugador
				  	  break;
							}
						}
					}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, J1_LED0_Pin|J1_LED1_Pin|J1_LED2_Pin|J1_LED3_Pin
                          |J2_LED0_Pin|J2_LED1_Pin|J2_LED2_Pin|J2_LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Disp_a_Pin|Disp_b_Pin|Disp_c_Pin|Disp_e_Pin
                          |Disp_f_Pin|Disp_g_Pin|Disp_pt_Pin|Disp_d_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : J1_LED0_Pin J1_LED1_Pin J1_LED2_Pin J1_LED3_Pin
                           J2_LED0_Pin J2_LED1_Pin J2_LED2_Pin J2_LED3_Pin */
  GPIO_InitStruct.Pin = J1_LED0_Pin|J1_LED1_Pin|J1_LED2_Pin|J1_LED3_Pin
                          |J2_LED0_Pin|J2_LED1_Pin|J2_LED2_Pin|J2_LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : J1_PB_Pin J2_PB_Pin Start_PB_Pin */
  GPIO_InitStruct.Pin = J1_PB_Pin|J2_PB_Pin|Start_PB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Disp_a_Pin Disp_b_Pin Disp_c_Pin Disp_e_Pin
                           Disp_f_Pin Disp_g_Pin Disp_pt_Pin Disp_d_Pin */
  GPIO_InitStruct.Pin = Disp_a_Pin|Disp_b_Pin|Disp_c_Pin|Disp_e_Pin
                          |Disp_f_Pin|Disp_g_Pin|Disp_pt_Pin|Disp_d_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void juego(uint8_t *jugador)
{
	if (*jugador == 0)	// Se utiliza un puntero, si el contador del jugador es 0 entonces suma 1
	{
		*jugador = 1;
	}
	else if (*jugador < 0x04)	// Si el contador del jugador es menor que el máximo de leds entonces corre el bit (contador de décadas)
	{
		*jugador <<= 1;
	}
	else	// Si se superó el valor máximo del contador de décadas entonces se enciende la bandera del ganador
	{
		ganador = 1;
	}
}

void show_display(uint8_t disp_counter)
{
	uint8_t disp_value[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F,0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};	// Listado de valores para display
	uint8_t value = disp_value[disp_counter];	// Se saca el valor de la lista equivalente al contador que ingresa a la función

	HAL_GPIO_WritePin(GPIOB, Disp_a_Pin, (value & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, Disp_b_Pin, (value & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, Disp_c_Pin, (value & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, Disp_d_Pin, (value & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, Disp_e_Pin, (value & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, Disp_f_Pin, (value & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, Disp_g_Pin, (value & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, Disp_pt_Pin, (value & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == J1_PB_Pin)
	{
		pb_flag = 2;
	}
	else if(GPIO_Pin == J2_PB_Pin)
	{
		pb_flag = 3;
	}
	else if(GPIO_Pin == Start_PB_Pin)
	{
		pb_start = 1;
		countdown_active = 1;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
	  if (countdown_active == 1)
	  {
		  counter++;

		  if (disp_counter == 0)	// Una vez el contador llega a cero entonces se enciende la bander de inicio del juego
		  {
			  start = 1;
		  }

		  switch(disp_counter)		// Se hace un switch dependiendo del valor de la bandera de los displays
		  {
		  	  case 5:
		  	  show_display(disp_counter);	// Se muestra el valor de la lista en el display
		  	  break;

		  	  case 4:
		  	  show_display(disp_counter);	// Se muestra el valor de la lista en el display
		  	  break;

		  	  case 3:
		  	  show_display(disp_counter);	// Se muestra el valor de la lista en el display
		  	  break;

		  	  case 2:
		  	  show_display(disp_counter);	// Se muestra el valor de la lista en el display
		  	  break;

		  	  case 1:
		  	  show_display(disp_counter);	// Se muestra el valor de la lista en el display
		  	  break;

		  	  case 0:
		  	  show_display(disp_counter);	// Se muestra el valor de la lista en el display
		  	  pb_start = 0;
		  	  countdown_active = 0;
		  	  start = 1; // Se reinicia el contador
		  	  break;
		  }
	  }
  }

}
/* USER CODE END 4 */

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
