/*
 * TFT_LCD_reg.h
 *
 *  Created on: Dec 5, 2024
 *      Author: spc
 */

#ifndef TFT_LCD_REG_H_
#define TFT_LCD_REG_H_

#define Background_gray 0x2104//0x4208

#define button_gray 0x4208

#define Red				0xf800				// 빨
#define Orange			0xfb00				// 주
#define Yellow			0x07ff				// 노
#define Green			0x07e0				// 초
#define Blue			0x001f			    // 파
#define Dark_blue		0x000b				// 남
#define Violet			0x980e				// 보
#define White		    0xffff				// 흰
#define Black			0x0000				// 검
#define	Cyan			0xffe0
#define	Magenta			0xf81f
#define khaki           0x7f13              // 0xf731
#define olive           0x8400
#define pink            0xfb56
#define purple          0x8010
#define silver          0xc618
#define Transparent 	0x0821

//registers
#define NOP         0x00
#define SWRESET     0x01
#define RDDID       0x04
#define RDDST       0x09
#define SLPIN       0x10
#define SLPOUT      0x11
#define INVOFF      0x20
#define INVON       0x21
#define DISPOFF     0x28
#define DISPON      0x29
#define CASET       0x2A
#define PASET       0x2B
#define RAMWR       0x2C
#define RAMRD       0x2E
#define TEON        0x35
#define TEARLINE    0x44
#define MADCTL      0x36
#define COLMOD      0x3A
#define SETOSC      0xB0
#define SETPWR1     0xB1
#define SETRGB      0xB3
#define SETCOM      0xB6
#define SETCYC      0xB4
#define SETC        0xB9
#define SETSTBA     0xC0
#define SETPANEL    0xCC
#define SETGAMMA    0xE0

#define HX8357_NOP 0x00
#define HX8357_SWRESET 0x01
#define HX8357_RDDID 0x04
#define HX8357_RDDST 0x09
#define HX8357_SLPIN 0x10
#define HX8357_SLPOUT 0x11
#define HX8357_INVOFF 0x20
#define HX8357_INVON 0x21
#define HX8357_DISPOFF 0x28
#define HX8357_DISPON 0x29
#define HX8357_CASET 0x2A
#define HX8357_PASET 0x2B
#define HX8357_RAMWR 0x2C
#define HX8357_RAMRD 0x2E
#define HX8357_TEON 0x35
#define HX8357_TEARLINE 0x44
#define HX8357_MADCTL 0x36
#define HX8357_COLMOD 0x3A
#define HX8357_SETOSC 0xB0
#define HX8357_SETPWR1 0xB1
#define HX8357_SETRGB 0xB3
#define HX8357D_SETCOM 0xB6
#define HX8357D_SETCYC 0xB4
#define HX8357D_SETC 0xB9
#define HX8357D_SETSTBA 0xC0
#define HX8357_SETPANEL 0xCC
#define HX8357D_SETGAMMA 0xE0


#endif /* TFT_LCD_REG_H_ */
