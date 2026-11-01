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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LENGTH 10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


uint8_t FlowingLight_run_flag = 0;
uint16_t count = 0; //time counts for flowing_light running.
uint8_t timer_en = 0; //systimer
uint8_t key_flag = 0x00;
uint8_t LightSensor_flag= 0;
uint8_t RxBuff[LENGTH] = {0};
uint8_t RxCount = 0;
uint8_t LightOn_Flag = 0;
uint8_t cmd_ready_flag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	uint8_t ch = 'F';
	char msg[]="Hello,world!\r\n";
	uint8_t recvBuf[2];
	
	printf("已开启接收中断,等待数据\r\n");
	HAL_UART_Receive_IT(&huart1 ,RxBuff, 1);
//	HAL_UART_Receive_IT(&huart1 ,RxBuff,LENGTH);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//press key1,turn on FlowingLight
//		if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_SET)
//		{
//			FlowingLight_run_flag = 1;
//			count = 0;
//			timer_en = 1;
//		}
//		else if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_SET)
//		{
//			FlowingLight_run_flag = 0;
//			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_G_Pin | LED_R_Pin | LED_B_Pin, GPIO_PIN_SET); //all is off
//			timer_en = 0;
//		}
		
//		HAL_UART_Transmit(&huart1, &ch, 1, HAL_MAX_DELAY); //send a byte.
//		HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY); 
//		printf("你好呀！今天天气真好！\r\n");
//		
//		if(HAL_UART_Receive(&huart1 ,recvBuf,2,10) == HAL_OK){
//				printf("byte0=%c, byte1=%d\r\n", recvBuf[0], recvBuf[1]);//'2''3' byte0=2, byte1=51(ASCII) 23
//				HAL_UART_Transmit(&huart1, recvBuf, 2, HAL_MAX_DELAY); 
//		}

	if(cmd_ready_flag == 1)
	{
		cmd_ready_flag = 0;
		if(strcmp((char*)RxBuff, "on") == 0)
		{
			LightOn_Flag = 1;
			printf("收到on，开灯\r\n");
		}
		else if(strcmp((char*)RxBuff, "off") == 0)
		{
			LightOn_Flag = 0;
			printf("收到off，关灯\r\n");
		}
		memset(RxBuff, 0, sizeof(RxBuff));
	}
		
		if(key_flag & (1 << 1) )
		{
			HAL_Delay(5); 
			if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_SET )//eliminate shaking
			{
				FlowingLight_run_flag = 0;
				HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_G_Pin | LED_R_Pin | LED_B_Pin, GPIO_PIN_SET); //all is off
				timer_en = 0;
				LightOn_Flag = 0;
			}
			key_flag &= ~(1 << 1);
		}
		
		if(LightOn_Flag == 1) 
		{
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_G_Pin | LED_R_Pin | LED_B_Pin, GPIO_PIN_RESET);
		}
		
		if(FlowingLight_run_flag == 1 && LightOn_Flag == 0) 
		{
				//GPIO out,flowing water lights
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET); //red light on
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_G_Pin | LED_B_Pin, GPIO_PIN_SET); //green and blue off
			HAL_Delay(100); //delay 100ms
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin | LED_B_Pin, GPIO_PIN_SET);
			HAL_Delay(100);
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_G_Pin | LED_R_Pin, GPIO_PIN_SET);
			HAL_Delay(100);
		}
		
		if(LightSensor_flag == 1){
			if(HAL_GPIO_ReadPin(LightSensor_D0_GPIO_Port, LightSensor_D0_Pin) == GPIO_PIN_RESET)
			{
				HAL_GPIO_WritePin(LED_PB3_GPIO_Port, LED_PB3_Pin, GPIO_PIN_RESET);
			}
			else if(HAL_GPIO_ReadPin(LightSensor_D0_GPIO_Port, LightSensor_D0_Pin) == GPIO_PIN_SET)
			{
				HAL_GPIO_WritePin(LED_PB3_GPIO_Port, LED_PB3_Pin, GPIO_PIN_SET);
			}	
			LightSensor_flag = 0;
		}
		
//		if(HAL_GPIO_ReadPin(LightSensor_D0_GPIO_Port, LightSensor_D0_Pin) == GPIO_PIN_RESET)
//		{
//			HAL_GPIO_WritePin(LED_PB3_GPIO_Port, LED_PB3_Pin, GPIO_PIN_RESET);
//		}
//		else if(HAL_GPIO_ReadPin(LightSensor_D0_GPIO_Port, LightSensor_D0_Pin) == GPIO_PIN_SET)
//		{
//			HAL_GPIO_WritePin(LED_PB3_GPIO_Port, LED_PB3_Pin, GPIO_PIN_SET);
//		}
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
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == KEY2_Pin){
			key_flag |= (1 << 1); // 0000 0010
	}
	else if(GPIO_Pin == KEY1_Pin){
			FlowingLight_run_flag = 1;
			count = 0;
			timer_en = 1;
	}
	else if(GPIO_Pin == LightSensor_D0_Pin){
		LightSensor_flag = 1;
		HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
	}
	
}

int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	
	if(huart -> Instance == USART1){
//		RxCount++;
		if(RxBuff[RxCount] == '\r')
        {
            HAL_UART_Receive_IT(&huart1, &RxBuff[RxCount], 1);
            return;
        }
        if(RxBuff[RxCount] == '\n')
        {
            RxBuff[RxCount] = '\0';
            cmd_ready_flag = 1;
            RxCount = 0;
        }
        else
        {
            RxCount++;
            if(RxCount >= LENGTH)
            {
                memset(RxBuff,0,sizeof(RxBuff));
                RxCount = 0;
            }
        }
//		if(RxCount == 2 && strncmp((char *)RxBuff, "on", 2) == 0){
//			LightOn_Flag = 1;
//			memset(RxBuff, 0 ,sizeof(RxBuff));
//			RxCount = 0;
//		}
//		else if(RxCount == 3 && strncmp((char *)RxBuff, "off", 3) == 0){
//			LightOn_Flag = 0;
//			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_G_Pin | LED_R_Pin | LED_B_Pin, GPIO_PIN_SET);
//			memset(RxBuff, 0 ,sizeof(RxBuff));
//			RxCount = 0;
//		}
		HAL_UART_Receive_IT(&huart1, &RxBuff[RxCount], 1);
////		printf("已接收到数据。数据回显如下：\r\n");
//		HAL_UART_Transmit_IT(&huart1 ,RxBuff,LENGTH);
////		printf("\r\n再次开启接收中断.\r\n");
//		HAL_UART_Receive_IT(&huart1 ,RxBuff,LENGTH);
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
