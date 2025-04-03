/*
 * Youn_s_lab_board_info.h
 *
 *  Created on: Mar 1, 2025
 *      Author: spc
 */

#ifndef INC_YOUN_S_LAB_BOARD_INFO_H_
#define INC_YOUN_S_LAB_BOARD_INFO_H_


#ifdef __cplusplus
 extern "C" {

#define DEBUG 0

#endif
//BUTTONS
#define BOTTON_VOL_P GPIO_PIN_11
#define BOTTON_VOL_M GPIO_PIN_12
#define BOTTON_REPEAT GPIO_PIN_13
#define BOTTON_HOME GPIO_PIN_4
#define BOTTON_PAUSE GPIO_PIN_9
#define BOTTON_NEXT GPIO_PIN_10
#define BOTTON_PREV GPIO_PIN_8

//SPI
#define SPI_PORT GPIOB
#define SCK2 GPIO_PIN_13
#define MISO2 GPIO_PIN_14
#define MOSI2 GPIO_PIN_15

//vs1003
#define vs1003_PORT GPIOC
#define SPIF 7
#define VS1003B_XRESET GPIO_PIN_6// PB5
#define VS1003B_DREQ GPIO_PIN_7//PB6
#define VS1003B_XCS GPIO_PIN_9//PE3
#define VS1003B_XDCS GPIO_PIN_8//PF0

//SD card
#define SD_PORT GPIOB
#define SD_CS GPIO_PIN_12
#define EXT_SD_CS GPIO_PIN_10
#define EXT2_SD_CS GPIO_PIN_5
#define SPI_TIMEOUT 100

//TFT LCD

 //chose communication mode
#define TFT_8BIT
//#define TFT_SPI

#ifdef TFT_8BIT
//#include "BUS_control.h
#define TFT_CON_PORT GPIOB
#define TFT_DATA_PORT GPIOA
#define TFT_RW_PORT GPIOC
#define TFT_CS_PORT GPIOB
#define TFT_DC_PORT GPIOB

#define TFT_RD GPIO_PIN_4
#define TFT_WR GPIO_PIN_5
#define TFT_DC GPIO_PIN_1
#define TFT_CS GPIO_PIN_0

#define TFT_D0 GPIO_PIN_0
#define TFT_D1 GPIO_PIN_1
#define TFT_D2 GPIO_PIN_2
#define TFT_D3 GPIO_PIN_3
#define TFT_D4 GPIO_PIN_4
#define TFT_D5 GPIO_PIN_5
#define TFT_D6 GPIO_PIN_6
#define TFT_D7 GPIO_PIN_7
//#define TFT_RST 21

#elif defined(TFT_SPI)
//#define TFT_RST 21
//#define TFT_CS 22 /// tft_lcd chip select
//#define TFT_DC 20
#define TFT_PORT GPIOB
#define TFT_MISO GPIO_PIN_14
#define TFT_MOSI GPIO_PIN_15
#define TFT_SCL GPIO_PIN_13
#define TFT_CS_PORT GPIOB
#define TFT_DC_PORT GPIOB
#define TFT_DC GPIO_PIN_1
#define TFT_CS GPIO_PIN_0

enum SPI_Prescaler{
	SPI_PRESCALER_2,
	SPI_PRESCALER_4,
	SPI_PRESCALER_8,
	SPI_PRESCALER_16,
	SPI_PRESCALER_32,
	SPI_PRESCALER_64,
	SPI_PRESCALER_128,
	SPI_PRESCALER_256,
};
#endif


#ifdef __cplusplus
}
#endif

#endif /* INC_YOUN_S_LAB_BOARD_INFO_H_ */
