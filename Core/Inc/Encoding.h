/*
 * Encoding.h
 *
 *  Created on: Jan 14, 2025
 *      Author: spc
 */

#ifndef INC_ENCODING_H_
#define INC_ENCODING_H_

#include <string.h>
#include "fatfs.h"

void char_to_tchar(const char* charStr, TCHAR* tcharStr, size_t maxLen);

void utf16_to_utf8(const WCHAR *utf16, char *utf8);
void TCHAR_to_UTF8(char* dest, const TCHAR* src);

void UTF16ToUTF8(const uint16_t* src, char* dest, size_t destSize) ;



#endif /* INC_ENCODING_H_ */
