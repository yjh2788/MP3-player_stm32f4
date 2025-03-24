/*
 * sd_card.cpp
 *
 *  Created on: Dec 5, 2024
 *      Author: spc
 */
#include "SDcard.h"
#include "myfunc.h"
#include <algorithm>
#include "main.h"
#include <cstring>
#include <iostream>
#include "Youn_s_lab_board_info.h"

SDcard::SDcard() {

}
SDcard::~SDcard() {

}
inline void SDcard::SD_select() {
	HAL_GPIO_WritePin(SD_PORT, SD_CS, GPIO_PIN_RESET); //EXT2_SD_CS
}
inline void SDcard::SD_deselect() {
	HAL_GPIO_WritePin(SD_PORT, SD_CS, GPIO_PIN_SET);
}
bool SDcard::SD_Init(SPI_HandleTypeDef *spi) {
	m_spi = spi;

	uint8_t i, j, status = 1, res1, cmd_flag = 0;
	uint8_t r1_response;
	uint8_t r7_response[4];
	uint8_t r3_response[7];
	int count = 0;

	// SDcard check /////////////////////////////////////////////////////

	uint8_t tmphex = 0, response;

	//m_lcd.color_screen(Black);
	//m_lcd.frame();
	m_lcd.string(10, 5, Green, Black,
			(char*) " ******************************");
	m_lcd.string(10, 16, Magenta, Black,
			(char*) "     SD/SDHC initialize...     ");
	m_lcd.string(10, 28, Green, Black,
			(char*) " ******************************");
//////////////////////////////////////////////////////////////////

	SPI_speed_175kHz(); // SPI 250kHz (100 ~ 400 kHz)
	//SPI_speed_350kHz();
// SD_CS = High로 놓고 Dummy Data 10번 전송
// 74 클럭 이상 인가해서 SPI 모드 진입
	SD_deselect();
	for (i = 0; i < 20; i++) {
		SPI_write(0xFF);
	}
	HAL_Delay(2);
	SD_select();

	HAL_Delay(2);
	//m_lcd.string(10, 48, Magenta, Black, " CMD0 : ");
	//m_lcd.xy(30, 48);
	START: SD_Command(CMD0, 0);
	do {
		response = SPI_write(0xFF);
	} while (response == 0xFF);

	if (response == 0x01) {
		// IDLE 상태 진입 성공
		SD_deselect();
		m_lcd.string(60, 48, Green, Black, " Card in IDLE state");
		SD_select();
		std::cout << "IDLE state \r" << std::endl;
	} else {
		// 오류 처리
		SD_deselect();
		m_lcd.string(60, 48, Red, Black, "Error");

		std::cout << "error state \r" << std::endl;
		std::cout << "normal" << response << "\r" << std::endl;
		std::cout << std::hex << response << "\r" << std::endl;

		//m_lcd.hexadecimal(response, 2);
		SD_deselect();
		return 0x01;
	}
//	for (i = 0; i < 10; i++) {
//		tmphex = SPI_write(0xFF);
//		m_lcd.hexadecimal(tmphex, 2);
//		///////////////////////////////////////////////////////
//		std::cout << "tmphex:" << std::hex << (int) tmphex << "\r" << std::endl;
//		///////////////////////////////////////////////////////
//		HAL_Delay(3);
//	}
	SD_deselect();
	m_lcd.string(10, 64, Magenta, Black, " CMD8 : ");
	SD_select();

	SD_Command(CMD8, 0x1AA);
	HAL_Delay(10);

	do {
		r1_response = SPI_write(0xFF);

		if (r1_response == 0x05) {
			sd_type = SD_VER1;
			std::cout << "CMD8: V1.X\r" << std::endl;
			break;
		} else if (r1_response == 0x01) {
			sd_type = SD_VER2;
			std::cout << "CMD8: V2.X\r" << std::endl;
			break;
		} else
			std::cout << "CMD8 failed\r" << std::endl;
		//SD_deselect();
		//return 0x01;
	} while (1);

	switch (sd_type) {
	case SD_VER2:
		// R7 Response 수신 4 u_chars
		// (R1 Response 는 do while 에서 수신)

		for (i = 0; i < 4; i++)		//receive r7 response
			r7_response[i] = SPI_write(0xFF);

		if (r1_response == 0x05) {
			SD_deselect();
			m_lcd.string(100, 64, Green, Black, "/ V1.X");
			SD_select();

		} else if (r1_response == 0x01) {
			SD_deselect();
			m_lcd.string(100, 64, Green, Black, "/ V2.X");
			SD_select();
		}

		if ((r7_response[2] == 0x01) && (r7_response[3] == 0xAA)) {

			//SPI_write(0xFF);
			do {
				// 5번 이상 반복해야 R1 Response = 0x0 수신 가능
				SD_deselect();
				m_lcd.string(10, 80, Magenta, Black, " CMD55: ");
				SD_select();

				SD_Command(CMD55, 0);
				HAL_Delay(10);
				// R1 Response
				do {
					response = SPI_write(0xFF);
				} while (response == 0xFF);
				if (response == 0x01) {
					std::cout << "CMD55 success \r" << std::endl;
					SD_deselect();
					m_lcd.string(60, 80, Magenta, Black, "success");
					SD_select();
				} else {
					std::cout << "CMD55 failed\r" << std::endl;
					SD_deselect();
					m_lcd.string(60, 80, Magenta, Black, " failed ");
					SD_select();
					//return 0x01;
				}

				//printString(tftm_lcd_xpos, tftm_lcd_ypos, "\n", 0xFFFF, 0x0, 1);

				SPI_write(0xFF);
				SPI_write(0xFF);

				SD_deselect();
				m_lcd.string(10, 96, Magenta, Black, " ACMD41:");
				SD_select();

				SD_Command(CMD41, 0x40000000); // HCS bit = 1
				HAL_Delay(10);
				do {
					response = SPI_write(0xFF);
				} while (response == 0xFF);

				if (response == 0x00) {
					SD_deselect();
					m_lcd.string(60, 96, Magenta, Black, "success");
					SD_select();
					std::cout << "CMD41 success\r" << std::endl;
					std::cout << "res1=" << std::hex << int(response) << "\r"
							<< std::endl;
					break;
				} else {
					SD_deselect();
					m_lcd.string(60, 96, Magenta, Black, " failed ");
					SD_select();
					std::cout << "CMD41 failed\r" << std::endl;
					//SD_deselect();
					//return 0x01;
				}
//				for (i = 0; i < 10; i++) {
//					res1 = SPI_write(0xFF);		// R1 Response
//					if (res1 == 0x0)
//						cmd_flag = 1;
//
//				}
				std::cout << "res1=" << std::hex << int(response) << "\r"
						<< std::endl;

				HAL_Delay(100);
				count++;
			} while (count < 50);
			if (count >= 50) {
				count = 0;
				goto START;
			}
		} //end of if
		else {
			std::cout << "cmd failed\r" << std::endl;
			goto START;
		}

		//while((r7_response[2] == 0x01 && r7_response[3] == 0xAA));
		//if(cmd_flag == 1) //printString(tftm_lcd_xpos, tftm_lcd_ypos, "SD ready!\n", 0xFFFF, 0xF800, 2);
		SD_deselect();
		m_lcd.string(10, 112, Magenta, Black, " CMD58: "); // send CMD58(check SDHC)
		SD_select();
		//m_lcd.color(Cyan, Black);
		SD_Command(CMD58, 0);			// check ccs bit
		HAL_Delay(10);

		for (i = 0; i < 5; i++) {
			r3_response[0] = SPI_write(0xff);
			HAL_Delay(3);
			if (r3_response[0] == 0x0) {
				for (j = 1; j < 5; j++) {
					r3_response[j] = SPI_write(0xff);
					HAL_Delay(3);
				}
				break;
			}
		}
		if ((r3_response[1] & 0x40) != 0) {
			sd_type = SD_VER2_HC;
			SD_deselect();
			m_lcd.string(100, 112, Magenta, Black, "(SDHC)");
			SD_select();
		} else {
			sd_type = SD_VER2_SD;
			SD_deselect();
			m_lcd.string(100, 112, Magenta, Black, "(SD)");
			SD_select();
		}

		for (i = 0; i < 10; i++) {
			SPI_write(0xFF);
			HAL_Delay(1);
		}
		break;

		// case ver1
	case SD_VER1:
		for (j = 0; j < 5; j++) {
			SD_Command(CMD55, 0);
			HAL_Delay(10);
			for (i = 0; i < 10; i++) {
				SPI_write(0xFF);
				HAL_Delay(3);
			}

			SD_Command(CMD41, 0x00000000);
			HAL_Delay(10);
			for (i = 0; i < 10; i++) {
				SPI_write(0xFF);
				HAL_Delay(3);
			}
		}
		break;
	}
	//	SD_CS = 1;
	SD_deselect();

	m_lcd.string(40, 128, Green, Black, "initialize complete");
	std::cout << "sd card initialized \r" << std::endl;
	return 0;

}

