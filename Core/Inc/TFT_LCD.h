/*
 * TFT_LCD.h
 *
 *  Created on: Dec 5, 2024
 *      Author: spc
 */

#ifndef TFT_LCD_H_
#define TFT_LCD_H_

#include "myfunc.h"
#include <math.h>
#include <stdint.h>
#include "TFT_LCD_reg.h"
#include "Youn_s_lab_board_info.h"

#define __DELAY_BACKWARD_COMPATIBLE__
#define TFTWIDTH 320
#define TFTHEIGHT 480
//#define TFTWIDTH 480
//#define TFTHEIGHT 320
#define TFTLCD_DELAY 250




#define TFT_RGB 0x08
#define TFT_BGR 0x00
#define TFT_degree0 0x00
#define TFT_degree90 0x60
#define TFT_degree180 0xD0
#define TFT_degree270 0xA0
#define TFT_Y_INVERT 0x80
#define TFT_X_INVERT 0x40
#define TFT_XY_EXCHANGE 0x20

//#define READ_BIT 0x80

#ifdef __cplusplus
extern "C" {
#endif


class TFT_LCD {
private:
	inline void cs_select();
	inline void cs_deselect();
	inline void DC_DATA();
	inline void DC_COMMAND();
	TFT_LCD();
	~TFT_LCD();
public:


#ifdef TFT_8BIT
private:
	inline void WR_LOW();
	inline void WR_HIGH();
	inline void RD_LOW();
	inline void RD_HIGH();
	inline void write8(uint8_t data);
	inline void write16(uint16_t data);
public:
	void init();
	void color_screen_8(uint16_t color);

#elif defined(TFT_SPI)
private:
    inline void spi_transfer_byte(uint8_t byte);
    inline void spi_transfer_word(uint16_t word);
public:
    SPI_HandleTypeDef *m_spi;
    uint32_t m_baud;
    inline void write8(uint8_t data);
    inline void write16(uint16_t data);
    void init(SPI_HandleTypeDef *spi);
    void color_screen(uint16_t color);
    void ChangeSPIspeed(SPI_HandleTypeDef* hspi, uint32_t newPrescaler);
#endif

    //singleton pattern
    static TFT_LCD& getInstance() {
            static TFT_LCD instance; // static local variable instance
            return instance;
   }
    TFT_LCD(const TFT_LCD&) = delete;
    TFT_LCD& operator=(const TFT_LCD&) = delete;

	uint8_t m_degree;
	uint8_t m_color;

	void reg_init();
	void setRotation(uint8_t m);
	//void imshow(Array<uint8_t> arr, uint8_t res);
	void imshow(uint8_t *arr, uint8_t res);
	void imshow(uint8_t *arr, int width, int height);
	void imshow(uint8_t *arr, int x, int y, int width, int height);
	void sendWord(uint8_t IR, uint16_t data);
	void sendByte(uint8_t IR, uint8_t data);
	void sendcommand(uint8_t IR);
	void sendDataByte(uint8_t data);
	void sendDataWord(uint16_t data);
	void setBGR();
	void setRGB();
	// void TFT_clear();
	void clear_screen(void); // TFT-LCD clear screen with black color
	// void TFT_color_screen(uint16_t color);		// TFT-LCD full screen color
	void GRAM_address(uint16_t xPos, uint16_t yPos); // set GRAM address of TFT-LCD
	void setAddrWindow(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h);

	void frame();
	void screen_mode(uint8_t mode); // set screen direction mode
	void TFT_pixel(uint16_t xPos, uint16_t yPos, uint16_t color);
	void writeFillRect(int16_t x, int16_t y, int16_t size_x, int16_t size_y,
			uint16_t color);
	void writeFastVLine(int16_t x, int16_t y, int16_t i, uint16_t bg);
	void xy(uint8_t xChar, uint8_t yChar);            // set character position
	void color(uint16_t colorfore, uint16_t colorback); // set foreground and background color
	void drawChar(uint16_t x, uint16_t y, uint8_t c, uint16_t color, uint16_t bg,
			uint8_t size_x, uint8_t size_y);
	void drawstring(uint16_t x, uint16_t y, char *str, uint16_t text_color,
			uint16_t background_color, uint8_t size_x, uint8_t size_y);
	void string_size(int16_t xChar, int16_t yChar, int16_t colorfore,
			int16_t colorback, char *str, int8_t width, int8_t height);
	void string(uint16_t xChar, uint16_t yChar, int16_t colorfore,
			int16_t colorback, char *str);	// write TFT-LCD string
	void TFT_English(uint8_t code);           // write a English ASCII character
	unsigned int KS_code_conversion(uint16_t KSSM); // convert KSSM(�ϼ���) to KS(������)
	void TFT_Korean(uint16_t code);                 // write a Korean character
	void TFT_cursor(uint16_t cursor_color);         // set cursor and color
	void TFT_outline(uint16_t outline_color);       // set outline and color
	unsigned int Unicode_to_KS(uint16_t unicode);
	void unsigned_decimal(uint32_t number, uint8_t zerofill, uint8_t digit); // display unsigned decimal number
	void signed_decimal(int32_t number, uint8_t zerofill, uint8_t digit); // display signed decimal number
	void hexadecimal(uint32_t number, uint8_t digit); // display hexadecimal number
	void _0x_hexadecimal(uint32_t number, uint8_t digit); // display hexadecimal number with 0x
	void unsigned_float(float number, uint8_t integral, uint8_t fractional); // display unsigned floating-point number
	void signed_float(float number, uint8_t integral, uint8_t fractional); // display signed floating-point number
	//
	void drawSLine(int16_t x, int16_t y, int16_t length, uint16_t color);
	void Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color); // draw a straight line
	void Line_width(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t width, uint16_t color);
	void Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
			uint16_t color);       // draw a rectangle
	void Rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t width,
				uint16_t color);       // draw a rectangle
	void Block(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color,
			uint16_t fill); // draw a rectangle with filled color
	void Circle(int16_t x1, int16_t y1, int16_t r, uint16_t color); // draw a circle
	void Sine(int16_t peak, uint8_t mode, uint16_t color);
};
#ifdef __cplusplus
}
#endif
#endif /* TFT_LCD_H_ */
