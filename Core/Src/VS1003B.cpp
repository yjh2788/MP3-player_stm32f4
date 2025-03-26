/*
 * VS1003B.cpp
 *
 *  Created on: Dec 5, 2024
 *      Author: spc
 */

#include "VS1003B.h"
#include "Youn_s_lab_board_info.h"
#include "TFT_LCD.h"
#include <iostream>

VS1003B::VS1003B() {

}
VS1003B::~VS1003B() {

}
inline void VS1003B::vs1003b_set_cs() {
	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XCS, GPIO_PIN_SET);
}

inline void VS1003B::vs1003b_clear_cs() {
	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XCS, GPIO_PIN_RESET);
}

inline void VS1003B::vs1003b_set_dcs() {
	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XDCS, GPIO_PIN_SET);
}

inline void VS1003B::vs1003b_clear_dcs() {
	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XDCS, GPIO_PIN_RESET);
}

inline void VS1003B::vs1003b_set_xrst() {
	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XRESET, GPIO_PIN_SET);
}

inline void VS1003B::vs1003b_clear_xrst() {
	HAL_GPIO_WritePin(vs1003_PORT, VS1003B_XRESET, GPIO_PIN_RESET);
}

void VS1003B::vs1003b_spi_send(uint8_t data) {
	if (HAL_SPI_Transmit(m_spi, &data, 1, HAL_MAX_DELAY) != HAL_OK) {
		Error_Handler();
	}
}

uint8_t VS1003B::vs1003b_spi_receive() {
	uint8_t receivedData = 0;
	if (HAL_SPI_Receive(m_spi, &receivedData, 1, HAL_MAX_DELAY) != HAL_OK) {
		Error_Handler();
	}
	return receivedData;
}

void VS1003B::SPI_speed_11MHz(void) {
	__HAL_SPI_DISABLE(m_spi);             // SPI disable
	m_spi->Instance->CR1 &= ~SPI_CR1_BR;    // prescaler reset
	m_spi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_4; //prescaler 8
	__HAL_SPI_ENABLE(m_spi);              // SPI enable
}

void VS1003B::SPI_speed_350kHz(void) {
	__HAL_SPI_DISABLE(m_spi);             // SPI disable
	m_spi->Instance->CR1 &= ~SPI_CR1_BR;    // prescaler reset
	m_spi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_128; //prescaler 128
	__HAL_SPI_ENABLE(m_spi);              // SPI enable
}

void VS1003B::SPI_speed_1_4MHz(void) {
	__HAL_SPI_DISABLE(m_spi);             // SPI disable
	m_spi->Instance->CR1 &= ~SPI_CR1_BR;    // prescaler reset
	m_spi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_32; //prescaler 64
	__HAL_SPI_ENABLE(m_spi);              // SPI enable
}
void VS1003B::SPI_speed_3MHz(void) {
	__HAL_SPI_DISABLE(m_spi);             // SPI disable
	m_spi->Instance->CR1 &= ~SPI_CR1_BR;    // prescaler reset
	m_spi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_16; //prescaler 32
	__HAL_SPI_ENABLE(m_spi);
}
void VS1003B::SPI_speed_6MHz(void) {
	__HAL_SPI_DISABLE(m_spi);             // SPI disable
	m_spi->Instance->CR1 &= ~SPI_CR1_BR;    // prescaler reset
	m_spi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_8; //prescaler 16
	__HAL_SPI_ENABLE(m_spi);
}

uint8_t VS1003B::SPI_write(uint8_t data) {
	uint8_t receivedData = 0;

	if (HAL_SPI_TransmitReceive(m_spi, &data, &receivedData, 1, HAL_MAX_DELAY)
			!= HAL_OK) {
		Error_Handler();
	}
//	else{
//			std::cout<<"received data="<<std::hex <<static_cast<int>(receivedData)<<"\r"<<std::endl;
//	}
	// 수신된 데이터가 0xFF일 경우 유효한 응답을 받을 때까지 기다리기
//	while (receivedData == 0xFF) {
//		receivedData = SPI2->DR;  // 새로운 데이터 수신 대기
//	}

	return receivedData;
}

bool VS1003B::vs1003b_is_busy(void) {
	bool status =
			(HAL_GPIO_ReadPin(vs1003_PORT, VS1003B_DREQ) == GPIO_PIN_RESET) ? 1 : 0;
	return status;
}

void VS1003B::vs1003b_hardware_reset(void) {
	vs1003b_clear_xrst();
	HAL_Delay(2);
	vs1003b_set_xrst();
}

