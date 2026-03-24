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
#include "pitchesPRE.h"
#include "pitchesDAC.h"
#include "math.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIM_FREQ 84000000
#define ARR_pwm 100
#define size_dac 128
#define PI 3.1415926
#define PSC_dac 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint32_t Ysine[size_dac];

int notes[] = {
    NOTE_A4, NOTE_E5, NOTE_A5, NOTE_A5, NOTE_B5, NOTE_CS6, NOTE_D6, NOTE_E6,
    NOTE_E6, NOTE_E6, NOTE_F6, NOTE_G6, NOTE_A6,
    NOTE_A6, NOTE_A6, NOTE_G6, NOTE_F6, NOTE_G6, NOTE_F6, NOTE_E6,
    NOTE_E6, NOTE_D6, NOTE_D6, NOTE_E6, NOTE_F6,

    NOTE_E6, NOTE_D6, NOTE_C6, NOTE_C6, NOTE_D6, NOTE_E6,
    NOTE_D6, NOTE_C6, NOTE_B5, NOTE_B5, NOTE_C6, NOTE_D6, NOTE_F6, NOTE_E6,
    /*NOTE_A4, NOTE_E5, NOTE_A5, NOTE_A5, NOTE_B5, NOTE_CS6, NOTE_D6, NOTE_E6,
    NOTE_E6, NOTE_E6, NOTE_F6, NOTE_G6, NOTE_A6,*/

    /*NOTE_C6, NOTE_B5, NOTE_G5,
    NOTE_E5, NOTE_F5, NOTE_A5, NOTE_G5, NOTE_E5,
    NOTE_E5, NOTE_F5, NOTE_A5, NOTE_GS5, NOTE_E5,
    NOTE_CS5, NOTE_D5, NOTE_F5, NOTE_E5, NOTE_C5,

    NOTE_A4, NOTE_B4, NOTE_B4, NOTE_C5, NOTE_D5,
    NOTE_F5, NOTE_E5*/
};

int durations[] = {
    170, 500, 170, 170, 170, 170, 170, 500,
    170, 170, 170, 170, 500,
    170, 170, 170, 170, 170, 170, 500,
    170, 170, 170, 170, 340,

    170, 170, 170, 170, 170, 500,
    170, 170, 170, 170, 170, 170, 425, 425,
    /*170, 170, 170, 170, 170, 170, 170, 340,
    170, 170, 170, 170, 380,*/

    /*255, 255, 425,
    190, 190, 190, 190, 380,
    190, 190, 190, 190, 380,
    190, 190, 190, 190, 425,

    190, 190, 190, 190, 380,
    300, 560*/
};

int pauses[] = {
    215, 215, 215, 215, 215, 215, 215, 430,
    215, 215, 215, 215, 430,
    215, 215, 215, 215, 215, 215, 430,
    215, 215, 215, 215, 430,

    215, 215, 215, 215, 215, 430,
    215, 215, 215, 215, 215, 215, 320, 510,
    /*215, 215, 215, 215, 215, 215, 215, 430,
    215, 215, 215, 215, 470,*/

    /*330, 330, 520,
    245, 245, 245, 245, 470,
    245, 245, 245, 245, 470,
    245, 245, 245, 245, 520,

    245, 245, 245, 245, 470,
    390, 690*/
};

int kirbymelody[] = {
    784, 698, 622, 587, 466, 392,
    523, 587, 622, 698, 587, 0, 1047, 0,

    784, 0, 622, 0, 587, 0, 523, 0, 523, 587,
    0, 622, 523, 0, 466, 523, 0, 392, 1047, 0,

    784, 0, 622, 0, 587, 0, 523, 0, 523, 587,
    0, 622, 698, 0, 587, 466, 0, 523, 392, 0,

    523, 0, 1047
};

int kirbydurations[] = {
    320, 160, 160, 160, 160, 320,
    160, 160, 160, 160, 400, 70, 320, 110,

	320, 65, 320, 65, 320, 65, 260, 55, 200, 200,
    55, 200, 200, 55, 200, 200, 55, 260, 380, 110,

    320, 65, 320, 65, 320, 65, 260, 55, 200, 200,
    55, 200, 260, 55, 200, 200, 55, 260, 380, 110,

    290, 65, 620
	};

int kirbypauses[] = {
    1,1,1,1,1,1,
    1,1,1,1,1,200,

    200,200,200,200,200,200,
    200,200,200,200,200,350,

    200,200,200,200,200,200,
    200,200,200,200,200,350,

    250,500
};

int Mariomelody[] = {
    659, 659, 659, 0, 523, 659, 784, 392, 523, 0, 392, 0, 330, 0, 440, 0, 494, 0
};

int MarionoteDurations[] = {
    150, 300, 150, 150, 150, 300, 600, 600, 300, 150, 150, 300, 300, 150, 150, 150, 150, 150
};