void SDcard::SD_Command(uint8_t cmd, uint32_t arg) {
	uint8_t crc;
	uint8_t status_;

	SPI_write(cmd | 0x40);		// transition bit 1(6비트째) | cmd
	SPI_write(arg >> 24);		// 4byte 인수 전송
	SPI_write(arg >> 16);
	SPI_write(arg >> 8);
	SPI_write(arg);

	crc = 0x01;		//except CMD0, CMD8
	if (cmd == CMD0)
		crc = 0x95;		// CRC for CMD0
	if (cmd == CMD8)
		crc = 0x87;		// CRC for CMD8(0x000001AA)

	SPI_write(crc);

//	for (int i = 0; i < 8; i++) {
//		status_ = SPI_write(0xFF);
//
//		m_lcd.hexadecimal(status_, 2);
//		///////////////////////////////////////////////////////
//		std::cout << "tmphex:" << std::hex << (int) status_ << "\r" << std::endl;
//		///////////////////////////////////////////////////////
//		HAL_Delay(3);
//		if (status_ != 0xFF) break;
//	}
//	return status_;

}

void SDcard::SD_Read(uint32_t sector) {
	uint16_t i;
	//SPI_speed_6MHz();
	SPI_speed_1_4MHz();
	//SPI_speed_350kHz();

	// High Capacity 카드가 아니면 섹터번호에 *512

	//	SD_CS = 0;			// SD_CS = Low (SD 카드 활성화)
	SD_select();
	//HAL_Delay(1);

	SD_Command(CMD17, sector);
	//std::cout<<std::hex<<(int)status<<std::endl;
	while (SPI_write(0xFF) != 0x00) {
		//std::cout<<std::hex<<(int)SPI_write(0xFF)<<"\r"<<std::endl;
	}

	// wait for R1 = 0x00
	//blink(1);
	//std::cout << "0x00 checked\r" << std::endl;
	while (SPI_write(0xFF) != 0xFE) {
		//std::cout<<std::hex<<(int)SPI_write(0xFF)<<"\r"<<std::endl;
	}
	// wait for Start Block Token = 0xFE
	//check(500);
	//std::cout << "0xfe checked\r" << std::endl;
	for (i = 0; i < 512; i++)
		buffer[i] = SPI_write(0xFF);

	// CRC 수신 (2 byte)
	SPI_write(0xFF);
	SPI_write(0xFF);
	SPI_write(0xFF);

	//	SD_CS = 1;				// SD 카드 비활성화
	SD_deselect();
}

