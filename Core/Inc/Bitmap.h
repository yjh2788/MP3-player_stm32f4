/*
 * Bitmap.h
 *
 *  Created on: Mar 25, 2025
 *      Author: spc
 */

#ifndef INC_BITMAP_H_
#define INC_BITMAP_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)) {
	uint16_t signature;      // 0x00: "BM"
	uint32_t file_size;      // 0x02
	uint32_t reserved;       // 0x06
	uint32_t data_offset;    // 0x0A
	uint32_t dib_size;       // 0x0E
	uint32_t width;          // 0x12
	uint32_t height;         // 0x16
	uint16_t planes;         // 0x1A
	uint16_t bits_per_pixel; // 0x1C
} BMPHeader;

uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b);
void display_bmp_to_lcd(int x, int y, const char *filename);
void display_bmp_to_arr(const char *filename, uint16_t *buf);

#ifdef __cplusplus
}
#endif
#endif
