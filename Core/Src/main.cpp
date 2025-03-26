/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include <cstdio>
#include <iostream>

#include "cmsis_os.h"
#include "fatfs.h"
//#include "lvgl.h"

#include "myfunc.h"
#include "MP3.h"
#include "Bitmap.h"

#include "TFT_LCD.h"
#include "SDcard.h"
#include "VS1003B.h"

using namespace std;

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
ADC_HandleTypeDef hadc1;

SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart1;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* Definitions for ControlTask */
osThreadId_t ControlTaskHandle;
const osThreadAttr_t ControlTask_attributes = { .name = "ControlTask",
		.stack_size = 512 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for AudioTask */
osThreadId_t AudioTaskHandle;
const osThreadAttr_t AudioTask_attributes = { .name = "AudioTask", .stack_size =
		1024 * 4, .priority = (osPriority_t) osPriorityLow, };

osSemaphoreId_t stateSemaphore;
osMessageQueueId_t buttonQueue;

/*class UARTRedirector: public std::streambuf {
protected:
	int overflow(int c) override {
		if (c != EOF) {
			HAL_UART_Transmit(&huart1, (uint8_t*) &c, 1, HAL_MAX_DELAY);
		}
		return c;
	}
};*/
class UARTRedirector : public std::streambuf {
private:
  char buffer[128];
  int pos = 0;
protected:
  int overflow(int c) override {
    if (c != EOF) {
      buffer[pos++] = (char)c;
      if (pos >= sizeof(buffer) || c == '\n') {
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, pos, 10);
        pos = 0;
      }
    }
    return c;
  }
  int sync() override {
    if (pos > 0) {
      HAL_UART_Transmit(&huart1, (uint8_t*)buffer, pos, 10);
      pos = 0;
    }
    return 0;
  }
};

UARTRedirector uart_redirector;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
void StartControlTask(void *argument);
void StartAudioTask(void *argument);

void Home_page();
static void Main_page();
static void init_page();
void UpdateMusicName(char* name);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

TFT_LCD &lcd = TFT_LCD::getInstance();
SDcard &sd = SDcard::getInstance();
VS1003B &vs = VS1003B::getInstance();

int current_page=HOME_PAGE;
bool botton_next = 0;
bool botton_prev = 0;
bool botton_repeat = 0;
bool botton_pause = 0;
bool init=0;
bool is_playing = 0;
bool reset_track = 1;
volatile int numofFile = 0;
int music_no = 0;
uint8_t volume=200;

#define SIZE 100

uint16_t play_emoji[SIZE * SIZE];
uint16_t pause_emoji[SIZE * SIZE];
//uint16_t next_emoji[SIZE * SIZE];
uint16_t prev_emoji[SIZE * SIZE];
uint16_t repeat[3200];
uint16_t repeat1[3200];


