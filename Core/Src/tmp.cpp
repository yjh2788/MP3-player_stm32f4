/*
 * tmp.cpp
 *
 *  Created on: Feb 26, 2025
 *      Author: spc
 */




static void MP3_Play(int f_no) {
	sd.SPI_speed_11MHz();
	vs.vs1003b_set_volume(50, 50);
	play_state = 1;
	stop_flag = 0;
//TFT_color_screen(Black);
//frame();
//TFT_string(0,0,Cyan, Black,"it is playing");
//TFT_string(0,30,Cyan,Black,(char*)FileName[f_no]);
	u_long c_clust;
	u_long rd_sec;
	int i, n, format;
	u_long k = 0, kd;
	cout << sd.FileName[f_no] << "\r" << endl;
	cout << sd.FileLongName[f_no] << "\r" << endl;
	const char16_t *utf16Data =
			reinterpret_cast<const char16_t*>(sd.FileLongName[f_no]);
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	// 변환 수행
	std::string utf8String = converter.to_bytes(utf16Data, utf16Data + 130);

	// UTF-8로 출력
	std::cout << utf8String << "\r" << std::endl;

	kd = sd.file_len[f_no];                            // 파일 길이 섹터수
	c_clust = sd.fileStartClust[f_no];                        // f_no 파일 시작 클러스터
	while (1) {
		rd_sec = sd.fatClustToSect(c_clust);    // 현재 클러스터를 섹터로 변환
		for (n = 0; n < sd.SecPerClus; n++)    // 클러스터당 섹터 수만큼 읽기
				{

			sd.SD_Read(rd_sec);			// 섹터(512바이트) 읽기

			// 읽은 데이터 VS1033칩에 출력
			for (i = 0; i < 512; i++) {
				// VS1033 데이터수신가능 체크
				while ((vs.vs1003b_is_busy()))
					;
				//vs.vs1003b_clear_dcs();//VS1033_xDCS = 0;     // xDCS = low, VS1033 활성화
				HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XDCS, GPIO_PIN_RESET);
				vs.vs1003b_spi_send(sd.buffer[i]);		// VS1033 칩으로 MP3 데이터 전송
				//vs.vs1003b_set_dcs();//VS1033_xDCS = 1;      // xDCS = high, VS1033 비활성화
				HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XDCS, GPIO_PIN_SET);

				//set_vol (volume_flag);
				if (change_flag) {
					change_flag = 0;
					return;
				}
			}
			k++;
			if (k >= kd)
				return;

			rd_sec++;                     // 섹터 번호 +1
			//if(stop_play == 0) return;    // 연주 정지
			if (home_flag == 1)
				return;

			if (stop_flag == 1) {
				while (1) {
					cout << "resume\r" << endl;

					//TFT_string_size(100, 22, Magenta, Black, " resume ", 2, 2);
					if (stop_flag == 0) {
						cout << "playing\r" << endl;
						//TFT_string_size(100, 22, Magenta, Black, " playing ", 2, 2);
						break;
					}
				}
			}

			if ((p_flag == 1) && (play_again)) {
				cout << "repeat\r" << endl;
				//TFT_string_size(100, 230, Magenta, Black, " repeat ", 2, 2);
				p_flag = 0;
			}
			if ((p_flag == 1) && (!play_again)) {
				//cout << "repeat\r" << endl;
				//TFT_string_size(100, 230, Magenta, Black, "            ", 2, 2);
				p_flag = 0;

			}

		}
		c_clust = sd.FAT_NextCluster(c_clust);    // 다음 cluster계산
		if (c_clust == 0)
			break;    // 마지막 클러스터이면 종료
	}

}
