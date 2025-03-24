/*
 * sd_card.h
 *
 *  Created on: Dec 5, 2024
 *      Author: spc
 */

#ifndef INC_SD_CARD_H_
#define INC_SD_CARD_H_

#include "main.h"
#include "TFT_LCD.h"

#define USE_FATFS_LIB

#define _BV(n) (1 << n)
// 비트 클리어
#define cbi(reg, bit) reg &= ~(_BV(bit))
// 비트 셋
#define sbi(reg, bit) reg |= (_BV(bit))

// SD card instruction
#define CMD0	0				// GO_IDLE_STATE
#define CMD8	8				// SEND_IF_COND
#define CMD17	17				// READ_SINGLE_BLOCK
#define CMD41	41				// SD_SEND_OP_COND
#define CMD55	55				// APP_CMD
#define CMD58	58				// READ_OCR

// SD 카드 버전
#define SD_VER1		0				// SD Version 1.X SD Memory Card
#define SD_VER2		1				// SD Version 2.0 or later SD Memory Card
#define SD_VER2_HC	1				// SD Version 2.0 High Capacity Card
#define SD_VER2_SD	0				// SD Version 2.0 표준 포맷 Card

// FAT32 정의
#define MBR				0				// sector #0(Master Boot Record)
#define EndOfCluster	0x0FFFFFFF		// end of cluster
#define u_charPerSector	512				// 512 Byte

#define MAX_NUM_FILE	50				// maximum number of file

#define BytePerSec 512
// 변수

typedef unsigned long u_long;

// MBR 섹터에 있는 파티션 테이블 구조
struct __attribute__((packed))PartTable {
	uint8_t Bootable;
	uint8_t StartHead;
	uint16_t StartCylSec;
	uint8_t Type;
	uint8_t EndHead;
	uint16_t EndCylSec;
	uint32_t LBA_Begin;		// 파티션의 시작 번지
	uint32_t Size;
};

// PBR 섹터의 전체 구조
struct __attribute__((packed))BootSector {
	uint8_t BS_impBoot[3];
	char BS_OEMName[8];
	uint16_t BPB_uint8_tsPerSec;
	uint8_t BPB_SecPerClus;
	uint16_t BPB_RsvdSecCnt;
	uint8_t BPB_NumFATs;
	uint16_t BPB_RootEntCnt;
	uint16_t BPB_TotSec16;
	uint8_t BPB_Media;
	uint16_t BPB_FATSz16;
	uint16_t BPB_SecPerTrk;
	uint16_t BPB_NumHeads;
	uint32_t BPB_HiddSec;
	uint32_t BPB_TotSec32;

	uint32_t BPB_FATSz32;
	uint16_t BPB_ExtFlags;
	uint16_t BPB_FSVer;
	uint32_t BPB_RootClus;
	uint16_t BPB_FSInfo;
	uint16_t BPB_BkBootSec;
	uint8_t BPB_Reserved[12];
	uint8_t BS_DrvNum;
	uint8_t BS_Reserved1;
	uint8_t BS_BootSig;
	uint32_t BS_VollD;
	char BS_VolLab[11];
	char BS_FilSysType[8];
	uint8_t boot_code[422];
};

//  directory entry structure(32_chars)
//	short dir
struct __attribute__((packed)) DirEntry {
	uint8_t DIR_Name[8];
	uint8_t DIR_Ext[3];
	uint8_t DIR_Attr;
	uint8_t DIR_NTRes;
	uint8_t DIR_CrtTimeTenth;
	uint16_t DIR_CrtTime;
	uint16_t DIR_CrtDate;
	uint16_t DIR_LastAccDate;
	uint16_t DIR_FstClusHI;
	uint16_t DIR_WrtTime;
	uint16_t DIR_WrtDate;
	uint16_t DIR_FstClusLO;
	uint32_t DIR_FileSize;
};

// long dir

struct __attribute__((packed)) DirEntryLong {
	uint8_t DIR_order;
	uint8_t DIR_Name_1[10];
	uint8_t DIR_Attr;
	uint8_t DIR_Type;
	uint8_t DIR_ChkSUM;
	uint8_t DIR_Name_2[12];
	uint16_t DIR_FstClusLo;
	uint8_t DIR_Name_3[4];

};

#ifdef __cplusplus
extern "C" {
#endif

class SDcard {
private:
	static SDcard* instance;
	inline void SD_select();
	inline void SD_deselect();
	SDcard();
	~SDcard();

public:
	//singleton pattern
	static SDcard& getInstance() {
		static SDcard instance; // static local variable instance
		return instance;
	}
	SDcard(const SDcard&) = delete;
	SDcard& operator=(const SDcard&) = delete;

	TFT_LCD& m_lcd=TFT_LCD::getInstance();
	SPI_HandleTypeDef *m_spi;

	//file start cluster
	uint32_t fileStartClust[MAX_NUM_FILE];
	uint32_t file_len[MAX_NUM_FILE];
	uint32_t RootDirSector;
	uint16_t numOfFile;			// file name
	uint32_t First_FAT_Sector;
	uint8_t SecPerClus;
	uint8_t sd_type;
	uint8_t FileName[MAX_NUM_FILE][9];
	uint8_t FileLongName[MAX_NUM_FILE][26*10];
	uint8_t buffer[BytePerSec];

	uint8_t stop_play = 0;
	uint8_t repeat = 0;
	uint16_t sel_pg = 1;
	uint16_t sel_no = 0;
	uint16_t sel_tno;
	uint16_t tot_pg;
	uint16_t tot_no;
	uint16_t play_no = 0;
	uint8_t sbuf[20];

	uint8_t spibuf[6];

	bool SD_Init(SPI_HandleTypeDef* spi);
	void SD_Command(uint8_t cmd, uint32_t arg);
	void SD_Read(uint32_t sector);
	int SD_Read(uint32_t sector,uint8_t *buffer) ;
	void SPI_speed_175kHz(void);
	void SPI_speed_350kHz(void);
	void SPI_speed_1_4MHz(void);
	void SPI_speed_3MHz(void);
	void SPI_speed_6MHz(void);
	void SPI_speed_11MHz(void);
	uint8_t SPI_write(uint8_t data);

	void FAT_Init(void);
	uint16_t fatGetDirEntry(void);
	uint32_t fatClustToSect(uint32_t cluster);
	uint32_t FAT_NextCluster(uint32_t cluster);

	uint8_t SD_readRes1(void);

};
#ifdef __cplusplus
}
#endif

#endif /* INC_SD_CARD_H_ */

