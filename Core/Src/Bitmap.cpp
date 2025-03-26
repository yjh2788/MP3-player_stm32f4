/*
 * Bitmap.c
 *
 *  Created on: Mar 25, 2025
 *      Author: spc
 */

#include "Bitmap.h"
#include <iostream>
#include <stdint.h>
#include "ff.h"
#include "TFT_LCD.h"

inline uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
	uint16_t r5 = (r >> 3) & 0x1F;  // 8bit -> 5bit
	uint16_t g6 = (g >> 2) & 0x3F;  // 8bit -> 6bit
	uint16_t b5 = (b >> 3) & 0x1F;  // 8bit -> 5bit
	return (r5 << 11) | (g6 << 5) | b5;
}

inline void display_bmp_to_lcd(int xpos, int ypos, const char *filename) {
	FIL file;
	FRESULT res;
	UINT bytes_read;
	BMPHeader header;
	TFT_LCD &lcd = TFT_LCD::getInstance();

	res = f_open(&file, filename, FA_READ);
	if (res != FR_OK) {
		std::cout << "File open failed: " << res << "\r\n";
		return;
	}

	// file header (read 14byte)
	uint8_t file_header[14];
	f_read(&file, file_header, 14, &bytes_read);
	if (bytes_read != 14) {
		std::cout << "File header read failed: " << bytes_read << "\r\n";
		f_close(&file);
		return;
	}

	header.signature = *(uint16_t*) &file_header[0];
	header.file_size = *(uint32_t*) &file_header[2];
	header.data_offset = *(uint32_t*) &file_header[10];

	// DIB header (read 16byte)
	uint8_t dib_header[16];
	f_read(&file, dib_header, 16, &bytes_read);
	if (bytes_read != 16) {
		std::cout << "DIB header read failed: " << bytes_read << "\r\n";
		f_close(&file);
		return;
	}

	header.dib_size = *(uint32_t*) &dib_header[0];
	header.width = *(uint32_t*) &dib_header[4];
	header.height = *(uint32_t*) &dib_header[8];
	header.planes = *(uint16_t*) &dib_header[12];
	header.bits_per_pixel = *(uint16_t*) &dib_header[14];

//	        std::cout << "Signature: " << std::hex << header.signature << std::dec << "\r\n";
//	        std::cout << "File Size: " << header.file_size << "\r\n";
//	        std::cout << "Data Offset: " << header.data_offset << "\r\n";
//	        std::cout << "DIB Size: " << header.dib_size << "\r\n";
//	        std::cout << "Width: " << header.width << "\r\n";
//	        std::cout << "Height: " << header.height << "\r\n";
//	        std::cout << "Planes: " << header.planes << "\r\n";
//	        std::cout << "Bits per Pixel: " << header.bits_per_pixel << "\r\n";

	if (header.bits_per_pixel != 24) {
		std::cout << "Not supported format, bits per pixel: "
				<< header.bits_per_pixel << "\r\n";
		f_close(&file);
		return;
	}

	std::cout << "Width: " << header.width << ", Height: " << header.height
			<< "\r\n";

	f_lseek(&file, header.data_offset);

	uint8_t rgb[3];
	uint16_t rgb565;
	for (int y = header.height - 1; y >= 0; y--) {
		for (int x = 0; x < header.width; x++) {
			f_read(&file, rgb, 3, &bytes_read);
			if (bytes_read != 3) {
				std::cout << "Pixel data read failed at (" << x << ", " << y
						<< ")\r\n";
				f_close(&file);
				return;
			}
			rgb565 = rgb888_to_rgb565(rgb[2], rgb[1], rgb[0]);
			lcd.TFT_pixel(x + xpos, y + ypos, rgb565);
		}
	}

	f_close(&file);
}

inline void display_bmp_to_arr(const char *filename, uint16_t *buf) {
	FIL file;
	FRESULT res;
	UINT bytes_read;
	BMPHeader header;
	TFT_LCD &lcd = TFT_LCD::getInstance();

	res = f_open(&file, filename, FA_READ);
	if (res != FR_OK) {
		std::cout << "File open failed: " << res << "\r\n";
		return;
	}

	// file header (read 14byte)
	uint8_t file_header[14];
	f_read(&file, file_header, 14, &bytes_read);
	if (bytes_read != 14) {
		std::cout << "File header read failed: " << bytes_read << "\r\n";
		f_close(&file);
		return;
	}

	header.signature = *(uint16_t*) &file_header[0];
	header.file_size = *(uint32_t*) &file_header[2];
	header.data_offset = *(uint32_t*) &file_header[10];

	//DIB header (read 16byte)
	uint8_t dib_header[16];
	f_read(&file, dib_header, 16, &bytes_read);
	if (bytes_read != 16) {
		std::cout << "DIB header read failed: " << bytes_read << "\r\n";
		f_close(&file);
		return;
	}

	header.dib_size = *(uint32_t*) &dib_header[0];
	header.width = *(uint32_t*) &dib_header[4];
	header.height = *(uint32_t*) &dib_header[8];
	header.planes = *(uint16_t*) &dib_header[12];
	header.bits_per_pixel = *(uint16_t*) &dib_header[14];

//	std::cout << "Signature: " << std::hex << header.signature << std::dec
//			<< "\r\n";
//	std::cout << "File Size: " << header.file_size << "\r\n";
//	std::cout << "Data Offset: " << header.data_offset << "\r\n";
//	std::cout << "DIB Size: " << header.dib_size << "\r\n";
//	std::cout << "Width: " << header.width << "\r\n";
//	std::cout << "Height: " << header.height << "\r\n";
//	std::cout << "Planes: " << header.planes << "\r\n";
//	std::cout << "Bits per Pixel: " << header.bits_per_pixel << "\r\n";

	if (header.bits_per_pixel != 24) {
		std::cout << "Not supported format, bits per pixel: "
				<< header.bits_per_pixel << "\r\n";
		f_close(&file);
		return;
	}

//	std::cout << "Width: " << header.width << ", Height: " << header.height
//			<< "\r\n";

	f_lseek(&file, header.data_offset);

	uint8_t rgb[3];
	uint16_t rgb565;
	for (int y = header.height - 1; y >= 0; y--) {
		for (int x = 0; x < header.width; x++) {
			f_read(&file, rgb, 3, &bytes_read);
			if (bytes_read != 3) {
				std::cout << "Pixel data read failed at (" << x << ", " << y
						<< ")\r\n";
				f_close(&file);
				return;
			}
			buf[x + y * header.width] = rgb888_to_rgb565(rgb[2], rgb[1],
					rgb[0]);

		}
	}
	f_close(&file);
}

