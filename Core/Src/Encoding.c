/*
 * Encoding.c
 *
 *  Created on: Jan 14, 2025
 *      Author: spc
 */
#include "Encoding.h"

void char_to_tchar(const char* charStr, TCHAR* tcharStr, size_t maxLen) {
    size_t i;
    for (i = 0; i < maxLen && charStr[i] != '\0'; ++i) {
        tcharStr[i] = (TCHAR)charStr[i];  // char에서 TCHAR(=unsigned short)로 변환
    }
    tcharStr[i] = 0;  // Null terminator 추가
}



//void TCHAR_to_UTF8(char *dest, const TCHAR *src) {
//	wcstombs(dest, src, MAX_FILENAME); // wchar_t -> char 변환
//}
void UTF16ToUTF8(const uint16_t *src, char *dest, size_t destSize) {
	size_t i = 0;
	while (src[i] != 0 && i < destSize - 1) {
		if (src[i] <= 0x7F) {
			// 1바이트 ASCII 문자
			dest[i] = (char) src[i];
		} else if (src[i] <= 0x7FF) {
			// 2바이트 UTF-8 문자
			dest[i] = (char) (0xC0 | (src[i] >> 6));
			dest[i + 1] = (char) (0x80 | (src[i] & 0x3F));
			i++;
		} else {
			// 3바이트 이상 UTF-8 문자 (어떤 특수문자도 처리 가능)
			dest[i] = (char) (0xE0 | (src[i] >> 12));
			dest[i + 1] = (char) (0x80 | ((src[i] >> 6) & 0x3F));
			dest[i + 2] = (char) (0x80 | (src[i] & 0x3F));
			i += 2;
		}
		i++;
	}
	dest[i] = '\0';  // 문자열 끝
}
void utf16_to_utf8(const WCHAR *utf16, char *utf8) {
	while (*utf16) {
		uint16_t w = *utf16++;
		if (w < 0x80) {
			*utf8++ = (char) w;
		} else if (w < 0x800) {
			*utf8++ = 0xC0 | ((w >> 6) & 0x1F);
			*utf8++ = 0x80 | (w & 0x3F);
		} else {
			*utf8++ = 0xE0 | ((w >> 12) & 0x0F);
			*utf8++ = 0x80 | ((w >> 6) & 0x3F);
			*utf8++ = 0x80 | (w & 0x3F);
		}
	}
	*utf8 = '\0';
}