uint8_t menu = 1;
volatile uint8_t ctrl_cmd;
volatile uint8_t rx_data;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */
int presForFrequency(int frequency);
void playTone(int *tone, int *duration, int *pause, int size);
void playTonePRE(int *tone, int *duration, int *pause, int size);
void noTone(void);
void generarSin(void);
int calcularARR(int freq);
void playToneDAC(int *tone, int *duration, int *pause, int Nsize);
void noToneDAC(void);
void startToneDAC(int arr_value);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int presForFrequency(int frequency)
{
	if (frequency == 0)
	{
		return 0;
	}
	return ((TIM_FREQ / (ARR_pwm * frequency)) -1 );
}

void playTone(int *tone, int *duration, int *pause, int size)
{
	for (int i = 0; i < size; i++)
	{
		int prescaler = presForFrequency(tone[i]); // Calcular el prescaler
		int dur = duration[i]; // Obtener la duración
		int pauseBetweenTones = 0;
		if (pause != NULL)
		{
			pauseBetweenTones = pause[i] - duration[i];
		}

		__HAL_TIM_SET_PRESCALER(&htim1, prescaler);
		HAL_Delay(dur); // Duración de la nota
		noTone(); // Pausa
		HAL_Delay(pauseBetweenTones); // Duración sin tono
	}
}

void playTonePRE(int *tone, int *duration, int *pause, int size)
{
    for (int i = 0; i < size; i++)
    {
        int prescaler = tone[i];
        int dur = duration[i];
        int pauseBetweenTones = 0;

        if (pause != NULL)
        {
            pauseBetweenTones = pause[i] - duration[i];
            if (pauseBetweenTones < 0)
            {
                pauseBetweenTones = 0;
            }
        }

        __HAL_TIM_SET_PRESCALER(&htim1, prescaler);
        __HAL_TIM_SET_COUNTER(&htim1, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 50);  // activar PWM

        HAL_Delay(dur);

        noTone();  // apagar PWM
        HAL_Delay(pauseBetweenTones);
    }
}

void noTone(void)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
}

void generarSin(void)
{
	for (int x = 0; x < size_dac; x++)
	{
		Ysine[x] = ((sin(x * 2 * PI / size_dac) + 1) * (4096 / 2));
	}
}

int calcularARR(int freq)
{
	if (freq == 0)
		return 0;
	int TF = size_dac * freq;
	return ((TIM_FREQ/((PSC_dac+1)*TF))-1);
}

void playToneDAC(int *tone, int *duration, int *pause, int Nsize)
{
	for (int i=0; i<Nsize; i++)
	{
		int valorARR = calcularARR(tone[i]); // Calcular el valor de ARR
		int dur = duration[i]; // Obtener la duración
		int pauseBetweenNotes = 0;
		if (pause != NULL) pauseBetweenNotes = pause[i] - duration[i];

		TIM6->ARR = valorARR;
		HAL_Delay(dur); // Duración nota
		noTone(); // Pausa
		HAL_Delay(pauseBetweenNotes); // Duración sin tono
	}
}

void noToneDAC(void)
{
	TIM6->ARR = 0; // ARR = 0
}


void startToneDAC(int arr_value)
{
    __HAL_TIM_SET_AUTORELOAD(&htim6, arr_value);
    __HAL_TIM_SET_COUNTER(&htim6, 0);

    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, Ysine, size_dac, DAC_ALIGN_12B_R);
    HAL_TIM_Base_Start(&htim6);
}

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
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_DAC_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_UART_Receive_IT(&huart2, &rx_data, 1);
  noTone();
  noToneDAC();
  generarSin();
  HAL_DAC_Init(&hdac);
  HAL_TIM_Base_Start(&htim6);
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, Ysine, size_dac, DAC_ALIGN_12B_R);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  switch (ctrl_cmd)
	  {
	  case '1':
		  ctrl_cmd = 0;
		  menu = 2;
		  break;

	  case '2':
		  ctrl_cmd = 0;
		  menu = 3;
		  break;
	  }

	  if (menu == 1)
	  {
		  menu = 0;
		  HAL_UART_Transmit(&huart2, (uint8_t*)"Seleccione la melodia que desea escuchar:\r\n", 43, 1000);
		  HAL_UART_Transmit(&huart2, (uint8_t*)"1.- Melodia 1 (PWM)\r\n", 21, 1000);
		  HAL_UART_Transmit(&huart2, (uint8_t*)"2.- Melodia 2 (DAC)\r\n", 21, 1000);
		  HAL_UART_Transmit(&huart2, (uint8_t*)" \r\n", 3, 1000);
	  }
	  else if (menu == 2)
	  {
		  playTonePRE(notes, durations, pauses, (sizeof(notes)/sizeof(notes[0])));
		  noTone();
		  //__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
		  menu = 1;
	  }
	  else if (menu == 3)
	  {
		  playToneDAC(kirbymelody, kirbydurations, NULL, (sizeof(kirbymelody)/sizeof(kirbymelody[0])));
		  noToneDAC();
		  menu = 1;
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
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 50;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 1000-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

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
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
    	ctrl_cmd = rx_data;

        //HAL_UART_Transmit(&huart2, &rx_data, 1, 100);

        HAL_UART_Receive_IT(&huart2, &rx_data, 1);
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
