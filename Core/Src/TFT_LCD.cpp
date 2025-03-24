/*
 * TFT_LCD.cpp
 *
 *  Created on: Dec 5, 2024
 *      Author: spc
 */

#include "TFT_LCD.h"
#include "myfunc.h"
#include <iostream>
#include "TFT_font.h"
#include "Youn_s_lab_board_info.h"

unsigned char ScreenMode = 'P';         // screen mode(P=portrait, L=landscape)
unsigned char LineLimit = 40;           // character length of line (30 or 40)
unsigned char KoreanBuffer[32] = { 0 };   // 32 byte Korean font buffer
unsigned int xcharacter, ycharacter; // xcharacter(0-29), ycharacter(0-39) for portrait
									 // xcharacter(0-39), ycharacter(0-29) for landscape
unsigned int foreground, background;    // foreground and background color
unsigned char cursor_flag = 0;          // 0 = cursor off, 1 = cursor on
unsigned char xcursor = 0, ycursor = 0; // cursor position
unsigned int cursor = 0;                // cursor color
unsigned char outline_flag = 0;         // 0 = outline off, 1 = outline on
unsigned int outline = 0;               // outline color

TFT_LCD::TFT_LCD() {
}
TFT_LCD::~TFT_LCD() {
}
#ifdef TFT_SPI
//***********************************************************************************
// TFT_SPI
inline void TFT_LCD::cs_select() {
	HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS, GPIO_PIN_RESET);
}

inline void TFT_LCD::cs_deselect() {
	HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS, GPIO_PIN_SET);

}

inline void TFT_LCD::DC_DATA() {
	HAL_GPIO_WritePin(TFT_CS_PORT, TFT_DC, GPIO_PIN_SET);
}

inline void TFT_LCD::DC_COMMAND() {
	HAL_GPIO_WritePin(TFT_CS_PORT, TFT_DC, GPIO_PIN_RESET);
}

inline void TFT_LCD::spi_transfer_byte(uint8_t byte) {
	HAL_SPI_Transmit(m_spi, &byte, 1, 0xffff);
}

inline void TFT_LCD::spi_transfer_word(uint16_t word) {
	uint8_t data[2];
	data[0] = word >> 8;
	data[1] = word & 0xff;
	HAL_SPI_Transmit(m_spi, data, 2, 0xffff);
}

inline void TFT_LCD::write8(uint8_t data) {
	spi_transfer_byte(data);
}
inline void TFT_LCD::write16(uint16_t data) {
	spi_transfer_word(data);
}

void TFT_LCD::sendWord(uint8_t IR, uint16_t data) {
	cs_select();
	DC_COMMAND();
	spi_transfer_byte(IR);
	DC_DATA();
	spi_transfer_word(data);
	cs_deselect();
}
void TFT_LCD::sendByte(uint8_t IR, uint8_t data) {
	cs_select();
	DC_COMMAND();
	spi_transfer_byte(IR);
	DC_DATA();
	spi_transfer_byte(data);
	cs_deselect();
}

void TFT_LCD::sendcommand(uint8_t IR) {
	cs_select();
	DC_COMMAND();
	spi_transfer_byte(IR);
	cs_deselect();
}
void TFT_LCD::sendDataByte(uint8_t data) {
	cs_select();
	DC_DATA();
	spi_transfer_byte(data);
	cs_deselect();
}
void TFT_LCD::sendDataWord(uint16_t data) {
	cs_select();
	DC_DATA();
	spi_transfer_word(data);
	cs_deselect();
}

//*****************------TFT initial-------------------------------
void TFT_LCD::init(SPI_HandleTypeDef *spi) /* initialize TFT-LCD with HX8347 */
{

	m_spi = spi;
	reg_init();
	setRotation(0);
}

void ChangeSPIspeed(SPI_HandleTypeDef *hspi, uint32_t newPrescaler) {
	// SPI stop
	HAL_SPI_DeInit(hspi);

	// BaudRatePrescaler change
	hspi->Init.BaudRatePrescaler = newPrescaler;

	// SPI init
	if (HAL_SPI_Init(hspi) != HAL_OK) {
		// error
		//Error_Handler();
	}
}
//
//void TFT_LCD::imshow(Array<uint8_t> arr, uint8_t res)
//{
//  int width = 0;
//  int height = 0;
//  switch (res)
//  {
//  case resolution::QVGA:
//    width=QVGA_width;
//    height=QVGA_height;
//    break;
//  case resolution::QQVGA:
//    width=QQVGA_width;
//    height=QQVGA_height;
//    break;
//  case resolution::CIF:
//    width=CIF_width;
//    height=CIF_height;
//    break;
//  case resolution::QCIF:
//    width=QCIF_width;
//    height=QCIF_height;
//    break;
//
//  }
//  uint8_t *buf = arr.getbuf();
//  int t_width=width*2;
//  setAddrWindow(0, 0, width, height);
//  cs_select();
//  DC_DATA();
//  for (int j = 0; j < height; j++)
//  {
//    for (int i = 0; i <t_width; i++)
//    {
//      spi_transfer_byte(buf[i+j*t_width]);
//    }
//  }
//  cs_deselect();
//}
//
//void TFT_LCD::imshow(uint8_t* arr, uint8_t res)
//{
//  int width = 0;
//  int height = 0;
//  switch (res)
//  {
//  case resolution::QVGA:
//    width=QVGA_width;
//    height=QVGA_height;
//    break;
//  case resolution::QQVGA:
//    width=QQVGA_width;
//    height=QQVGA_height;
//    break;
//  case resolution::CIF:
//    width=CIF_width;
//    height=CIF_height;
//    break;
//  case resolution::QCIF:
//    width=QCIF_width;
//    height=QCIF_height;
//    break;
//
//  }
//
//  int t_width=width*2;
//  setAddrWindow(0, 0, width, height);
//  cs_select();
//  DC_DATA();
//  for (int j = 0; j < height; j++)
//  {
//    for (int i = 0; i <t_width; i++)
//    {
//      spi_transfer_byte(arr[i+j*t_width]);
//    }
//  }
//  cs_deselect();
//}

void TFT_LCD::imshow(uint8_t *arr, int width, int height) {
	int total = width * 2 * height;
	//setAddrWindow(0, 0, width, height);
	sendcommand(HX8357_RAMWR);
	cs_select();
	DC_DATA();
	for (int j = 0; j < total; j++) {

		spi_transfer_byte(arr[j]);

	}
	cs_deselect();
}

