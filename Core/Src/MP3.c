/*
 * MP3.c
 *
 *  Created on: Dec 22, 2024
 *      Author: spc
 */

#include "MP3.h"
#include "Encoding.h"
#include <stdio.h>

FATFS fs;           // File system object
FIL fil;            // File object
FILINFO fno;        // File information object
DIR dir;            // Directory object
TCHAR  fileNames[MAX_FILES][MAX_FILENAME];  // 파일 이름 저장 배열
UINT fileCount = 0; // 파일 개수


void scanMp3Files(TCHAR *path) {
	FRESULT res;
	fileCount = 0; // 파일 개수 초기화
	// 디렉토리 열기
	res = f_opendir(&dir, path);
	if (res == FR_OK) {
		while (1) {
			// 디렉토리 내 파일 정보 읽기
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0)
				break; // 끝까지 읽었거나 오류 발생

			if (!(fno.fattrib & AM_DIR)) { // 디렉토리가 아닌 경우
				// 확장자가 MP3인지 확인
				if (wcsstr(fno.fname, L".mp3") || wcsstr(fno.fname, L".MP3")) {
					if (fileCount < MAX_FILES) {
						wcscpy(fileNames[fileCount], fno.fname); // 파일 이름 저장
						fileCount++;
					}
				}
			}
		}
		f_closedir(&dir);
	} else {
		printf("Directory open failed!\n");
	}
}

void scanMp3FilesSFN(TCHAR *path) {
	FRESULT res;
	fileCount = 0; // 파일 개수 초기화

	// 디렉토리 열기
	res = f_opendir(&dir, path);
	if (res == FR_OK) {
		while (1) {
			// 디렉토리 내 파일 정보 읽기
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0)
				break; // 끝까지 읽었거나 오류 발생

			if (!(fno.fattrib & AM_DIR)) { // 디렉토리가 아닌 경우
				// 확장자가 MP3인지 확인
				if (strstr(fno.fname, ".mp3") || strstr(fno.fname, ".MP3")) {
					if (fileCount < MAX_FILES) {
						strcpy(fileNames[fileCount], fno.fname); // 파일 이름 저장
						fileCount++;
					}
				}

			}
		}
		f_closedir(&dir);
	} else {
		printf("Directory open failed!\n");
	}
}


void printFileName(const TCHAR *fname) {
	char utf8name[MAX_FILENAME];  // 출력할 UTF-8 문자열
	UTF16ToUTF8((const uint16_t*) fname, utf8name, sizeof(utf8name)); // UTF-16에서 UTF-8로 변환
	printf("File: %s\n\r", utf8name);  // UTF-8로 출력
}

void readAndSendFile(const char *filePath) {
	FRESULT res;
	UINT br;
	unsigned char buffer[512]; // 512바이트 버퍼

	// 파일 열기
	res = f_open(&fil, filePath, FA_READ);
	if (res == FR_OK) {
		printf("Opened file: %s\n", filePath);

		while (1) {
			// 파일에서 512바이트 읽기
			res = f_read(&fil, buffer, sizeof(buffer), &br);
			if (res != FR_OK || br == 0)
				break; // 오류 또는 파일 끝

			// 데이터 전송 (사용자 정의 함수로 구현 필요)
			//sendData(buffer, br);
		}
		f_close(&fil);
	} else {
		printf("Failed to open file: %s\n", filePath);
	}
}

