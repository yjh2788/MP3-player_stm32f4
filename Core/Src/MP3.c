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


void SkipID3v2(FIL *fil) {
    UINT bytesRead;
    uint8_t header[10];

    // ID3v2 헤더 읽기 (10바이트)
    f_read(fil, header, 10, &bytesRead);
    if (bytesRead == 10 && memcmp(header, "ID3", 3) == 0) {
        // ID3v2 크기 계산 (4바이트, 동기 안전 정수)
        uint32_t size = ((header[6] & 0x7F) << 21) |
                        ((header[7] & 0x7F) << 14) |
                        ((header[8] & 0x7F) << 7) |
                        (header[9] & 0x7F);
        size += 10; // 헤더 크기 포함

        // ID3v2 건너뛰기
        f_lseek(fil, size);
        printf("ID3v2 크기: %lu 바이트 건너뜀\n", size);
    } else {
        // ID3v2 없으면 처음으로 되돌리기
        f_lseek(fil, 0);
    }
}

void FindFirstAudioFrame(FIL *fil) {
    UINT bytesRead;
    uint8_t buffer[512];

    while (1) {
        uint32_t pos = f_tell(fil);
        f_read(fil, buffer, sizeof(buffer), &bytesRead);
        if (bytesRead < 4) break; // 파일 끝

        for (UINT i = 0; i < bytesRead - 1; i++) {
            if (buffer[i] == 0xFF && (buffer[i + 1] & 0xE0) == 0xE0) {
                // MPEG 프레임 발견
                f_lseek(fil, pos + i); // 프레임 시작으로 이동
                printf("오디오 프레임 발견 at %lu\n", pos + i);
                return;
            }
        }
    }
}

void ReadMP3Audio(const char *filename) {
    UINT bytesRead;
    uint8_t buffer[512];
    FRESULT res;

    f_mount(&fs, "", 1);
    if (f_open(&fil, filename, FA_READ) == FR_OK) {
        // ID3v2 건너뛰기
        SkipID3v2(&fil);
        // 첫 오디오 프레임 찾기
        FindFirstAudioFrame(&fil);

        // 오디오 데이터 읽기
        do {
            res = f_read(&fil, buffer, sizeof(buffer), &bytesRead);
            if (res != FR_OK) break;

            // 오디오 데이터 처리 (예: 디코더로 전달)
            for (UINT i = 0; i < bytesRead; i++) {
                printf("%02X ", buffer[i]);
            }
            printf("\n");
        } while (bytesRead > 0);

        f_close(&fil);
    }
    f_mount(NULL, "", 0);
}


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