int SDcard::SD_Read(uint32_t sector, uint8_t *buffer) {
	uint16_t i;
	//SPI_speed_6MHz();
	SPI_speed_1_4MHz();
	//SPI_speed_350kHz();

	// High Capacity 카드가 아니면 섹터번호에 *512

	//	SD_CS = 0;			// SD_CS = Low (SD 카드 활성화)
	SD_select();
	//HAL_Delay(1);

	SD_Command(CMD17, sector);
	//std::cout<<std::hex<<(int)status<<std::endl;
	while (SPI_write(0xFF) != 0x00) {
		//std::cout<<std::hex<<(int)SPI_write(0xFF)<<"\r"<<std::endl;
	}

	// wait for R1 = 0x00
	//blink(1);
	//std::cout << "0x00 checked\r" << std::endl;
	while (SPI_write(0xFF) != 0xFE) {
		//std::cout<<std::hex<<(int)SPI_write(0xFF)<<"\r"<<std::endl;
	}
	// wait for Start Block Token = 0xFE
	//check(500);
	//std::cout << "0xfe checked\r" << std::endl;
	for (i = 0; i < 512; i++)
		buffer[i] = SPI_write(0xFF);

	// CRC 수신 (2 byte)
	SPI_write(0xFF);
	SPI_write(0xFF);
	SPI_write(0xFF);

	//	SD_CS = 1;				// SD 카드 비활성화
	SD_deselect();
}