extern FATFS fs;           // File system object
extern FIL fil;            // File object
extern FILINFO fno;        // File information object
extern DIR dir;            // Directory object
extern char fileNames[MAX_FILES][MAX_FILENAME];  // 파일 이름 저장 배열
extern volatile UINT fileCount; // 파일 개수

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
	MX_ADC1_Init();
	MX_SPI2_Init();
	MX_USART1_UART_Init();
	//MX_USB_OTG_FS_PCD_Init();
	MX_FATFS_Init();
	std::cout.rdbuf(&uart_redirector);
	/* USER CODE BEGIN 2 */
	//init_page();
	//cout<<"music start\r"<<endl;
	//MP3_Play(0);
	//cout<<"kernel start\r"<<endl;
	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize();

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	stateSemaphore = osSemaphoreNew(1, 1, NULL);
	if (stateSemaphore == NULL) {
		cout<<"Failed to create stateSemaphore\r\n";
	}
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	buttonQueue = osMessageQueueNew(10, sizeof(uint8_t), NULL); // 큐 생성
	if (buttonQueue == NULL) {
		cout<<"Failed to create buttonQueue\r\n";
	}
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of ControlTask */
	ControlTaskHandle = osThreadNew(StartControlTask, NULL,	&ControlTask_attributes);

	/* creation of AudioTask */
	AudioTaskHandle = osThreadNew(StartAudioTask, NULL, &AudioTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

static void init_page() {

	lcd.init();  //8bit
	//lcd.init(&hspi2);
	lcd.color_screen_8(Black);

	//cout << "lcd init\r" << endl;
	//printf("finished/n/r");
	HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XCS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XDCS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_PORT, SD_CS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_PORT, EXT_SD_CS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_PORT, EXT2_SD_CS, GPIO_PIN_SET);

	if (sd.SD_Init(&hspi2))
	{
		std::cout << "sd card failed\r" << std::endl;
		lcd.string(40,150,White,Black,"SD card init failed");
		lcd.string(40,180,White,Black,"Check the SD card");
	}
	else
		std::cout << "sd card success\r" << std::endl;

	sd.FAT_Init();
	vs.vs1003b_init(&hspi2);
	sd.SPI_speed_11MHz();
	numofFile = sd.fatGetDirEntry();
	//cout << "num of File=" << std::dec << numofFile << "\r" << endl;

	//char buffer[20];

	//snprintf(buffer, sizeof(buffer), "%d", numofFile);

	//if(DEBUG) lcd.string(10, 340, Cyan, Black, "num of files:");
	//if(DEBUG) lcd.string(120, 340, Cyan, Black, buffer);


	if (f_mount(&fs, "", 1) == FR_OK)
	{
		std::cout << "Filesystem mounted!\r" << std::endl;
		display_bmp_to_lcd(90,120,"Logo.bmp");
		display_bmp_to_lcd(30,300,"product_name.bmp");

		// MP3 파일 검색
		scanMp3FilesSFN(_T("/"));
		//std::cout<<"num of files:"<<num<<"\r\n";

		// 검색된 파일 정보 출력
		std::cout << "Found " << fileCount << " MP3 files:\r" << std::endl;
		for (uint16_t i = 0; i < fileCount; i++) {

			std::cout << i + 1 << ": " << fileNames[i] << "\r" << std::endl;
		}
		numofFile=fileCount;

		lcd.string(40,390,White,Black,"Resource loading...");
		display_bmp_to_arr("play.bmp",play_emoji);
		//display_bmp_to_arr("next.bmp",next_emoji);
		display_bmp_to_arr("prev.bmp",prev_emoji);
		display_bmp_to_arr("pause.bmp",pause_emoji);
		display_bmp_to_arr("repeat.bmp",repeat);
		display_bmp_to_arr("repeat1.bmp",repeat1);
		//display_bmp_to_arr("start.bmp",start_logo);
		lcd.string(40,390,White,Black,"Resource loading... Done");
		Home_page();
//		cout<<"done\r\n";


	//		// 파일 열고 데이터 전송
	//		for (uint16_t i = 0; i < fileCount; i++) {
	//			char filePath[64];
	//			snprintf(filePath, sizeof(filePath), "/%s", fileNames[i]);
	//			readAndSendFile(filePath);
	//		}
	} else {
		std::cout << "Filesystem mount failed!\r" << std::endl;
	}


}

static void Main_page() {
	lcd.color_screen_8(Background_gray);
	lcd.frame();
	lcd.Rectangle(Rect(0, 0, TFTWIDTH-1, 50), 2, White);
	//lcd.Circle(160,160,10,White);
	lcd.string_size(20, 20, White, button_gray, "Play", 2, 2);
	lcd.string_size(10, 70, White, Background_gray, "Playing", 2, 2);
	lcd.string_size(10, 150, White, Background_gray, "Now:", 2, 2);
	lcd.bitmap(prev_emoji,10,370,100,100);
	lcd.bitmap(play_emoji,110,370,100,100);
	lcd.bitmap_b(prev_emoji,210,370,100,100);
	//lcd.bitmap(next_emoji,210,370,100,100);
	lcd.bitmap(repeat,10,330,80,40);
//	display_bmp_to_lcd( 110, 370, "play.bmp");
//	cout<<"lcd_finish\r\n";

}