void TFT_LCD::imshow(uint8_t *arr, int x, int y, int width, int height) {
	int total = width * 2 * height;
	setAddrWindow(x, y, width, height);
	sendcommand(HX8357_RAMWR);
	cs_select();
	DC_DATA();
	for (int j = 0; j < total; j++) {

		spi_transfer_byte(arr[j]);

	}
	cs_deselect();
}

void TFT_LCD::color_screen(uint16_t color) /* TFT-LCD full screen color */
{
	GRAM_address(TFTWIDTH, TFTHEIGHT);
	cs_select();
	DC_DATA();
	for (int j = 0; j < TFTHEIGHT; j++) {
		for (int i = 0; i < TFTWIDTH; i++) {
			spi_transfer_byte(color >> 8);
			spi_transfer_byte(color & 0xff);
		}
	}
	cs_deselect();
}

#elif defined(TFT_8BIT)
//***********************************************************************************
// TFT_8BIT
inline void TFT_LCD::cs_select() {
	HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS, GPIO_PIN_RESET);
}

inline void TFT_LCD::cs_deselect() {
	HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS, GPIO_PIN_SET);

}

inline void TFT_LCD::DC_DATA() {
	HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC, GPIO_PIN_SET);
}

inline void TFT_LCD::DC_COMMAND() {
	HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC, GPIO_PIN_RESET);
}

inline void TFT_LCD::WR_LOW() {
	HAL_GPIO_WritePin(TFT_RW_PORT, TFT_WR, GPIO_PIN_RESET);
}

inline void TFT_LCD::WR_HIGH() {
	HAL_GPIO_WritePin(TFT_RW_PORT, TFT_WR, GPIO_PIN_SET);
}
inline void TFT_LCD::RD_LOW() {
	HAL_GPIO_WritePin(TFT_RW_PORT, TFT_RD, GPIO_PIN_RESET);
}

inline void TFT_LCD::RD_HIGH() {
	HAL_GPIO_WritePin(TFT_RW_PORT, TFT_RD, GPIO_PIN_SET);
}