// -- Internal command and data wrangling --
void VS1003B::vs1003_write_reg(uint8_t reg, uint16_t data) {
	while ((vs1003b_is_busy()))
		;

	vs1003b_clear_cs();

	SPI_write(COMMAND_WRAM);
	SPI_write(reg);
	SPI_write((data >> 8) & 0xFF);
	SPI_write(data & 0xFF);

	vs1003b_set_cs();
}

void VS1003B::vs1003b_wait_while_busy(void) {
	while (vs1003b_is_busy())
		;
}

void VS1003B::vs1003b_command_write(uint8_t reg, uint16_t data) {

	vs1003b_clear_cs();
	vs1003b_spi_send(0x02);

	vs1003b_spi_send(reg);

	vs1003b_spi_send((data & 0xFF00) >> 8);
	vs1003b_spi_send(data & 0x00FF);
	vs1003b_set_cs();

}

uint16_t VS1003B::vs1003b_command_read(uint8_t reg) {

	vs1003b_clear_cs();
	vs1003b_spi_send(0x03);
	vs1003b_spi_send(reg);

	uint8_t high = vs1003b_spi_receive();
	uint8_t low = vs1003b_spi_receive();

	vs1003b_set_cs();
	return (((uint16_t) high) << 8) | low;
}

void VS1003B::vs1003b_command_bass(uint16_t control) {
	vs1003b_current_bass = control;

	vs1003b_command_write(COMMAND_BASS, control);
	vs1003b_wait_while_busy();
}

void VS1003B::vs1003b_command_volume(uint16_t volume) {
	vs1003b_current_volume = volume;

	vs1003b_command_write(COMMAND_VOLUME, volume);
	vs1003b_wait_while_busy();
}

void VS1003B::vs1003b_data_write(uint8_t *data, uint8_t n) {

	vs1003b_clear_dcs();
	while (n-- > 0)
		vs1003b_spi_send(*data++);

	vs1003b_set_dcs();
}

void VS1003B::vs1003b_data_empty() {
	if (vs1003b_is_busy())
		return;
	vs1003b_clear_dcs();

	for (int i = 0; i < 32; i++)
		vs1003b_spi_send(0x00);
	vs1003b_set_dcs();

}

void VS1003B::vs1003b_init_registers(void) {
	vs1003b_wait_while_busy();

	/* Set a clock multiplier of 4.5x.
	 This is the highest supported clock and will allow the most simultaneous features.
	 It will also draw the most power.
	 */
	vs1003b_command_write(COMMAND_CLOCKF, 0xE000);
	vs1003b_wait_while_busy();

	vs1003b_command_volume(vs1003b_current_volume);
	vs1003b_command_bass(vs1003b_current_bass);
}

// -- Public VS1003B functions --

bool VS1003B::vs1003b_init(SPI_HandleTypeDef *spi) {
	m_spi = spi;
	TFT_LCD &lcd = TFT_LCD::getInstance();
	if(DEBUG) lcd.string(10, 260, Cyan, Black, " VS1003b initialize...");

	volume = 50;					// initial volume = 200
	bass = 0;					// initial bass and treble = 0
	treble = 0;

	SPI_speed_350kHz();

	vs1003b_reset();
	HAL_Delay(2);
	if(DEBUG) lcd.string(10, 280, Cyan, Black, "VS1003b reset");
	vs1003b_set_volume(volume, volume);
	while (vs1003b_is_busy())
		;
	vs1003b_set_bass_treble(bass, 100, treble, 5);
	while (vs1003b_is_busy())
		;
	vs1003b_command_write(COMMAND_CLOCKF, 0xe000); // Experimenting with higher clock settings
	while (vs1003b_is_busy())
		;
	vs1003b_command_write(COMMAND_AUDATA, 44100); // 44.1kHz stereo
	//while(vs1003b_is_busy());

	// vs1003b_command_write(COMMAND_VOLUME,0x2020); // VOL
	//
	// soft reset
	// vs1003b_command_write(COMMAND_MODE, _BV(SM_SDINEW) | _BV(SM_RESET));

	// await_data_request();

	//SPI_mode0_4MHz();

	//	VS1003_RegWrite(VS1003_CLOCKF_REG, 0x9800);
	//	VS1003_RegWrite(VS1003_INT_FCTLH_REG, 0x8008);
	//b8
	//vs1003b_command_write(COMMAND_CLOCKF,0xe000);
	// Make sure we're really using a VS1003B
	uint16_t mode = vs1003b_command_read(COMMAND_MODE);
	uint16_t status = vs1003b_command_read(COMMAND_STATUS);
	uint16_t clk = vs1003b_command_read(COMMAND_CLOCKF);
	uint16_t bassre = vs1003b_command_read(COMMAND_BASS);
	uint16_t vol = vs1003b_command_read(COMMAND_VOLUME);

	//TFT_hexadecimal(mode, 4);
	//TFT_hexadecimal(status,4);
	//TFT_hexadecimal(clk,4);
	//TFT_hexadecimal(bassre,4);
	//TFT_hexadecimal(vol,4);
	//while(1);
	//HAL_Delay(1000);

	if (((status & 0x0030) != 0x0030) || !(mode & 0x0800)) {
		if(DEBUG) lcd.string(10, 300, Cyan, Black, " VS1003b initialize failed 1");
		std::cout << "vs1003 fail\r" << std::endl;
		while (1)
			;
		return false;
	}
	if ((status == 0xff) || (mode == 0xff)) {
		if(DEBUG) lcd.string(10, 300, Cyan, Black, " VS1003b initialize failed 2");
		std::cout << "vs1003 fail\r" << std::endl;
		while (1)
			;
		return false;

	}
	if(DEBUG) lcd.string(10, 300, Cyan, Black, " VS1003b initialized");
	std::cout << "vs1003 init success\r" << std::endl;
	return true;
}

