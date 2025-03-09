/*
 * MP3.h
 *
 *  Created on: Dec 22, 2024
 *      Author: spc
 */

#ifndef INC_MP3_H_
#define INC_MP3_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "fatfs.h"
#include <string.h> // for memset, strcpy
#include "Youn_s_lab_board_info.h"

#define MAX_FILES 50     // 최대 파일 개수
#define MAX_FILENAME 100  // 최대 파일 이름 길이

enum Pages{
	HOME_PAGE,
	LIST_PAGE,
	PLAY_PAGE,
};


#define EVENT_VOL_P 1
#define EVENT_VOL_M 2
#define EVENT_REPEAT 3
#define EVENT_HOME 4
#define EVENT_PAUSE 5
#define EVENT_NEXT 6
#define EVENT_PREV 7


void scanMp3Files(TCHAR * path);
void scanMp3FilesSFN(TCHAR *path);
void printFileName(const TCHAR* fname) ;
void readAndSendFile(const char* filePath);
#ifdef __cplusplus
}
#endif

#endif /* INC_MP3_H_ */