void Home_page()
{
	lcd.color_screen_8(Black);
	//lcd.bitmap(start_logo, 60, 190, 200, 100);
	display_bmp_to_lcd(60,190,"start.bmp");
	f_mount(NULL,"",0);
	f_mount(&fs,"",1);


}
void UpdateMusicName(char* name)
{
	lcd.Block(Rect(5,170,310,30), Background_gray, Background_gray);
	lcd.string_size(20, 170, White, Background_gray, name, 1, 1);
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);//3

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 360;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Activate the Over-Drive mode
		 */
		if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
			Error_Handler();
		}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_10;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief SPI2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI2_Init(void) {

	/* USER CODE BEGIN SPI2_Init 0 */
	__HAL_RCC_SPI2_CLK_ENABLE();
	/* USER CODE END SPI2_Init 0 */

	/* USER CODE BEGIN SPI2_Init 1 */

	/* USER CODE END SPI2_Init 1 */
	/* SPI2 parameter configuration*/
	hspi2.Instance = SPI2;
	hspi2.Init.Mode = SPI_MODE_MASTER;
	hspi2.Init.Direction = SPI_DIRECTION_2LINES;
	hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi2.Init.NSS = SPI_NSS_SOFT;
	hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi2.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI2_Init 2 */

	/* USER CODE END SPI2_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

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
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief USB_OTG_FS Initialization Function
 * @param None
 * @retval None
 */
static void MX_USB_OTG_FS_PCD_Init(void) {

	/* USER CODE BEGIN USB_OTG_FS_Init 0 */

	/* USER CODE END USB_OTG_FS_Init 0 */

	/* USER CODE BEGIN USB_OTG_FS_Init 1 */

	/* USER CODE END USB_OTG_FS_Init 1 */
	hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
	hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
	hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
	hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
	hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
	if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USB_OTG_FS_Init 2 */

	/* USER CODE END USB_OTG_FS_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA,
			GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4
					| GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC,
			GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9
					| BOTTON_REPEAT | BOTTON_VOL_P | BOTTON_VOL_M, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB,
	GPIO_PIN_0 | GPIO_PIN_1 | BOTTON_HOME| GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_5,
			GPIO_PIN_RESET);

	HAL_GPIO_WritePin(GPIOB, SCK2 | MISO2 | MOSI2, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XDCS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XCS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SD_PORT, SD_CS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SD_PORT, EXT_SD_CS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SD_PORT, EXT2_SD_CS, GPIO_PIN_RESET);
	/*Configure GPIO pins : PC13 PC11 PC12 */
	GPIO_InitStruct.Pin = BOTTON_REPEAT | BOTTON_VOL_P | BOTTON_VOL_M;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : PB4 */
	GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = BOTTON_HOME;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PA8 PA9 PA10 */
	GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = BOTTON_NEXT | BOTTON_PAUSE | BOTTON_PREV;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PA0 PA1 PA2 PA3
	 PA4 PA5 PA6 PA7 */
	GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
			| GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PC4 PC5 PC6 PC8
	 PC9 */
	GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8
			| GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 PB1 PB10 PB12
	 PB5 */
	GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_10 | GPIO_PIN_12
			| GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = EXT_SD_CS | SD_CS | EXT2_SD_CS;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;    //GPIO_PULLUP
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	/*Configure GPIO pin : PC7 */
	GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = SCK2;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; // Alternate Function Push Pull
	GPIO_InitStruct.Pull = GPIO_NOPULL;    // Pull-Up/Down 없음
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; // 속도 설정
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI2; // Alternate Function 번호
	HAL_GPIO_Init(SPI_PORT, &GPIO_InitStruct);

	GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = MISO2 | MOSI2;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; // Alternate Function Push Pull
	GPIO_InitStruct.Pull = GPIO_NOPULL;    // Pull-Up/Down 없음
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; // 속도 설정
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI2; // Alternate Function 번호
	HAL_GPIO_Init(SPI_PORT, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XDCS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XCS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_PORT, SD_CS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_PORT, EXT_SD_CS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_PORT, EXT2_SD_CS, GPIO_PIN_SET);

	/* EXTI interrupt init*/
	  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
	  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
	  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
	  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

//USER CODE BEGIN 4
void EXTI4_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BOTTON_HOME); //
}
void EXTI8_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BOTTON_NEXT); //
}
void EXTI9_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BOTTON_PAUSE); //
}
void EXTI10_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BOTTON_PREV); //
}
void EXTI11_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BOTTON_VOL_P); //
}
void EXTI12_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BOTTON_VOL_M); //
}
void EXTI13_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(BOTTON_REPEAT); //
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	int event=0;
	if (GPIO_Pin == BOTTON_VOL_P) { // 버튼 핀 확인
		event = EVENT_VOL_P;
	}
	else if (GPIO_Pin == BOTTON_VOL_M) { // 버튼 핀 확인
		event = EVENT_VOL_M;
	}
	else if (GPIO_Pin == BOTTON_REPEAT) { // 버튼 핀 확인
		event = EVENT_REPEAT;
	}
	else if (GPIO_Pin == BOTTON_HOME) { // 버튼 핀 확인
		event = EVENT_HOME;
	}
	else if (GPIO_Pin == BOTTON_PAUSE) { // 버튼 핀 확인
		event = EVENT_PAUSE;
	}
	else if (GPIO_Pin == BOTTON_NEXT) { // 버튼 핀 확인
		event = EVENT_NEXT;
	}
	else if (GPIO_Pin == BOTTON_PREV) { // 버튼 핀 확인
		event = EVENT_PREV;
	}

	if(event!=0)
	{
		xTaskNotifyFromISR((TaskHandle_t)ControlTaskHandle, event, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // 태스크 전환 요청
	}
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartControlTask */
/**
 * @brief  Function implementing the ControlTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartControlTask */
void StartControlTask(void *argument) {
	/* USER CODE BEGIN 5 */
	/* Infinite loop */
	uint8_t event=0;
	//current_page=PLAY_PAGE;
	is_playing = false;
	reset_track = false;
	uint32_t ulNotificationValue;
	for (;;) {
		if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue,	portMAX_DELAY) == pdTRUE)
		{
			std::cout<<"botton event:"<<ulNotificationValue<<"\r"<<endl;
			osSemaphoreAcquire(stateSemaphore, osWaitForever);
			if(init){
				if (ulNotificationValue == EVENT_VOL_P)//volume up , 0 is full
				{
					if (volume < 254) volume = (254 - volume < 10) ? 250 : volume + 10;
					event=EVENT_VOL_P;
					osMessageQueuePut(buttonQueue, &event, 0, 0);
				}
				else if (ulNotificationValue == EVENT_VOL_M)//volume down, 254 is silence
				{
					if (volume > 0) volume = (volume < 10) ? 0 : volume - 10;
					event=EVENT_VOL_M;
					osMessageQueuePut(buttonQueue, &event, 0, 0);
				}

				else if(current_page == HOME_PAGE)
				{
					if (ulNotificationValue == EVENT_PAUSE)
					{
						current_page = PLAY_PAGE;
						is_playing = true;
						reset_track = true;
						Main_page();
					}
				}
				else if(current_page == PLAY_PAGE)
				{
					if (ulNotificationValue == EVENT_REPEAT)
					{ // repeat
						botton_repeat = !botton_repeat;
						if(botton_repeat) lcd.bitmap(repeat1,10,330,80,40);
						else lcd.bitmap(repeat,10,330,80,40);
					}
					if (ulNotificationValue == EVENT_HOME)
					{ // goto homepage
						current_page = HOME_PAGE;
						music_no=0;
						is_playing = false;
						reset_track = true;
						Home_page();
					}
					if (ulNotificationValue == EVENT_PAUSE)
					{ // pause/resume
						is_playing = !is_playing;

						if(is_playing)
						{
							lcd.string_size(10, 70, White, Background_gray, "Playing", 2, 2);
							lcd.bitmap(play_emoji,110,370,100,100);
						}
						else
						{
							lcd.string_size(10, 70, White, Background_gray, "Pause  ", 2, 2);
							lcd.bitmap(pause_emoji,110,370,100,100);
						}
						reset_track = false;
					}
					else if (ulNotificationValue == EVENT_NEXT)
					{ // next
						event=EVENT_NEXT;
						osMessageQueuePut(buttonQueue, &event, 0, 0);
					}
					else if (ulNotificationValue == EVENT_PREV)
					{ // prev
						event=EVENT_PREV;
						osMessageQueuePut(buttonQueue, &event, 0, 0);
					}
				}
			}//end of if
			osSemaphoreRelease(stateSemaphore);
		}
	}