void SDcard::SPI_speed_11MHz(void) {
	__HAL_SPI_DISABLE(m_spi);             // SPI disable
	m_spi->Instance->CR1 &= ~SPI_CR1_BR;    // prescaler reset
	m_spi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_4; //prescaler 8
	__HAL_SPI_ENABLE(m_spi);              // SPI enable
}

void SDcard::SPI_speed_175kHz(void) {
	__HAL_SPI_DISABLE(m_spi);             // SPI disable
	m_spi->Instance->CR1 &= ~SPI_CR1_BR;    // prescaler reset
	m_spi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_256; //prescaler 128
	__HAL_SPI_ENABLE(m_spi);              // SPI enable
}

void SDcard::SPI_speed_350kHz(void) {
	__HAL_SPI_DISABLE(m_spi);             // SPI disable
	m_spi->Instance->CR1 &= ~SPI_CR1_BR;    // prescaler reset
	m_spi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_128; //prescaler 128
	__HAL_SPI_ENABLE(m_spi);              // SPI enable
}

void SDcard::SPI_speed_1_4MHz(void) {
	__HAL_SPI_DISABLE(m_spi);             // SPI disable
	m_spi->Instance->CR1 &= ~SPI_CR1_BR;    // prescaler reset
	m_spi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_32; //prescaler 64
	__HAL_SPI_ENABLE(m_spi);              // SPI enable
}
void SDcard::SPI_speed_3MHz(void) {
	__HAL_SPI_DISABLE(m_spi);             // SPI disable
	m_spi->Instance->CR1 &= ~SPI_CR1_BR;    // prescaler reset
	m_spi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_16; //prescaler 64
	__HAL_SPI_ENABLE(m_spi);
}
void SDcard::SPI_speed_6MHz(void) {
	__HAL_SPI_DISABLE(m_spi);             // SPI disable
	m_spi->Instance->CR1 &= ~SPI_CR1_BR;    // prescaler reset
	m_spi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_8; //prescaler 64
	__HAL_SPI_ENABLE(m_spi);
}

uint8_t SDcard::SPI_write(uint8_t data) {
	uint8_t receivedData = 0;

	if (HAL_SPI_TransmitReceive(m_spi, &data, &receivedData, 1, HAL_MAX_DELAY)
			!= HAL_OK) {
		std::cout << "spi error code:" << m_spi->ErrorCode << "\r" << std::endl;
		Error_Handler();
	} else {
		//std::cout<<"received data="<<std::hex <<static_cast<int>(receivedData)<<"\r"<<std::endl;

	}

	return receivedData;
}
/************************************************************************
 MBR과 PBR sector를 차례대로 읽어 loot 디렉터리 섹터 찾기
 출력:
 First_FAT_Sector : FAT 시작 섹터
 RootDirSector : 루트 디렉터리 섹터
 SecPerClus : 클러스터당 섹터 수
 ************************************************************************/

