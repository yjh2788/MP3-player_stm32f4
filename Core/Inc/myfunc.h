#ifndef MYFUNC_H
#define MYFUNC_H
#include "stm32f4xx_hal.h"

#ifdef __cplusplus

void debug_print(char*);
extern "C" {
#endif

#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

class Rect{
private:
public:
    Rect();
    Rect(int x, int y, int width, int height);
    ~Rect();
    int x;
    int y;
    int width;
    int height;
};


//void set_uart_handle(UART_HandleTypeDef *huart);
void blink(int num);
void analog_init(GPIO_TypeDef* GPIOx,uint16_t pin);
uint16_t ADC_Read(void);
float hal_ADC_read(ADC_HandleTypeDef *hadc);
void sleep_ms(int32_t time);
#ifdef __cplusplus
}
#endif

#endif