inline void TFT_LCD::write8(uint8_t data) {
	HAL_GPIO_WritePin(TFT_DATA_PORT, TFT_D0,
			(data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET); // Bit 0
	HAL_GPIO_WritePin(TFT_DATA_PORT, TFT_D1,
			(data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET); // Bit 1
	HAL_GPIO_WritePin(TFT_DATA_PORT, TFT_D2,
			(data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET); // Bit 2
	HAL_GPIO_WritePin(TFT_DATA_PORT, TFT_D3,
			(data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET); // Bit 3
	HAL_GPIO_WritePin(TFT_DATA_PORT, TFT_D4,
			(data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET); // Bit 4
	HAL_GPIO_WritePin(TFT_DATA_PORT, TFT_D5,
			(data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET); // Bit 5
	HAL_GPIO_WritePin(TFT_DATA_PORT, TFT_D6,
			(data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET); // Bit 6
	HAL_GPIO_WritePin(TFT_DATA_PORT, TFT_D7,
			(data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET); // Bit 7
	WR_LOW();
	WR_HIGH();
}
inline void TFT_LCD::write16(uint16_t data) {
	uint8_t h = data >> 8;
	uint8_t l = data & 0xff;
	write8(h);
	write8(l);
}

void TFT_LCD::sendWord(uint8_t IR, uint16_t data) {
	cs_select();
	DC_COMMAND();
	write8(IR);
	DC_DATA();
	write16(data);
	cs_deselect();
}
void TFT_LCD::sendByte(uint8_t IR, uint8_t data) {
	cs_select();
	DC_COMMAND();
	write8(IR);
	DC_DATA();
	write8(data);
	cs_deselect();
}

void TFT_LCD::sendcommand(uint8_t IR) {
	cs_select();
	DC_COMMAND();
	write8(IR);
	cs_deselect();
}
void TFT_LCD::sendDataByte(uint8_t data) {
	cs_select();
	DC_DATA();
	write8(data);
	cs_deselect();
}
void TFT_LCD::sendDataWord(uint16_t data) {
	cs_select();
	DC_DATA();
	write16(data);
	cs_deselect();
}
//*****************------TFT initial-------------------------------
void TFT_LCD::init() /* initialize TFT-LCD with HX8347 */
{
	RD_HIGH();
	reg_init();
	setRotation(2);
}
//
//void TFT_LCD::imshow(Array<uint8_t> arr, uint8_t res)
//{
//  int width = 0;
//  int height = 0;
//  switch (res)
//  {
//  case resolution::QVGA:
//    width=QVGA_width;
//    height=QVGA_height;
//    break;
//  case resolution::QQVGA:
//    width=QQVGA_width;
//    height=QQVGA_height;
//    break;
//  case resolution::CIF:
//    width=CIF_width;
//    height=CIF_height;
//    break;
//  case resolution::QCIF:
//    width=QCIF_width;
//    height=QCIF_height;
//    break;
//
//  }
//  PICO2LCD();
//  uint8_t *buf = arr.getbuf();
//  int t_width=width*2;
//  setAddrWindow(0, 0, width, height);
//  cs_select();
//  DC_DATA();
//  for (int j = 0; j < height; j++)
//  {
//    for (int i = 0; i <t_width; i++)
//    {
//      write8(buf[i+j*t_width]);
//    }
//  }
//  cs_deselect();
//}
//
//void TFT_LCD::imshow(uint8_t* arr, uint8_t res)
//{
//  int width = 0;
//  int height = 0;
//  switch (res)
//  {
//  case resolution::QVGA:
//    width=QVGA_width;
//    height=QVGA_height;
//    break;
//  case resolution::QQVGA:
//    width=QQVGA_width;
//    height=QQVGA_height;
//    break;
//  case resolution::CIF:
//    width=CIF_width;
//    height=CIF_height;
//    break;
//  case resolution::QCIF:
//    width=QCIF_width;
//    height=QCIF_height;
//    break;
//
//  }
//  PICO2LCD();
//  int t_width=width*2;
//  setAddrWindow(0, 0, width, height);
//  cs_select();
//  DC_DATA();
//  for (int j = 0; j < height; j++)
//  {
//    for (int i = 0; i <t_width; i++)
//    {
//      write8(arr[i+j*t_width]);
//    }
//  }
//  cs_deselect();
//}
//
//void TFT_LCD::imshow(uint8_t* arr, int width, int height)
//{
//  PICO2LCD();
//  int total=width*2*height;
//  //setAddrWindow(0, 0, width, height);
//  sendcommand(HX8357_RAMWR);
//  cs_select();
//  DC_DATA();
//  for (int j = 0; j <total; j++)
//  {
//
//    write8(arr[j]);
//
//  }
//  cs_deselect();
//}

void TFT_LCD::imshow(uint8_t *arr, int x, int y, int width, int height) {

	int total = width * 2 * height;
	setAddrWindow(x, y, width, height);
	sendcommand(HX8357_RAMWR);
	cs_select();
	DC_DATA();
	for (int j = 0; j < total; j++) {

		write8(arr[j]);

	}
	cs_deselect();
}
void TFT_LCD::bitmap(uint16_t * arr, int x, int y, int width, int height)
{
	int total = width * height;
	setAddrWindow(x, y, width, height);
	sendcommand(HX8357_RAMWR);
	cs_select();
	DC_DATA();
	for (int j = total-1; j >= 0; j--) {

		write16(arr[j]);

	}
	cs_deselect();
}

void TFT_LCD::color_screen_8(uint16_t color) /* TFT-LCD full screen color */
{
	cs_select();

	DC_COMMAND();
	write8(HX8357_CASET);
	DC_DATA();
	write16(0x0000);
	DC_DATA();
	write16(TFTWIDTH-1);
	DC_COMMAND();
	write8(HX8357_PASET);
	DC_DATA();
	write16(0x0000);
	DC_DATA();
	write16(TFTHEIGHT-1);
	DC_COMMAND();
	write8(HX8357_RAMWR);

//	sendWord(HX8357_CASET, 0x0000); // xPos = 0~239
//	sendDataWord(TFTWIDTH-1);
//	sendWord(HX8357_PASET, 0x0000); // yPos = 0~319
//	sendDataWord(TFTHEIGHT-1);
//	sendcommand(HX8357_RAMWR);
//	cs_select();
	DC_DATA();
	for (uint32_t i = 0; i < 320 * 480; i++) {
			write8(color >> 8);
			write8(color & 0xff);
	}
	cs_deselect();
}

#endif

void TFT_LCD::reg_init() {
	sendByte(HX8357_SWRESET, 0x01); // window setting
	sleep_ms(10);

	sendWord(HX8357D_SETC, 0xFF83);
	sendDataByte(0x57);
	sendDataByte(0xFF);
	sleep_ms(300);

	sendWord(HX8357_SETRGB, 0x8000);
	sendDataWord(0x0606);
	sendByte(HX8357D_SETCOM, 0x25);
	sendByte(HX8357_SETOSC, 0x68); // 0x68
	sendByte(HX8357_SETPANEL, 0x05);
	sendWord(HX8357_SETPWR1, 0x0015);
	sendDataWord(0x1C1C);
	sendDataWord(0x83AA);
	sendWord(HX8357D_SETSTBA, 0x5050);
	sendDataWord(0x013C);
	sendDataWord(0x1E08);
	sendWord(HX8357D_SETCYC, 0x0240);
	sendDataWord(0x002A);
	sendDataWord(0x2A0D);
	sendDataWord(0x78);            // sendDataByte(0x78)
	sendcommand(HX8357D_SETGAMMA); // setting gamma
	sendDataWord(0x020A);
	sendDataWord(0x111d);
	sendDataWord(0x2335);
	sendDataWord(0x414b);
	sendDataWord(0x4b42);
	sendDataWord(0x3A27);
	sendDataWord(0x1b08);
	sendDataWord(0x0903);
	sendDataWord(0x020A);
	sendDataWord(0x111d);
	sendDataWord(0x2335);
	sendDataWord(0x414b);
	sendDataWord(0x4b42);
	sendDataWord(0x3A27);
	sendDataWord(0x1b08);
	sendDataWord(0x0903);
	sendDataWord(0x0001);

	sendByte(HX8357_COLMOD, 0x55);
	sendByte(HX8357_MADCTL, 0x20);
	sendByte(HX8357_TEON, 0x00);
	sendByte(HX8357_TEARLINE, 0x0002);
	sendByte(HX8357_SLPOUT, 0x11);
	sendByte(HX8357_DISPON, 0x29);
}

//*****************************************************************************
/* ---------------------------------------------------------------------------- */
/*	 출력제어 함수					*/
/* ---------------------------------------------------------------------------- */
void TFT_LCD::setBGR() {
	uint8_t madctl = m_degree | TFT_BGR;
	sendcommand(HX8357_MADCTL);
	sendDataByte(madctl);
	m_color = TFT_BGR;
}
void TFT_LCD::setRGB() {
	uint8_t madctl = m_degree | TFT_RGB;
	sendcommand(HX8357_MADCTL);
	sendDataByte(madctl);
	m_color = TFT_RGB;

}
void TFT_LCD::setRotation(uint8_t m) {
	uint8_t madctl = 0;

	switch (m) {
	case 0:
		madctl = TFT_degree0 | m_color; // 0 degree rotation (default)
		m_degree = TFT_degree0;
		break;
	case 1:
		madctl = TFT_degree90 | m_color; // 90 degree rotation
		m_degree = TFT_degree90;
		break;
	case 2:
		madctl = TFT_degree180 | m_color; // 180 degree rotation
		m_degree = TFT_degree180;
		break;
	case 3:
		madctl = TFT_degree270 | m_color; // 270 degree rotation
		m_degree = TFT_degree270;
		break;
	case 4:
		madctl = TFT_X_INVERT | m_color; // 270 degree rotation
		m_degree = TFT_X_INVERT;
		break;
	case 5:
		madctl = TFT_Y_INVERT | m_color; // 270 degree rotation
		m_degree = TFT_Y_INVERT;
		break;
	case 6:
		madctl = TFT_XY_EXCHANGE | m_color; // 270 degree rotation
		m_degree = TFT_XY_EXCHANGE;
		break;
	}
	sendcommand(HX8357_MADCTL); // MADCTL command
	sendDataByte(madctl);       // Set the rotation
}

void TFT_LCD::setAddrWindow(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h) {
	uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);
	sendWord(HX8357_CASET, x1); // xPos = 0~239
	sendDataWord(x2);
	sendWord(HX8357_PASET, y1); // yPos = 0~319
	sendDataWord(y2);
	sendcommand(HX8357_RAMWR);
}
void TFT_LCD::GRAM_address(uint16_t xPos, uint16_t yPos) /* set GRAM address of TFT-LCD */
{
	// if((xPos > 239) || (yPos > 319)) return;
	cs_select();
	DC_COMMAND();
	write8(HX8357_CASET);
	DC_DATA();
	write16(xPos);
	DC_DATA();
	write16(xPos + 1);
	DC_COMMAND();
	write8(HX8357_PASET);
	DC_DATA();
	write16(yPos);
	DC_DATA();
	write16(yPos + 1);
	DC_COMMAND();
	write8(HX8357_RAMWR);
	cs_deselect();
}

void TFT_LCD::clear_screen(void) /* TFT-LCD clear screen with black color */
{
#ifdef TFT_8BIT
	color_screen_8(Black);
#elif defined(TFT_SPI)
	color_screen(Black);
#endif
}

void TFT_LCD::screen_mode(uint8_t mode) {
}

void TFT_LCD::frame() {
	Rectangle(0, 0, TFTWIDTH - 1, TFTHEIGHT - 1,2, White);
}

void TFT_LCD::TFT_pixel(uint16_t xPos, uint16_t yPos, uint16_t color) /* write a pixel */
{
	if ((xPos < 0) || (yPos < 0) || (xPos >= TFTWIDTH) || (yPos >= TFTHEIGHT))
		return;
	cs_select();
	DC_COMMAND();
	write8(HX8357_CASET);
	DC_DATA();
	write16(xPos);
	DC_DATA();
	write16(xPos + 1);
	DC_COMMAND();
	write8(HX8357_PASET);
	DC_DATA();
	write16(yPos);
	DC_DATA();
	write16(yPos + 1);
	DC_COMMAND();
	write8(HX8357_RAMWR);
//	sendcommand(HX8357_CASET); // Column address set
//	sendDataWord(xPos);
//	sendDataWord(xPos + 1);
//	sendcommand(HX8357_PASET); // Row address set
//	sendDataWord(yPos);
//	sendDataWord(yPos + 1);
//	sendcommand(HX8357_RAMWR); // Write to RAM

	DC_DATA();
	for (int i = 0; i < 1; i++)
	{
		//sendDataWord(color);
		write16(color);
	}

	cs_deselect();
}

void TFT_LCD::xy(uint8_t xChar, uint8_t yChar) /* set character position (x,y) */
{
	xcharacter = xChar;
	ycharacter = yChar;
}

void TFT_LCD::color(uint16_t colorfore, uint16_t colorback) /* set foreground and background color */
{
	foreground = colorfore;
	background = colorback;
}

void TFT_LCD::string_size(int16_t xChar, int16_t yChar, int16_t colorfore,
		int16_t colorback, char *str, int8_t width, int8_t height) /* write TFT-LCD string */
		{
	drawstring(xChar, yChar, str, colorfore, colorback, width, height);
}
void TFT_LCD::string(uint16_t xChar, uint16_t yChar, int16_t colorfore,
		int16_t colorback, char *str) /* write TFT-LCD string */
		{
	drawstring(xChar, yChar, str, colorfore, colorback, 1, 1);
}

void TFT_LCD::drawstring(uint16_t x, uint16_t y, char *str, uint16_t text_color,
		uint16_t background_color, uint8_t size_x, uint8_t size_y) {
	char ch1;
	char width = 0;
	if (size_x == 1)
		width = 5;
	else if (size_x == 2)
		width = 10;
	int16_t xpos = 0, ypos = 0;

	int count = 0, cnty = 0;
	xcharacter = x;
	ycharacter = y;
	while (*str) {
		ch1 = *str;
		str++;
		xpos = xcharacter + count * (width + 3);
		ypos = ycharacter + 16 * (cnty);
		if (xpos > CHAR_RETURN_LEN) {
			cnty++;
			xcharacter = 0;
			count = 0;
			//ychar=16;
		}
		drawChar(xpos, ypos, ch1, text_color, background_color, size_x, size_y); // English ASCII character
		count++;

	}
}
void TFT_LCD::drawChar(uint16_t x, uint16_t y, uint8_t c, uint16_t color,
		uint16_t bg, uint8_t size_x, uint8_t size_y) {
	for (uint16_t i = 0; i < 5; i++)  // Char bitmap = 5 columns
			{
		uint8_t line = font[c * 5 + i];
		for (uint16_t j = 0; j < 8; j++, line >>= 1) {
			if (line & 1) {
				if (size_x == 1 && size_y == 1)
					TFT_pixel(x + i, y + j, color);
				else
					writeFillRect(x + i * size_x, y + j * size_y, size_x,
							size_y, color);
			} else if (bg != color) {
				if (size_x == 1 && size_y == 1)
					TFT_pixel(x + i, y + j, bg);
				else
					writeFillRect(x + i * size_x, y + j * size_y, size_x,
							size_y, bg);
			}
		}
	}
	if (bg != color)  // If opaque, draw vertical line for last column
			{
		if (size_x == 1 && size_y == 1)
			writeFastVLine(x + 5, y, 8, bg);
		else
			writeFillRect(x + 5 * size_x, y, size_x, 8 * size_y, bg);
	}
}
void TFT_LCD::writeFillRect(int16_t x, int16_t y, int16_t size_x,
		int16_t size_y, uint16_t color) {
	for (short i = 0; i < size_x; i++)
		drawSLine(x + 1, y + i, size_y, color);
}

void TFT_LCD::drawSLine(int16_t x, int16_t y, int16_t length, uint16_t color) {
	int i;
	int x2 = x + length;

	setAddrWindow(x, y, x2, y);
	cs_select();
	DC_COMMAND();
	write8(RAMWR);
	DC_DATA();
	for (i = 0; i < (length - 1); i++) {
		write8(color >> 8);
		write8(color);
	}
	cs_deselect();
}

void TFT_LCD::writeFastVLine(int16_t x, int16_t y, int16_t i, uint16_t bg) {
	// Overwrite in subclasses if startWrite is defined!
	// Can be just
	Line(x, y, x, y + i - 1, bg);
	// or writeFillRect(x, y, 1, h, bg);
}

void TFT_LCD::TFT_English(uint8_t code) /* write a English ASCII character */
{
	unsigned char data, x, y;
	unsigned int pixel[8][16];
	unsigned int dot0, dot1, dot2, dot3, dot4;

	for (x = 0; x < 8; x++) // read English ASCII font
			{
		data = E_font[code][x];
		for (y = 0; y < 8; y++) {
			if (data & 0x01)
				pixel[x][y] = foreground;
			else
				pixel[x][y] = background;
			data = data >> 1;
		}
	}

	for (x = 0; x < 8; x++) {
		data = E_font[code][x + 8];
		for (y = 0; y < 8; y++) {
			if (data & 0x01)
				pixel[x][y + 8] = foreground;
			else
				pixel[x][y + 8] = background;
			data = data >> 1;
		}
	}

	if (outline_flag == 1) // display outline
		for (x = 0; x < 8; x++) {
			dot0 = E_font[code][x] + E_font[code][x + 8] * 256;
			dot1 = dot0 >> 1;                                       // up side
			dot2 = dot0;                                            // down side
			dot3 = E_font[code][x + 1] + E_font[code][x + 8] * 256; // left side
			dot4 = E_font[code][x - 1] + E_font[code][x + 7] * 256; // right side

			for (y = 0; y < 15; y++) {
				if (!(dot0 & 0x0001)) {
					if (dot1 & 0x0001)
						pixel[x][y] = outline;
					if (dot2 & 0x0001)
						pixel[x][y] = outline;
					if ((dot3 & 0x0001) && (x < 7))
						pixel[x][y] = outline;
					if ((dot4 & 0x0001) && (x > 0))
						pixel[x][y] = outline;
				}

				dot1 >>= 1;
				dot2 = dot0;
				dot0 >>= 1;
				dot3 >>= 1;
				dot4 >>= 1;
			}
		}

	if ((cursor_flag == 1) && (xcharacter == xcursor)
			&& (ycharacter == ycursor)) {
		for (x = 0; x < 8; x++) // display cursor
				{
			pixel[x][14] = cursor;
			pixel[x][15] = cursor;
		}
	}

	if (ScreenMode == 'P') {
		for (y = 0; y < 16; y++) // write in portrait mode
			for (x = 0; x < 8; x++)
				TFT_pixel(xcharacter * 8 + x, ycharacter * 8 + y, pixel[x][y]);
	} else {
		for (y = 0; y < 16; y++) // write in landscape mode
			for (x = 0; x < 8; x++)
				TFT_pixel((29 - ycharacter) * 8 + 7 - y, xcharacter * 8 + x,
						pixel[x][y]);
	}

	xcharacter += 1;
	if (xcharacter >= LineLimit) // end of line ?
			{
		xcharacter = 0;
		ycharacter += 2;
	}
}

unsigned int TFT_LCD::KS_code_conversion(uint16_t KSSM) /* convert KSSM(�ϼ���) to KS(������) */
{
	unsigned char HB, LB;
	unsigned int index, KS;

	HB = KSSM >> 8;
	LB = KSSM & 0x00FF;

	if (KSSM >= 0xB0A1 && KSSM <= 0xC8FE) {
		index = (HB - 0xB0) * 94 + LB - 0xA1;
		KS = KS_Table[index][0] * 256;
		KS |= KS_Table[index][1];

		return KS;
	} else
		return -1;
}

void TFT_LCD::TFT_Korean(uint16_t code) /* write a Korean character */
{
	unsigned char cho_5bit, joong_5bit, jong_5bit;
	unsigned char cho_bul, joong_bul, jong_bul = 0, i, jong_flag;
	unsigned int ch;

	cho_5bit = table_cho[(code >> 10) & 0x001F];   // get 5bit(14-10) of chosung
	joong_5bit = table_joong[(code >> 5) & 0x001F]; // get 5bit(09-05) of joongsung
	jong_5bit = table_jong[code & 0x001F];        // get 5bit(04-00) of jongsung

	if (jong_5bit == 0) // if jongsung not exist
			{
		jong_flag = 0;
		cho_bul = bul_cho1[joong_5bit];
		if ((cho_5bit == 1) || (cho_5bit == 16))
			joong_bul = 0;
		else
			joong_bul = 1;
	} else // if jongsung exist
	{
		jong_flag = 1;
		cho_bul = bul_cho2[joong_5bit];
		if ((cho_5bit == 1) || (cho_5bit == 16))
			joong_bul = 2;
		else
			joong_bul = 3;
		jong_bul = bul_jong[joong_5bit];
	}

	ch = cho_bul * 20 + cho_5bit; // get chosung font
	for (i = 0; i < 32; i++)
		KoreanBuffer[i] = K_font[ch][i];
	ch = 8 * 20 + joong_bul * 22 + joong_5bit; // OR joongsung font
	for (i = 0; i < 32; i++)
		KoreanBuffer[i] |= K_font[ch][i];
	if (jong_flag) // OR jongsung font
	{
		ch = 8 * 20 + 4 * 22 + jong_bul * 28 + jong_5bit;
		for (i = 0; i < 32; i++)
			KoreanBuffer[i] |= K_font[ch][i];
	}

	unsigned char data, x, y;
	unsigned int pixel[16][16];
	unsigned int dot0, dot1, dot2, dot3, dot4;

	for (x = 0; x < 16; x++) // read Korean font
			{
		data = KoreanBuffer[x];
		for (y = 0; y < 8; y++) {
			if (data & 0x01)
				pixel[x][y] = foreground;
			else
				pixel[x][y] = background;
			data = data >> 1;
		}
	}

	for (x = 0; x < 16; x++) {
		data = KoreanBuffer[x + 16];
		for (y = 0; y < 8; y++) {
			if (data & 0x01)
				pixel[x][y + 8] = foreground;
			else
				pixel[x][y + 8] = background;
			data = data >> 1;
		}
	}

	if (outline_flag == 1) // display outline
		for (x = 0; x < 16; x++) {
			dot0 = KoreanBuffer[x] + KoreanBuffer[x + 16] * 256;
			dot1 = dot0 >> 1;                                        // up side
			dot2 = dot0;                                            // down side
			dot3 = KoreanBuffer[x + 1] + KoreanBuffer[x + 16] * 256; // left side
			dot4 = KoreanBuffer[x - 1] + KoreanBuffer[x + 15] * 256; // right side

			for (y = 0; y < 16; y++) {
				if (!(dot0 & 0x0001)) {
					if (dot1 & 0x0001)
						pixel[x][y] = outline;
					if (dot2 & 0x0001)
						pixel[x][y] = outline;
					if ((dot3 & 0x0001) && (x < 15))
						pixel[x][y] = outline;
					if ((dot4 & 0x0001) && (x > 0))
						pixel[x][y] = outline;
				}

				dot1 >>= 1;
				dot2 = dot0;
				dot0 >>= 1;
				dot3 >>= 1;
				dot4 >>= 1;
			}
		}

	if ((cursor_flag == 1) && (xcharacter == xcursor)
			&& (ycharacter == ycursor)) {
		for (x = 0; x < 16; x++) // display cursor
				{
			pixel[x][14] = cursor;
			pixel[x][15] = cursor;
		}
	}

	if (xcharacter >= (LineLimit - 1)) // end of line ?
			{
		xcharacter = 0;
		ycharacter += 2;
	}

	if (ScreenMode == 'P') // write in portrait mode
			{
		for (y = 0; y < 16; y++)
			for (x = 0; x < 16; x++)
				TFT_pixel(xcharacter * 8 + x, ycharacter * 8 + y, pixel[x][y]);
	} else // write in landscape mode
	{
		for (y = 0; y < 16; y++)
			for (x = 0; x < 16; x++)
				TFT_pixel((29 - ycharacter) * 8 + 7 - y, xcharacter * 8 + x,
						pixel[x][y]);
	}

	xcharacter += 2;
	if (xcharacter >= LineLimit) // end of line ?
			{
		xcharacter = 0;
		ycharacter += 2;
	}
}

void TFT_LCD::TFT_cursor(uint16_t cursor_color) /* set cursor and color */
{
	if (cursor_color == Transparent) // disable cursor
		cursor_flag = 0;
	else // enable cursor
	{
		cursor_flag = 1;
		cursor = cursor_color;
	}
}
void TFT_LCD::TFT_outline(uint16_t outline_color) /* set outline and color */
{
	if (outline_color == Transparent) // disable outline
		outline_flag = 0;
	else // enable outline
	{
		outline_flag = 1;
		outline = outline_color;
	}
}

unsigned int TFT_LCD::Unicode_to_KS(uint16_t unicode) // convert Unicode(�����ڵ�) to KS(������)
		{
	unsigned char cho = 0, joong = 0, jong = 0;
	unsigned int value;

	value = unicode - 0xAC00; // �����ڵ忡�� '��'�� �ش��ϴ� ���� ����.

	jong = value % 28; // �����ڵ带 �ʼ�, �߼�, �������� �и�
	joong = ((value - jong) / 28) % 21;
	cho = ((value - jong) / 28) / 21;

	cho += 2; // �ʼ� + ������

	if (joong < 5)
		joong += 3; // �߼� + ������
	else if (joong < 11)
		joong += 5;
	else if (joong < 17)
		joong += 7;
	else
		joong += 9;

	if (jong < 17)
		jong++; // ���� + ������
	else
		jong += 2;

	return 0x8000 | (cho << 10) | (joong << 5) | jong; // ������ �ڵ�
}

void TFT_LCD::unsigned_decimal(uint32_t number, uint8_t zerofill, uint8_t digit) /* display unsigned decimal number */
{
	unsigned char zero_flag, character, count = 0;
	unsigned long div;

	if ((digit == 0) || (digit > 9))
		return;

	div = 1;
	while (--digit)
		div *= 10;

	zero_flag = zerofill;
	while (div > 0) // display number
	{
		count++;
		xcharacter += 5 * count;
		character = number / div;
		if ((character == 0) && (zero_flag == 0) && (div != 1))
			TFT_English(character + ' ');
		else {
			zero_flag = 1;
			TFT_English(character + '0');
		}
		number %= div;
		div /= 10;
	}
}

void TFT_LCD::signed_decimal(int32_t number, uint8_t zerofill, uint8_t digit) /* display signed decimal number */
{
	unsigned char zero_flag, character;
	unsigned long div;

	if ((digit == 0) || (digit > 9))
		return;

	if (number >= 0) // display sign
		TFT_English('+');
	else {
		TFT_English('-');
		number = -number;
	}

	div = 1;
	while (--digit)
		div *= 10;

	zero_flag = zerofill;
	while (div > 0) // display number
	{
		character = number / div;
		if ((character == 0) && (zero_flag == 0) && (div != 1))
			TFT_English(character + ' ');
		else {
			zero_flag = 1;
			TFT_English(character + '0');
		}
		number %= div;
		div /= 10;
	}
}

void TFT_LCD::hexadecimal(uint32_t number, uint8_t digit) /* display hexadecimal number */
{
	unsigned char i, character, count = 0;

	if ((digit == 0) || (digit > 8))
		return;

	for (i = digit; i > 0; i--) {
		count++;
		character = (number >> 4 * (i - 1)) & 0x0F;
		xcharacter += 5 * count;
		if (xcharacter > 230) {
			xcharacter = 0;
			ycharacter += 16;
		}
		if (character < 10)
			TFT_English(character + '0'); //
		else
			TFT_English(character - 10 + 'A'); //
		xcharacter += 8;
	}
	xcharacter += 8;
}

void TFT_LCD::_0x_hexadecimal(uint32_t number, uint8_t digit) /* display hexadecimal number with 0x */
{
	unsigned char i, character;

	if ((digit == 0) || (digit > 8))
		return;

	TFT_English('0');
	TFT_English('x');

	for (i = digit; i > 0; i--) {
		character = (number >> 4 * (i - 1)) & 0x0F;
		if (character < 10)
			TFT_English(character + '0');
		else
			TFT_English(character - 10 + 'A');
	}
}

void TFT_LCD::unsigned_float(float number, uint8_t integral, uint8_t fractional) /* display unsigned floating-point number */
{
	unsigned char zero_flag, digit, character; // integral = digits of integral part
	unsigned long div, integer;        // fractional = digits of fractional part

	digit = integral + fractional;
	if ((integral == 0) || (fractional == 0) || (digit > 9))
		return;

	div = 1;
	while (--digit)
		div *= 10;

	while (fractional--)
		number *= 10.;
	integer = (uint32_t) (number + 0.5);

	zero_flag = 0;
	digit = 1;
	while (div > 0) // display number
	{
		character = integer / div;
		if ((character == 0) && (zero_flag == 0) && (digit != integral))
			TFT_English(character + ' ');
		else {
			zero_flag = 1;
			TFT_English(character + '0');
		}
		integer %= div;
		div /= 10;

		if (digit == integral)
			TFT_English('.');
		digit++;
	}
}

void TFT_LCD::signed_float(float number, uint8_t integral, uint8_t fractional) /* display signed floating-point number */
{
	unsigned char zero_flag, digit, character;
	unsigned long div, integer;

	digit = integral + fractional;
	if ((integral == 0) || (fractional == 0) || (digit > 9))
		return;

	if (number >= 0) // display sign
		TFT_English('+');
	else {
		TFT_English('-');
		number = -number;
	}

	div = 1;
	while (--digit)
		div *= 10;

	while (fractional--)
		number *= 10.;
	integer = (uint32_t) (number + 0.5);

	zero_flag = 0;
	digit = 1;
	while (div > 0) // display number
	{
		character = integer / div;
		if ((character == 0) && (zero_flag == 0) && (digit != integral))
			TFT_English(character + ' ');
		else {
			zero_flag = 1;
			TFT_English(character + '0');
		}
		integer %= div;
		div /= 10;

		if (digit == integral)
			TFT_English('.');
		digit++;
	}
}

/* ---------------------------------------------------------------------------- */
/*		�׷��� �Լ�							*/
/* ---------------------------------------------------------------------------- */

void TFT_LCD::Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		uint16_t color) /* draw a straight line */
		{
	int x, y;
	int x_diff = x2 - x1;
	int y_diff = y2 - y1;

	if (y1 != y2) // if y1 != y2, y is variable
			{
		if (x1 == x2) {
			sendcommand(HX8357_CASET); // Column address set
			sendDataWord(x1);
			sendDataWord(x2);
			sendcommand(HX8357_PASET); // Row address set
			sendDataWord(y1);
			sendDataWord(y2);
			sendcommand(HX8357_RAMWR); // Write to RAM
			for (int i = 0; i < y_diff; i++)
				sendDataWord(color);
			cs_deselect();
		} else {
			if (y1 < y2) //              x is function
				for (y = y1; y <= y2; y++) {
					x = x1 + (long) (y - y1) * (long) (x_diff) / y_diff;
					TFT_pixel(x, y, color);
				}
			else
				for (y = y1; y >= y2; y--) {
					x = x1 + (long) (y - y1) * (long) (x_diff) / y_diff;
					TFT_pixel(x, y, color);
				}
		}

	} else if (x1 != x2) // if x1 != x2, x is variable
			{
		if (y1 == y2) {
			sendcommand(HX8357_CASET); // Column address set
			sendDataWord(x1);
			sendDataWord(x2);
			sendcommand(HX8357_PASET); // Row address set
			sendDataWord(y1);
			sendDataWord(y2);
			sendcommand(HX8357_RAMWR); // Write to RAM
			for (int i = 0; i < x_diff; i++)
				sendDataWord(color);
			cs_deselect();
		} else {
			if (x1 < x2) //              y is function
				for (x = x1; x <= x2; x++) {
					y = y1 + (long) (x - x1) * (long) (y_diff) / x_diff;
					TFT_pixel(x, y, color);
				}
			else
				for (x = x1; x >= x2; x--) {
					y = y1 + (long) (x - x1) * (long) (y_diff) / x_diff;
					TFT_pixel(x, y, color);
				}
		}

	} else
		// if x1 == x2 and y1 == y2, it is a dot
		TFT_pixel(x1, y1, color);
}

void TFT_LCD::Line_width(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		int16_t width, uint16_t color) {
	int x, y;
	int x_diff = x2 - x1;
	int y_diff = y2 - y1;
	int linx = x_diff * width;
	int liny = y_diff * width;
	int tmp = 0;
	if (y1 != y2) // if y1 != y2, y is variable
			{
		if (x1 == x2) {
			if (x2 + width >= TFTWIDTH) {
				sendcommand(HX8357_CASET); // Column address set
				sendDataWord(TFTWIDTH - width);
				sendDataWord(TFTWIDTH-1);
			} else {
				sendcommand(HX8357_CASET); // Column address set
				sendDataWord(x1);
				sendDataWord(x2 + width-1);
			}
			sendcommand(HX8357_PASET); // Row address set
			sendDataWord(y1);
			sendDataWord(y2);
			sendcommand(HX8357_RAMWR); // Write to RAM
			for (int i = 0; i < liny; i++)
				sendDataWord(color);
//			cs_deselect();
		} else {
			if (y1 < y2) //              x is function
				for (y = y1; y <= y2; y++) {
					x = x1 + (long) (y - y1) * (long) (x_diff) / y_diff;
					TFT_pixel(x, y, color);
				}
			else
				for (y = y1; y >= y2; y--) {
					x = x1 + (long) (y - y1) * (long) (x_diff) / y_diff;
					TFT_pixel(x, y, color);
				}
		}

	} else if (x1 != x2) // if x1 != x2, x is variable
			{
		if (y1 == y2) {
			sendcommand(HX8357_CASET); // Column address set
			sendDataWord(x1);
			sendDataWord(x2);
			if (y1-width<0) {
				sendcommand(HX8357_PASET); // Column address set
				sendDataWord(y1);
				sendDataWord(y2+width);
			} else {
				sendcommand(HX8357_PASET); // Column address set
				sendDataWord(y1-width);
				sendDataWord(y2);
			}
			sendcommand(HX8357_RAMWR); // Write to RAM
			for (int i = 0; i < linx; i++)
				sendDataWord(color);
			//cs_deselect();
		} else {
			if (x1 < x2) //              y is function
				for (x = x1; x <= x2; x++) {
					y = y1 + (long) (x - x1) * (long) (y_diff) / x_diff;
					TFT_pixel(x, y, color);
				}
			else
				for (x = x1; x >= x2; x--) {
					y = y1 + (long) (x - x1) * (long) (y_diff) / x_diff;
					TFT_pixel(x, y, color);
				}
		}

	} else
		// if x1 == x2 and y1 == y2, it is a dot
		TFT_pixel(x1, y1, color);
}
void TFT_LCD::Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		uint16_t color) /* draw a rectangle */
{
	Line(x1, y1, x1, y2, color); // horizontal line
	Line(x2, y1, x2, y2, color);
	Line(x1, y1, x2, y1, color); // vertical line
	Line(x1, y2, x2, y2, color);
}
void TFT_LCD::Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		int16_t width, uint16_t color) /* draw a rectangle */
{
	Line_width(x1, y1, x1, y2, width, color); // horizontal line
	Line_width(x2, y1, x2, y2, width, color);
	Line_width(x1, y1, x2, y1, width, color); // vertical line
	Line_width(x1, y2, x2, y2, width, color);
}
void TFT_LCD::Rectangle_rect(int16_t x, int16_t y, int16_t width, int16_t height,
		int16_t l_width, uint16_t color) /* draw a rectangle */
{
	int16_t x1 = x;
	int16_t x2 = x+width;
	int16_t y1 = y;
	int16_t y2 = y+height;
	Line_width(x1, y1, x1, y2, l_width, color); // horizontal line
	Line_width(x2, y1, x2, y2, l_width, color);
	Line_width(x1, y1, x2, y1, l_width, color); // vertical line
	Line_width(x1, y2, x2, y2, l_width, color);
}

void TFT_LCD::Rectangle(Rect rect, int16_t width, uint16_t color) /* draw a rectangle */
{
	int16_t x1 = rect.x;
	int16_t x2 = rect.x+rect.width;
	int16_t y1 = rect.y;
	int16_t y2 = rect.y+rect.height;
	Line_width(x1, y1, x1, y2, width, color); // horizontal line
	Line_width(x2, y1, x2, y2, width, color);
	Line_width(x1, y1, x2, y1, width, color); // vertical line
	Line_width(x1, y2, x2, y2, width, color);
}

void TFT_LCD::Block(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, uint16_t fill) /* draw a rectangle with filled color */
{
	int i;

	Line(x1, y1, x1, y2, color); // horizontal line
	Line(x2, y1, x2, y2, color);
	Line(x1, y1, x2, y1, color); // vertical line
	Line(x1, y2, x2, y2, color);

	if ((y1 < y2) && (x1 != x2)) // fill block
			{
		for (i = y1 + 1; i <= y2 - 1; i++)
			Line(x1 + 1, i, x2 - 1, i, fill);
	} else if ((y1 > y2) && (x1 != x2)) {
		for (i = y2 + 1; i <= y1 - 1; i++)
			Line(x1 + 1, i, x2 - 1, i, fill);
	}
}
void TFT_LCD::Block(Rect rect, uint16_t color, uint16_t fill)
{
	int i;
	int16_t x1 = rect.x;
	int16_t x2 = rect.x+rect.width;
	int16_t y1 = rect.y;
	int16_t y2 = rect.y+rect.height;
	cs_select();
	DC_COMMAND();
	write8(HX8357_CASET);
	DC_DATA();
	write16(x1);
	DC_DATA();
	write16(x2-1);
	DC_COMMAND();
	write8(HX8357_PASET);
	DC_DATA();
	write16(y1);
	DC_DATA();
	write16(y2-1);
	DC_COMMAND();
	write8(HX8357_RAMWR);
	DC_DATA();
	for(i=0;i<rect.width*rect.height;i++)
	{
		write16(color);
	}
	cs_deselect();
	if(color != fill)
	{
		Line(x1, y1, x1, y2, color); // horizontal line
		Line(x2, y1, x2, y2, color);
		Line(x1, y1, x2, y1, color); // vertical line
		Line(x1, y2, x2, y2, color);
	}
//
//	if ((y1 < y2) && (x1 != x2)) // fill block
//			{
//		for (i = y1 + 1; i <= y2 - 1; i++)
//			Line(x1 + 1, i, x2 - 1, i, fill);
//	} else if ((y1 > y2) && (x1 != x2)) {
//		for (i = y2 + 1; i <= y1 - 1; i++)
//			Line(x1 + 1, i, x2 - 1, i, fill);
//	}
}

void TFT_LCD::Circle(int16_t x1, int16_t y1, int16_t r, uint16_t color) /* draw a circle */
{
	int x, y;
	float s;

	for (y = y1 - r * 3 / 4; y <= y1 + r * 3 / 4; y++) // draw with y variable
			{
		s = sqrt((long) r * (long) r - (long) (y - y1) * (long) (y - y1)) + 0.5;
		x = x1 + (int) s;
		TFT_pixel(x, y, color);
		x = x1 - (int) s;
		TFT_pixel(x, y, color);
	}

	for (x = x1 - r * 3 / 4; x <= x1 + r * 3 / 4; x++) // draw with x variable
			{
		s = sqrt((long) r * (long) r - (long) (x - x1) * (long) (x - x1)) + 0.5;
		y = y1 + (int) s;
		TFT_pixel(x, y, color);
		y = y1 - (int) s;
		TFT_pixel(x, y, color);
	}
}
void TFT_LCD::Circle(int16_t x1, int16_t y1, int16_t r, uint16_t color, bool fill) /* draw a circle */
{
    int x, y;
    float s;

    if (fill) {
        // 내부를 채우는 경우, y를 기준으로 수평선을 그림
        for (y = y1 - r; y <= y1 + r; y++) {
            s = sqrt((long)r * (long)r - (long)(y - y1) * (long)(y - y1)) + 0.5;
            x = (int)s;
            Line(x1 - x, y, x1 + x, y, color);  // 수평선 그리기
        }
    } else {
        // 원 테두리만 그리는 경우
        for (y = y1 - r * 3 / 4; y <= y1 + r * 3 / 4; y++) {
            s = sqrt((long)r * (long)r - (long)(y - y1) * (long)(y - y1)) + 0.5;
            x = x1 + (int)s;
            TFT_pixel(x, y, color);
            x = x1 - (int)s;
            TFT_pixel(x, y, color);
        }

        for (x = x1 - r * 3 / 4; x <= x1 + r * 3 / 4; x++) {
            s = sqrt((long)r * (long)r - (long)(x - x1) * (long)(x - x1)) + 0.5;
            y = y1 + (int)s;
            TFT_pixel(x, y, color);
            y = y1 - (int)s;
            TFT_pixel(x, y, color);
        }
    }
}

void TFT_LCD::Sine(int16_t peak, uint8_t mode, uint16_t color) /* draw a sine curve */
{
	int x, y;

	if (mode == 0)
		for (y = 0; y <= 319; y++) {
			x =	120	- (int) (sin((float) y * 1.6875 * M_PI / 180.) * peak + 0.5);
			TFT_pixel(x, y, color);
		}
	else
		for (y = 0; y <= 319; y++) {
			x =	120	+ (int) (sin((float) y * 1.6875 * M_PI / 180.) * peak + 0.5);
			TFT_pixel(x, y, color);
		}
}



void TFT_LCD::Slider(uint16_t xPos, uint16_t yPos,uint16_t percent, uint16_t background_color, uint16_t slider_color)
{
	Block(Rect(xPos,yPos,TFTWIDTH-1,50),background_color,background_color);
	Block(Rect(xPos+20,yPos+20,280,5),slider_color,slider_color);
	Block(Rect(xPos+10+percent,yPos+15,10,10),Orange,Orange);

	//Circle(xPos+20+percent*2.8,yPos+25,10,slider_color,1);


}
