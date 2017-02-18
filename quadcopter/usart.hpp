
#ifndef usart_hpp
#define usart_hpp

#include <stdio.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include <string>
#include <vector>
#include "main.hpp"
class usart {
private:
	
	USART_TypeDef *USARTx;
	GPIO_TypeDef *GPIOx;
	int rx, tx;
	int baudrate;
	void setup_USART();
	void usart_puts(USART_TypeDef *USARTx, const char *str);
	
public:
	usart();
	usart(USART_TypeDef *USARTx, GPIO_TypeDef *GPIOx, int rx, int tx, int baudrate);
	void printf(const char *, ...);
	void begin();
	bool available();
	
	/**
	 read from usart
	 DON'T FORGET TO DEALLOCATE!!
	 @return pointer to dynamically allocated char array
	 */
	char* read();
};

#endif /* usart_hpp */