/* USER CODE END 5 */
}
/* USER CODE BEGIN Header_StartAudioTask */
/**
 * @brief Function implementing the AudioTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartAudioTask */
void StartAudioTask(void *argument) {
	/* USER CODE BEGIN StartAudioTask */
	u_long c_clust;
	u_long rd_sec;
	int i, n;
	u_long k = 0, kd;
    FIL fil1;
	init_page();
	osSemaphoreAcquire(stateSemaphore, osWaitForever);
	init=1;
	osSemaphoreRelease(stateSemaphore);
	vs.vs1003b_set_volume(200, 200);
	sd.SPI_speed_6MHz();
	FRESULT res;
	uint8_t button_event;
	bool next = 0;
	bool prev = 0;
	bool vol_change=0;
	bool playing=0;
	int music;
	bool reset;
	bool repeat;
	uint8_t vol;
	UINT bytesRead;       // 읽은 바이트 수
	uint8_t buf[512];  // 데이터 버퍼 (섹터 크기 맞춤)

	/* Infinite loop */
	for (;;) {

		osSemaphoreAcquire(stateSemaphore, osWaitForever);
		playing = is_playing;
		music = music_no;
		reset = reset_track;
		repeat = botton_repeat;
		vol= volume;
		osSemaphoreRelease(stateSemaphore);

		next = 0;
		prev = 0;
		vol_change=0;
		if (osMessageQueueGet(buttonQueue, &button_event, NULL, 0)
				== osOK) {
			if (button_event == EVENT_NEXT)
				next = true;
			else if (button_event == EVENT_PREV)
				prev = true;
			else if((button_event==EVENT_VOL_P)||(button_event==EVENT_VOL_M))
			{
				vol_change=1;
				cout<<"volume change:"<<(int)vol<<"\r"<<endl;
			}
		}

		if (playing && reset) { // 재생 시작 시 초기화
			if(music!=0){
				osSemaphoreAcquire(stateSemaphore, osWaitForever);
				music_no = 0;
				osSemaphoreRelease(stateSemaphore);
			}
			k = 0;
			kd = sd.file_len[0];
			c_clust = sd.fileStartClust[0];
			osSemaphoreAcquire(stateSemaphore, osWaitForever);
			reset_track = false; // 초기화 후 리셋
			osSemaphoreRelease(stateSemaphore);
#ifdef USE_FATFS_LIB
			c_clust=1;
			bytesRead=0;
			//f_close(&fil);
			if (f_open(&fil, fileNames[0], FA_READ) != FR_OK) {
				c_clust=0;
				osSemaphoreAcquire(stateSemaphore, osWaitForever);
				music_no = 1;
				osSemaphoreRelease(stateSemaphore);
				std::cout<<"file open failed: \r\n";
			    //f_mount(NULL, "", 0); // 마운트 해제
			    //return;
			}
			UpdateMusicName(fileNames[music]);
			//lcd.string_size(20, 170, White, Background_gray, fileNames[music], 1, 1);
#endif
		}
		//std::cout<<"playing:"<<playing<<", reset:"<<reset<<", music_no:"<<music<<", clust:"<<c_clust<<", k:"<<k<<", kd:"<<kd<<" vol:"<<(int)vol<<"\r"<<endl;

		if (c_clust == 0 || next || prev) //end of music
		{
			if (prev) {
				if (music == 0) music = numofFile - 1;
				else music--;
				osSemaphoreAcquire(stateSemaphore, osWaitForever);
				botton_prev = 0;
				osSemaphoreRelease(stateSemaphore);
			} else if (next) {
				music++;
				osSemaphoreAcquire(stateSemaphore, osWaitForever);
				botton_next = 0;
				osSemaphoreRelease(stateSemaphore);
			} else	music = repeat ? music : music + 1;

			if (music >= numofFile)
				music = 0;
			osSemaphoreAcquire(stateSemaphore, osWaitForever);
			music_no = music;
			osSemaphoreRelease(stateSemaphore);
			//std::cout<<"num of file:"<<numofFile<<"music:"<<music_no<<"\r\n";
			k = 0;
			kd = sd.file_len[music];                            // 파일 길이 섹터수
			c_clust = sd.fileStartClust[music];               // f_no 파일 시작 클러스터
#ifdef USE_FATFS_LIB
			c_clust=1;
			if(f_close(&fil)!= FR_OK) std::cout<<"file close failed:\r"<<endl;
			//f_close(&fil);
			if (f_open(&fil, fileNames[music], FA_READ) != FR_OK) {
				c_clust=0;
				osSemaphoreAcquire(stateSemaphore, osWaitForever);
				music_no = music+1;
				osSemaphoreRelease(stateSemaphore);
				std::cout<<"file open failed: \r\n";
			    //f_mount(NULL, "", 0); // 마운트 해제
			    //return;
			}
			UpdateMusicName(fileNames[music]);
			//lcd.string_size(20, 170, White, Background_gray, fileNames[music], 1, 1);
#endif
		}

		if (playing) {

#ifdef USE_FATFS_LIB

			if(f_eof(&fil))//end of file
			{
				if(f_close(&fil)!= FR_OK) std::cout<<"file close failed:\r"<<endl;
				f_mount(NULL,"",0);
				f_mount(&fs,"",1);
				music = repeat ? music : music + 1;
				if (music >= numofFile)	music = 0;
				osSemaphoreAcquire(stateSemaphore, osWaitForever);
				music_no = music;
				osSemaphoreRelease(stateSemaphore);

				if (f_open(&fil, fileNames[music], FA_READ) != FR_OK) {
					c_clust=0;
					osSemaphoreAcquire(stateSemaphore, osWaitForever);
					music_no = music+1;
					osSemaphoreRelease(stateSemaphore);
					std::cout<<"file open failed: \r\n";
				}
				UpdateMusicName(fileNames[music]);
				//lcd.string_size(20, 170, White, Background_gray, fileNames[music], 1, 1);

			}
			else
			{
				// read file
				res=f_read(&fil, buf, sizeof(buf), &bytesRead);
				if ( res!= FR_OK) {
					std::cout<<"read failed:"<< res<<'\r'<<endl;
					DWORD pos=f_tell(&fil);
					f_close(&fil);
					f_mount(NULL,"",0);
					f_mount(&fs,"",1);
					if (f_open(&fil, fileNames[music], FA_READ) != FR_OK) {
						std::cout<<"file open failed: \r\n"; }
					f_lseek(&fil,pos);
					f_read(&fil, buf, sizeof(buf), &bytesRead);
				}


				if(bytesRead > 0)// if something is read
				{
					//cout<<"sizeof buf:"<<sizeof(buf)<<", bytesRead: "<<bytesRead<<"\r\n";
					HAL_GPIO_WritePin(SD_PORT, SD_CS, GPIO_PIN_SET);
					// 읽은 데이터 처리 (예: 출력 또는 오디오 재생)
					for (UINT i = 0; i < bytesRead; i++) {
						while ((vs.vs1003b_is_busy()));
						HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XDCS, GPIO_PIN_RESET);
						vs.vs1003b_spi_send(buf[i]);		// VS1033 칩으로 MP3 데이터 전송
						HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XDCS, GPIO_PIN_SET);
					}
					if(vol_change) {
						vs.vs1003b_set_volume(vol,vol);
						vol_change=0;
					}

					HAL_GPIO_WritePin(SD_PORT, SD_CS, GPIO_PIN_RESET);

				}
				else// nothing read, mount again
				{
					DWORD pos=f_tell(&fil);
					f_close(&fil);
					f_mount(NULL,"",0);
					f_mount(&fs,"",1);
					if (f_open(&fil, fileNames[music], FA_READ) != FR_OK) {
						std::cout<<"file open failed: \r\n"; }
					f_lseek(&fil,pos);
				}
			}
#else
			rd_sec = sd.fatClustToSect(c_clust);    // 현재 클러스터를 섹터로 변환
			for (n = 0; n < sd.SecPerClus; n++)    // 클러스터당 섹터 수만큼 읽기
			{
				sd.SD_Read(rd_sec++);			// 섹터(512바이트) 읽기
				for (i = 0; i < 512; i++)
				{				// 읽은 데이터 VS1003칩에 출력
					uint32_t timeout = 1000;
					while (vs.vs1003b_is_busy() && timeout--) {
						osDelay(1);
					}					// VS1033 데이터수신가능 체크
					if (timeout == 0)
						break;
					//while (vs.vs1003b_is_busy());
					HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XDCS, GPIO_PIN_RESET);// xDCS = low, VS1033 활성화
					vs.vs1003b_spi_send(sd.buffer[i]);	// VS1003 칩으로 MP3 데이터 전송
					HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XDCS, GPIO_PIN_SET); // xDCS = high, VS1033 비활성화
					if(vol_change) {
						vs.vs1003b_set_volume(vol,vol);
						vol_change=0;
					}
				}
				if (++k >= kd)
					break;
			}
			//u_long percent=(k/kd)*300;
			//lcd.Slider(0,280,percent,Background_gray,Yellow);
			c_clust = sd.FAT_NextCluster(c_clust);    // 다음 cluster계산
#endif
		}
		//osDelay(1);
	}
/* USER CODE END StartAudioTask */
}
/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
/* USER CODE BEGIN Callback 0 */

/* USER CODE END Callback 0 */
if (htim->Instance == TIM6) {
	HAL_IncTick();
}
/* USER CODE BEGIN Callback 1 */

/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
/* USER CODE BEGIN Error_Handler_Debug */
/* User can add his own implementation to report the HAL error return state */
__disable_irq();
while (1) {
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
