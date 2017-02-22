//
//  pwm.hpp
//  quadcopter
//
//  Created by Muhsin Fatih Yorulmaz on 18/02/2017.
//  Copyright © 2017 mekatrotekno. All rights reserved.
//

#ifndef pwm_hpp
#define pwm_hpp

#include <stdio.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include <math.h>
#include <vector>
#include "main.hpp"

class pwm {
private:
	
public:
	
	GPIO_TypeDef *GPIOx;
	int numOfPins;
	int *pins;
	volatile uint32_t *CCR;
	
	pwm();
	pwm(GPIO_TypeDef *GPIOx, int pin, volatile uint32_t *CCR);
	pwm(GPIO_TypeDef *GPIOx, std::vector<int> pins, int numOfPins, volatile uint32_t *CCR);
	
	
	/**
	 setup pwm pins without creating an insance of pwm class
	 */
	void setupPWM(GPIO_TypeDef *GPIOx, int *pins, int numOfPins);
	void write(uint32_t value);
	
};

#endif /* pwm_hpp */
