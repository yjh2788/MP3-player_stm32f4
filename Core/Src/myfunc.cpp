#include "myfunc.h"
#include "main.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdbool.h>

//UART_HandleTypeDef *uart_handle;  // Global variable to store the UART handle
//
//void set_uart_handle(UART_HandleTypeDef *huart) {
//    uart_handle = huart;  // Function to set the UART handle
//}
//
//PUTCHAR_PROTOTYPE
//{
//  // Use the global UART handle for transmission
//  if (uart_handle != NULL) {
//    HAL_UART_Transmit(uart_handle, (uint8_t *)&ch, 1, 0xFFFF);
//  }
//  return ch;
//}
void analog_init(GPIO_TypeDef* GPIOx,uint16_t pin)
{

//1. Enable ADC and GPIO clock
	//RCC->APB2ENR |= (1<<8);  // enable ADC1 clock
	//RCC->AHB1ENR |= (1<<0);  // enable GPIOA clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);

	//2. Set the prescalar in the Common Control Register (CCR)
	ADC->CCR |= 2<<16;  		 // PCLK2 divide by 6.... ADC_CLK = 90/6 = 15MHz
	//ADC config
	ADC1->CR2 = 0|0x001e0000; // ADC 비활성화 (일단 초기화 위해)
	ADC1->CR1 = 0; // 기본 설정 (단일 변환 모드)
	ADC1->SQR1 &= 0xff0fffff;
	ADC1->SQR3 = 0; // 변환 순서: 채널 0 (PA0이 채널 0)
	ADC1->SMPR1 = (3 << 0); // 샘플링 시간 설정 (ADC_SampleTime_56Cycles)
	ADC1->SMPR2;
	// 5. ADC 활성화
	ADC1->CR2 |= ADC_CR2_ADON; // ADC 활성화

	//ADC1->CR2|=ADC_CR2_
}
uint16_t ADC_Read(void)
{
    // 1. ADC 변환 시작 (CR2 레지스터의 SWSTART 비트 설정)
    ADC1->CR2 |= ADC_CR2_SWSTART;

    // 2. 변환 완료를 대기 (SR 레지스터의 EOC 비트 검사)
    while (!(ADC1->SR & ADC_SR_EOC));

    uint32_t res = (uint32_t)ADC1->DR;
    // 4. EOC 비트 클리어 (하드웨어에서 자동으로 클리어되지 않음)
    ADC1->SR &= ~ADC_SR_EOC;  // EOC 비트 클리어

    // 3. 변환 결과 읽기 (DR 레지스터)
    return res;
}

float hal_ADC_read(ADC_HandleTypeDef *hadc)
{
	float value,vout,tmp;

	HAL_ADC_Start(hadc);
	HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
	value = HAL_ADC_GetValue(hadc);
	HAL_ADC_Stop(hadc);

	vout = 3000 * value / 4096;
	tmp = (float)vout / 10.0;
	return tmp;

}
void blink(int num)
{
//	while(num--)
//	{
//		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
//		HAL_Delay(250);
//		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
//		HAL_Delay(250);
//	}
}
void debug_print(char*message)
{
	//std::cout<<message<<std::endl;
}
void sleep_ms(int32_t time)
{
	HAL_Delay(time);
}
