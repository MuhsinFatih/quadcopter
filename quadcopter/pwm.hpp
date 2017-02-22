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
	
	double _period, _frequency;
	uint32_t prescaler;
public:
	
	double period() const { return _period; }
	double frequency() const { return _frequency; }
	void period(const double &period) {
		_period = period;
		_frequency = 0;
	}
	void frequency(const double &frequency) {
		_frequency = frequency;
		_period = 0;
	}
	
	GPIO_TypeDef *GPIOx;
	int numOfPins;
	int pin;
	volatile uint32_t *CCR;
	
	pwm();
	pwm(GPIO_TypeDef *GPIOx, int pin, volatile uint32_t *CCR);
	
	
	/**
	 setup pwm pins without creating an insance of pwm class
	 */
	void setupPWM(GPIO_TypeDef *GPIOx, int *pins, int numOfPins);
	void write(uint32_t value);
	
};

#endif /* pwm_hpp */
