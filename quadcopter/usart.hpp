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
#include "main.hpp"
#include <math.h>
#include <stdarg.h>
#include <string>

class usart {
private:
	
	USART_TypeDef USARTx;
	int rx, tx;
	void setup_USART(int rx, int tx);
	void usart_puts(USART_TypeDef *USARTx, volatile char *str);
	
public:
	usart(USART_TypeDef *USARTx, int rx, int tx);
	void printf(const char *, ...);
	char* read();
};

#endif /* usart_hpp */
