/*
 * VS1003B.h
 *
 *  Created on: Dec 5, 2024
 *      Author: spc
 */

#ifndef VS1003B_H_
#define VS1003B_H_

#include "main.h"
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// VS1003 SCI Write Command byte is 0x02
#define VS_WRITE_COMMAND 0x02

// VS1003 SCI Read COmmand byte is 0x03
#define VS_READ_COMMAND  0x03

// SCI Registers

// SCI_MODE bits

// Commands
#define COMMAND_MODE       0x00
#define COMMAND_STATUS     0x01
#define COMMAND_BASS       0x02
#define COMMAND_CLOCKF     0x03
#define COMMAND_DECODETIME 0x04
#define COMMAND_AUDATA     0x05
#define COMMAND_WRAM	   0X06
#define COMMAND_WRAMADDR   0X07
#define COMMAND_HEADER0    0x08
#define COMMAND_HEADER1    0x09
#define COMMAND_AIADDR	   0X0A //어플리케이션 시작 주소
#define COMMAND_VOLUME     0x0B


typedef enum {
	kVS1003BFormatNone = 0,
	kVS1003BFormatWAV,
	kVS1003BFormatMP3,
	kVS1003BFormatWMA,
	kVS1003BFormatMID,
	kVS1003BFormatUnknown
} vs1003b_format_t;

class VS1003B {

private:
	VS1003B();
	~VS1003B();

public:

	static VS1003B& getInstance() {
		static VS1003B instance; // static local variable instance
		return instance;
	}
	VS1003B(const VS1003B&) = delete;
	VS1003B& operator=(const VS1003B&) = delete;

	SPI_HandleTypeDef *m_spi;
	// Remember the current volume and bass/treble setting
	uint16_t vs1003b_current_volume = 0xFEFE;
	uint16_t vs1003b_current_bass = 0x0000;

	uint8_t volume = 0;
	uint8_t bass = 0;
	uint8_t treble = 0;

	inline void vs1003b_set_cs();
	inline void vs1003b_clear_cs();
	inline void vs1003b_set_dcs();
	inline void vs1003b_clear_dcs();
	inline void vs1003b_set_xrst();
	inline void vs1003b_clear_xrst();

	void SPI_speed_350kHz(void);
	void SPI_speed_1_4MHz(void);
	void SPI_speed_3MHz(void);
	void SPI_speed_6MHz(void);
	void SPI_speed_11MHz(void);
	uint8_t SPI_write(uint8_t data);
	void vs1003b_spi_send(uint8_t data);
	uint8_t vs1003b_spi_receive(void);

	bool vs1003b_is_busy(void);
	void vs1003b_hardware_reset(void);
	void vs1003_write_reg(uint8_t reg, uint16_t data);
	void vs1003b_wait_while_busy(void);
	void vs1003b_command_write(uint8_t reg, uint16_t data);
	uint16_t vs1003b_command_read(uint8_t reg);
	void vs1003b_command_bass(uint16_t control);
	void vs1003b_command_volume(uint16_t volume);
	void vs1003b_data_write(uint8_t *data, uint8_t n);
	void vs1003b_data_empty();
	void vs1003b_init_registers(void);

	/** Initialize the VS1003B MP3/WMA/WAV/MID decoder. */
	bool vs1003b_init(SPI_HandleTypeDef *spi);
	void vs1003b_reset(void);
	bool vs1003b_ready(void);
	uint16_t vs1003b_feed_data(uint8_t *data, uint16_t count);
	void vs1003b_stop(void);
	void vs1003b_set_volume(uint8_t left, uint8_t right);
	void vs1003b_get_volume(uint8_t *left, uint8_t *right);
	uint16_t vs1003b_get_decode_time(void);
	vs1003b_format_t vs1003b_get_format(void);
	bool vs1003b_set_bass_treble(uint8_t bass_amplitude, uint8_t bass_freqlimit,
			int8_t treble_amplitude, uint8_t treble_freqlimit);

};

#endif