void VS1003B::vs1003b_reset(void) {
	vs1003b_hardware_reset();
	vs1003b_init_registers();
}

bool VS1003B::vs1003b_ready(void) {
	return !vs1003b_is_busy();
}

uint16_t VS1003B::vs1003b_feed_data(uint8_t *data, uint16_t count) {
	uint16_t processed = 0;
	while (count > 0) {
		if (!vs1003b_ready())
			break;

		uint8_t n = MIN(count, 32);

		vs1003b_data_write(data, n);

		processed += n;
		count -= n;
		data += n;
	}

	return processed;
}

void VS1003B::vs1003b_stop(void) {
	// Mute
	vs1003b_command_write(COMMAND_VOLUME, 0xFEFE);

	// Send stop command and write 0x00 until the format field has been cleared (= no current sound). MP3 can be stopped immediately.
	vs1003b_command_write(COMMAND_MODE, 0x8008);
	vs1003b_format_t format = vs1003b_get_format();
	while ((format != kVS1003BFormatNone) && (format != kVS1003BFormatMP3)) {
		vs1003b_data_empty();
		format = vs1003b_get_format();
	}

	// Reset the VS1003B, just to be sure a broken format didn't mess anything up
	vs1003b_reset();
}

void VS1003B::vs1003b_set_volume(uint8_t left, uint8_t right) {
	// 0 is full volume, 254 is silence
	left = 254 - MIN(left, 254);
	right = 254 - MIN(right, 254);

	vs1003b_command_volume(((uint16_t) left << 8) | right);
}

void VS1003B::vs1003b_get_volume(uint8_t *left, uint8_t *right) {
	*left = 254 - ((vs1003b_current_volume & 0xFF00) >> 8);
	*right = 254 - (vs1003b_current_volume & 0x00FF);
}

uint16_t VS1003B::vs1003b_get_decode_time(void) {
	return vs1003b_command_read(COMMAND_DECODETIME);
}

vs1003b_format_t VS1003B::vs1003b_get_format(void) {
	uint16_t header1 = vs1003b_command_read(COMMAND_HEADER1);

	if (header1 == 0x0000)
		return kVS1003BFormatNone;
	if (header1 == 0x7665)
		return kVS1003BFormatWAV;
	if (header1 == 0x574D)
		return kVS1003BFormatWMA;
	if (header1 == 0x4D54)
		return kVS1003BFormatMID;

	if ((header1 >> 5) == 0x7FF)
		return kVS1003BFormatMP3;

	return kVS1003BFormatUnknown;
}

bool VS1003B::vs1003b_set_bass_treble(uint8_t bass_amplitude,
		uint8_t bass_freqlimit, int8_t treble_amplitude,
		uint8_t treble_freqlimit) {
	if (bass_amplitude > 15)
		return false;
	if ((bass_freqlimit < 20) || (bass_freqlimit > 150))
		return false;
	if ((treble_amplitude < -8) || (treble_amplitude > 7))
		return false;
	if (treble_freqlimit > 15)
		return false;

	bass_freqlimit /= 10;

	uint16_t control = (((uint16_t) ((uint8_t) treble_amplitude)) << 12)
			| (((uint16_t) treble_freqlimit) << 8) | (bass_amplitude << 4)
			| (bass_freqlimit);

	VS1003B::vs1003b_command_bass(control);
	return true;
}

