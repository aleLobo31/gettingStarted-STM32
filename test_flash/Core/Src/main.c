/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct Sensors_calibration_t {
  uint32_t steering_correction_factor;
  uint32_t damper_left_correction_factor;
  uint32_t damper_right_correction_factor;
  uint16_t min_steering;
  uint16_t max_steering;
  uint16_t min_damper_left;
  uint16_t max_damper_left;
  uint16_t min_damper_right;
  uint16_t max_damper_right;

} Sensors_calibration_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FLASH_LAST_PAGE 0x0800FFFF
#define FLASH_PAGE_63 0x08007C00
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t bool = 1;

Sensors_calibration_t sensors_calibration_write = {
		.min_steering = 0x0002,
		.max_steering = 0x0005,
		.steering_correction_factor = 0x00000001,
		.min_damper_left = 0x0002,
		.max_damper_left = 0x0005,
		.damper_left_correction_factor = 0x00000001,
		.min_damper_right = 0x0002,
		.max_damper_right = 0x0005,
		.damper_right_correction_factor = 0x00000001
};

Sensors_calibration_t sensors_calibration_read = {
		.min_steering = 0,
		.max_steering = 0,
		.steering_correction_factor = 0,
		.min_damper_left = 0,
		.max_damper_left = 0,
		.damper_left_correction_factor = 0,
		.min_damper_right = 0,
		.max_damper_right = 0,
		.damper_right_correction_factor = 0
};


static uint32_t GetPage(uint32_t Address)
{
 for (int indx=0; indx<64; indx++)
 {
     if((Address < (0x08000000 + (1024 *(indx+1))) ) && (Address >= (0x08000000 + 1024*indx)))
     {
         return (0x08000000 + 1024*indx);
     }
 }
 return 0;
}


uint32_t store_flash_memory(uint32_t memory_address, Sensors_calibration_t*data, int number_words)
{
	//We declare some useful variables
	static FLASH_EraseInitTypeDef Erase_Init_Struct;
	uint32_t PAGEError;

	//We unlock the memory in order to write
	HAL_FLASH_Unlock();

	//Erase the USER FLASH area
	uint32_t start_page = GetPage(memory_address); //0x0800FFF
	uint32_t end_page_address = memory_address + number_words*4;
	uint32_t end_page = GetPage(end_page_address); //0x0800FFE7

	/*
	if(start_page == 0 || end_page == 0)
	{
		return 1;
	}
	*/

	Erase_Init_Struct.NbPages = ((end_page - start_page)/FLASH_PAGE_SIZE) + 1;
	Erase_Init_Struct.PageAddress = start_page;
	Erase_Init_Struct.TypeErase = FLASH_TYPEERASE_PAGES;

	if(HAL_FLASHEx_Erase(&Erase_Init_Struct, &PAGEError) != HAL_OK)
	{
		return 1; //We return if there is an error while erasing
	}

	//We create some useful pointers to access struct data
	uint32_t * px = (uint32_t *)data;
	uint16_t * gx = (uint16_t *)data;

	//We write data into FLASH
	for(int n=0; n<3; n++)
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, memory_address, *(px + n)) != HAL_OK)
			{
				return 1;
			}
		memory_address += 4;
	}



	for(int j=6; j<12; j++)
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, memory_address, *(gx + j)) != HAL_OK)
		{
			return 1;
		}
		memory_address += 2;
	}

	//We lock the memory to prevent writing
	HAL_FLASH_Lock();

	return 0;
};

uint32_t read_flash_memory(uint32_t memory_address, Sensors_calibration_t*data, int number_words)
{
	uint32_t * px = (uint32_t *)data;
	uint16_t * gx = (uint16_t *)data;

	for(int n=0; n<3; n++)
	{
		*(px+n) = *(__IO uint32_t *)memory_address;
		memory_address += 4;

	}

	for(int j = 6; j < 12; j++)
	{
		*(gx+j) = *(__IO uint16_t *)memory_address;
		memory_address += 2;
	}
	return 0;
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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(bool != 0)
	  {
		  bool = 0;

		  if(store_flash_memory(FLASH_PAGE_63, &sensors_calibration_write, 6) == 0)
			{
			  HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_RESET);

			  HAL_Delay(1000);

			  if(read_flash_memory(FLASH_PAGE_63, &sensors_calibration_read, 6) == 0)
			  {
				  HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_SET);
			  }
			}
	  } else
	  {
		HAL_GPIO_TogglePin(GPIOB, LED_Pin);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
