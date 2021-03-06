/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct{
	uint8_t   reportID;
	uint8_t   buttonMask;
	int8_t   	dx;
	int8_t   	dy;
	int8_t   	dWheel;
} MouseReport_t;

typedef struct{
	uint8_t   reportID;
	uint8_t   modifier;
	uint8_t   _RESERVED;
	uint8_t   byte1;
	uint8_t   byte2;
	//uint8_t 	backupKeys[3]; // In case of need, Plese modify the ReportDescriptor & Max_EPIN_Size accordingly. 
} KeyboardReport_t;

typedef struct
{
  uint8_t  reportId;                                 // Report ID = 0x03 (3)
                                                     // Collection: CA:TouchScreen CL:Finger
  uint8_t  DIG_TouchScreenFingerTipSwitch : 1;       // Usage 0x000D0042: Tip Switch, Value = 0 to 1
  uint8_t  DIG_TouchScreenFingerInRange : 1;         // Usage 0x000D0032: In Range, Value = 0 to 1
  uint8_t  DIG_TouchScreenFingerConfidence : 1;      // Usage 0x000D0047: Confidence, Value = 0 to 1
  uint8_t  : 1;                                      // Pad
  uint8_t  : 1;                                      // Pad
  uint8_t  : 1;                                      // Pad
  uint8_t  : 1;                                      // Pad
  uint8_t  : 1;                                      // Pad
  uint8_t  DIG_TouchScreenFingerContactIdentifier;   // Usage 0x000D0051: Contact Identifier, Value = 0 to 1
  uint8_t  GD_TouchScreenFingerX;                    // Usage 0x00010030: X, Value = 0 to 4213
  uint8_t  GD_TouchScreenFingerY;                    // Usage 0x00010031: Y, Value = 0 to 4213
  uint8_t  DIG_TouchScreenFingerTipSwitch_1 : 1;     // Usage 0x000D0042: Tip Switch, Value = 0 to 1
  uint8_t  DIG_TouchScreenFingerInRange_1 : 1;       // Usage 0x000D0032: In Range, Value = 0 to 1
  uint8_t  DIG_TouchScreenFingerConfidence_1 : 1;    // Usage 0x000D0047: Confidence, Value = 0 to 1
  uint8_t  : 1;                                      // Pad
  uint8_t  : 1;                                      // Pad
  uint8_t  : 1;                                      // Pad
  uint8_t  : 1;                                      // Pad
  uint8_t  : 1;                                      // Pad
  uint8_t  DIG_TouchScreenFingerContactIdentifier_1; // Usage 0x000D0051: Contact Identifier, Value = 0 to 1
  uint8_t  GD_TouchScreenFingerX_1;                  // Usage 0x00010030: X, Value = 0 to 4213
  uint8_t  GD_TouchScreenFingerY_1;                  // Usage 0x00010031: Y, Value = 0 to 4213
                                                     // Collection: CA:TouchScreen
  uint8_t  DIG_TouchScreenContactCount;              // Usage 0x000D0054: Contact Count, Value = 0 to 8
} TouchScreenReport_t;

typedef union {
	KeyboardReport_t keyboardReport;	// 5 Bytes
	MouseReport_t mouseReport;				// 5 Bytes
} ReportUnion;

typedef struct {
	uint8_t UniqueID;		// 1 Byte	| 1 Byte Recieve
	ReportUnion Report;	// 5 Byte	| 3 Byte Recieve - ReportID + Byte1 + Byte2
} UART_Key_t;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FLASH_TIMEOUT		1000
#define FLASH_UNLOCK_KEY1	0x45670123
#define	FLASH_UNLOCK_KEY2	0xCDEF89AB
#define NUMBER_OF_KEYS 8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */
ReportUnion KeyList[NUMBER_OF_KEYS]; // The size: # of the keys

/*

KeyList[$UniqueID] = {
	<ReportUnion> Report -> each is writen according to ReportID -> ReportUnioun Report.keyboardReport = incommingReport
};

*/
uint8_t UART2_rxBuffer[4];
MouseReport_t mouse_report;
KeyboardReport_t keyboard_report;
TouchScreenReport_t touch_screen_report;

extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint8_t USBD_CUSTOM_HID_SendReport(USBD_HandleTypeDef  *pdev,
                                   uint8_t *report,
                                   uint16_t len);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
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
	mouse_report.reportID 	= 0x02;
	mouse_report.dx 				= 100;
	mouse_report.dy 				= 0;
	mouse_report.dWheel 		= 0;
	mouse_report.buttonMask = 0;
	
	keyboard_report.reportID		= 0x01;
	
	touch_screen_report.reportId = 0x03;
	touch_screen_report.DIG_TouchScreenFingerTipSwitch 		= 1;
	touch_screen_report.DIG_TouchScreenFingerInRange 			= 1;
	touch_screen_report.DIG_TouchScreenFingerConfidence 	= 1;
	touch_screen_report.GD_TouchScreenFingerX 						= (uint16_t) 100;
	touch_screen_report.GD_TouchScreenFingerY 						= (uint16_t) 100;
	touch_screen_report.DIG_TouchScreenFingerContactIdentifier = 0;
	touch_screen_report.DIG_TouchScreenContactCount = 1;


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
  MX_USB_DEVICE_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

	HAL_UART_Receive_DMA(&huart2, UART2_rxBuffer, 4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *) &mouse_report, sizeof(MouseReport_t));
		HAL_Delay(500);

		keyboard_report.modifier	= 0;
		keyboard_report.byte1 		= 0x04;
		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *) &keyboard_report, sizeof(KeyboardReport_t));
		HAL_Delay(500);
		
		keyboard_report.modifier	= 0;
		keyboard_report.byte1 		= 0x00;
		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *) &keyboard_report, sizeof(KeyboardReport_t));
		HAL_Delay(500);

		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *) &touch_screen_report, sizeof(TouchScreenReport_t));
		HAL_Delay(500);

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
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
  /** Initializes the CPU, AHB and APB busses clocks 
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  huart2.Init.BaudRate = 9600;
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
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
		if(UART2_rxBuffer[1] == 0x01){			// Unique ID UART2_rxBuffer[0]
			
			KeyList[UART2_rxBuffer[0]].keyboardReport.reportID = 0x01;
			KeyList[UART2_rxBuffer[0]].keyboardReport.modifier = UART2_rxBuffer[2];
			KeyList[UART2_rxBuffer[0]].keyboardReport.byte1 = UART2_rxBuffer[3];
			
		} else if(UART2_rxBuffer[1] == 0x02){
			
			KeyList[UART2_rxBuffer[0]].mouseReport.reportID = 0x02;
			KeyList[UART2_rxBuffer[0]].mouseReport.buttonMask = UART2_rxBuffer[2];
			
		}
		
    HAL_UART_Transmit(&huart2, UART2_rxBuffer, 4, 100);
    HAL_UART_Receive_DMA(&huart2, UART2_rxBuffer, 4);
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
