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
	
	GPIO_TypeDef *GPIOx;
	int numOfPins;
	int *pins;
public:
	pwm();
	pwm(GPIO_TypeDef *GPIOx, int pin);
	pwm(GPIO_TypeDef *GPIOx, std::vector<int> pins, int numOfPins);
	
	
	/**
	 setup pwm pins without creating an insance of pwm class
	 */
	void setupPWM(GPIO_TypeDef *GPIOx, int *pins, int numOfPins);
	void write();
	
};

#endif /* pwm_hpp */