void SDcard::FAT_Init(void) {
	uint32_t StartLBA;		// PBR 섹터 주소
	struct PartTable *PartTable_ptr;		// MBR sector partition table 구조체
	struct BootSector *BootSector_ptr;	// PBR sector 구조체
	m_lcd.string(10, 160, Green, Black, " ******************************");
	m_lcd.string(10, 180, Magenta, Black, "         FAT32 initialize...  ");
	m_lcd.string(10, 200, Green, Black, " ******************************");

	std::cout << "parttable size:" << std::dec << sizeof(PartTable) << "\r"
			<< std::endl;
	std::cout << "BootSector size:" << std::dec << sizeof(BootSector) << "\r"
			<< std::endl;

	std::cout << "FAT init start\r" << std::endl;
	SD_Read(MBR);						// MBR 섹터(0번섹터)를 읽어 버퍼에 저장
	m_lcd.string(10, 240, Green, Black, " ******************************");
	// MBR 섹터의 첫번째 파티션 테이블을 저장
	// 446바이트는 Boot code이고, 446번부터 16바이트씩 파티션#1, #2, #3...
	PartTable_ptr = (struct PartTable*) (buffer + 446);

	StartLBA = PartTable_ptr->LBA_Begin;        // PBR sector address

	SD_Read(StartLBA);                            // PBR sector read
	BootSector_ptr = (struct BootSector*) buffer; // save PBR sector in structure

	// root dir sector 계산
	First_FAT_Sector = StartLBA + BootSector_ptr->BPB_RsvdSecCnt;
	RootDirSector = First_FAT_Sector
			+ BootSector_ptr->BPB_FATSz32 * BootSector_ptr->BPB_NumFATs;

	if (BootSector_ptr->BPB_FATSz32 != 0) {
		std::cout << "FAT32 confirmed\r" << std::endl;
		std::cout << "BPB_FATSz32:" << std::dec << BootSector_ptr->BPB_FATSz32
				<< "\r" << std::endl;
	} else
		std::cout << "It is not FAT32\r" << std::endl;

	// cluster 당 sector 수 저장
	SecPerClus = BootSector_ptr->BPB_SecPerClus;
	std::cout << "secperClus" << std::hex << (int) SecPerClus << "\r"
			<< std::endl;
	m_lcd.string(50, 220, Green, Black, "FAT32 initialized!");
}
/************************************************************************
 Directory Entry는 하나의 섹터에 16개 존재
 Directory Entry는 파일의 개수만큼 생성됨
 Directory Entry는 32바이트로 구성
 Directory Entry는 Cluster Number(파일의 시작위치)를 가지고 있음
 Directory Entry에서 Cluster Number 추출
 return :
 numOfFile : 파일 수
 출력(전역변수) :
 fileStartClust[num_File] : 각 파일의 시작 클러스터
 FileName[] : 파일명
 file_len : 파일길이 섹터 수
 ************************************************************************/

