//
//  usart.hpp
//  quadcopter
//
//  Created by Muhsin Fatih Yorulmaz on 11/02/2017.
//  Copyright © 2017 mekatrotekno. All rights reserved.
//

#ifndef usart_hpp
#define usart_hpp

#include <stdio.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include <string>
#include <vector>

class usart {
private:
	
	USART_TypeDef USARTx;
	GPIO_TypeDef *GPIOx;
	int rx, tx;
	void setup_USART(int rx, int tx);
	void usart_puts(USART_TypeDef *USARTx, volatile char *str);
	
public:
	usart(USART_TypeDef *USARTx, GPIO_TypeDef *GPIOx, int rx, int tx);
	void printf(const char *, ...);

	/**
	 read from usart
	 DON'T FORGET TO DEALLOCATE!!
	 @return pointer to dynamically allocated char array
	 */
	char* read();
};

#endif /* usart_hpp */