uint16_t SDcard::fatGetDirEntry(void) {
	struct DirEntry *DE_ptr = 0;
	struct DirEntryLong *DEL_ptr = 0;
	uint16_t num_File = 0;			// 파일 개수
	uint16_t clusterHI;				// 디렉터리 엔트리 HI
	uint16_t clusterLO;
	uint32_t clusterNumber;			// 클러스터 넘버
	uint16_t i, j, k, flag;
	uint32_t RD_clust, RD_sector;
	uint8_t lfnIndex = 0;

	RD_clust = 2;		// 루트 디렉터리 시작 클러스터

	while (1) {
		RD_sector = fatClustToSect(RD_clust);
		flag = 1;
		while (flag) {
			for (k = 0; k < SecPerClus; k++) {
				// 루트 디렉터리 섹터 읽기
				SD_Read(RD_sector);
				// 디렉터리 entry 저장
				DE_ptr = (struct DirEntry*) buffer;

				// 32바이트 단위로 디렉터리 entry 16개씩 조사
				for (j = 0; j < 16; j++) {
					if (DE_ptr->DIR_Name[0] == 0x00) {
						flag = 0;
						break;
					}

					//Deleted file pass
					if (DE_ptr->DIR_Name[0] != 0xE5) {

						//LFN check
						if (DE_ptr->DIR_Attr == 0x0F) {
							DEL_ptr = (struct DirEntryLong*) buffer;

							if (DEL_ptr->DIR_order & 0x40) {
								// 긴 이름의 첫 엔트리
								std::fill(std::begin(FileLongName[num_File]),
										std::end(FileLongName[num_File]), 0);
								lfnIndex = 0;
							}

							// 긴 이름 조합
							std::copy(DEL_ptr->DIR_Name_1,
									DEL_ptr->DIR_Name_1 + 10,
									FileLongName[num_File] + lfnIndex);
							lfnIndex = lfnIndex + 10;
							std::copy(DEL_ptr->DIR_Name_2,
									DEL_ptr->DIR_Name_2 + 12,
									FileLongName[num_File] + lfnIndex);
							lfnIndex = lfnIndex + 12;
							std::copy(DEL_ptr->DIR_Name_3,
									DEL_ptr->DIR_Name_3 + 4,
									FileLongName[num_File] + lfnIndex);
							lfnIndex = lfnIndex + 4;

						}

						else if (DE_ptr->DIR_Attr == 0x20
								&&			// 일반 파일
								((DE_ptr->DIR_Ext[0] == 'm')
										|| (DE_ptr->DIR_Ext[0] == 'M'))
								&&		// bmp 파일일시
								((DE_ptr->DIR_Ext[1] == 'p')
										|| (DE_ptr->DIR_Ext[1] == 'P'))
								&& DE_ptr->DIR_Ext[2] == '3') {
							// clusterHI:LO 설정
							clusterHI = DE_ptr->DIR_FstClusHI;
							clusterLO = DE_ptr->DIR_FstClusLO;

							clusterNumber = (uint32_t) (clusterHI);	// 16bit->32bit
							clusterNumber <<= 16;

							// cluster 넘버 추출
							clusterNumber |= (uint32_t) (clusterLO);

							// 시작 cluster 저장
							fileStartClust[num_File] = clusterNumber;

							// 파일명 저장
							for (i = 0; i < 8; i++) {
								FileName[num_File][i] = DE_ptr->DIR_Name[i];
							}
							FileName[num_File][8] = '\0';

							// 파일길이 섹터수(연주 종료조건으로 사용)
							file_len[num_File] = (DE_ptr->DIR_FileSize / 512)
									+ 1;

							num_File++;
							if (num_File >= MAX_NUM_FILE) {
								return num_File;
							}
						}
					}
					DE_ptr++;		// 디렉터리 entry pointer 증가
				}
				RD_sector++;
			}
		}
		RD_clust = FAT_NextCluster(RD_clust);
		if (RD_clust == 0)
			break;
	}
	return num_File;
}
/************************************************************************
 클러스터 넘버를 실제주소로 변환
 첫번째 파일의 클러스터 넘버는 언제나 3번 클러스터
 ************************************************************************/

uint32_t SDcard::fatClustToSect(uint32_t cluster) {
	return ((cluster - 2) * SecPerClus + RootDirSector);
}
/************************************************************************
 현재 cluster로부터 다음 cluster계산
 Arguments	: 현재 cluster 번호
 Returns		: 다음 cluster 번호
 ************************************************************************/

uint32_t SDcard::FAT_NextCluster(uint32_t cluster) {
	uint32_t fatOffset, sector, offset, I_off;
	uint32_t *I_buf, next_cluster;

	fatOffset = cluster << 2;		// 각 cluster마다 4Byte 차지

	// 현재의 클러스터 값으로부터 FAT 에서의 sector와 offset값 산출
	sector = First_FAT_Sector + (fatOffset / u_charPerSector);
	// 바이트 단위로 몇번째인지 계산
	offset = fatOffset % u_charPerSector;
	// 4바이트씩 나눌 때 몇 번째인지 계산
	I_off = offset >> 2;

	SD_Read(sector);			// sector  값 읽기
	I_buf = (uint32_t*) buffer;	// 읽은 데이터 4바이트 단위로 처리
	next_cluster = I_buf[I_off];	// next cluster값 읽기

	if (next_cluster == EndOfCluster)
		next_cluster = 0;		// 클러스터 종료

	return next_cluster;
}
//
//static SDcard &sdCardDriver=SDcard::getInstance();

//SDcard* SDcard::instance=nullptr;
extern SPI_HandleTypeDef hspi2;
extern "C" {
void sd_initialize() {
	SDcard &sdCardDriver = SDcard::getInstance();

	sdCardDriver.SD_Init(&hspi2);
}

int sd_read(uint32_t sector, uint8_t *buffer) {
	SDcard &sdCardDriver = SDcard::getInstance();
	return sdCardDriver.SD_Read(sector, buffer);
}

}

