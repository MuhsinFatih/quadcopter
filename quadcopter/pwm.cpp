//
//  pwm.cpp
//  quadcopter
//
//  Created by Muhsin Fatih Yorulmaz on 18/02/2017.
//  Copyright © 2017 mekatrotekno. All rights reserved.
//

#include "pwm.hpp"
using namespace std;


pwm::pwm() {
	
}
pwm::pwm(GPIO_TypeDef *GPIOx, int pin, volatile uint32_t *CCR){
	this->GPIOx = GPIOx;
	this->numOfPins = 1;
	this->pins = new int[1]{pin};
	this->CCR = CCR;
	setupPWM(GPIOx, this->pins, this->numOfPins);
}


/**
 pwm
 
 @param GPIOx gpio block
 @param pins array of pins. Numbers. not pinx notation. eg: {1,2,3,4}
 @param numOfPins size of the pins array
 @param CCR capture/compare register (pass it's address in! like &TIM4->CCR1) to write duty cycle. Look at datasheet to find the matching register.
 */
pwm::pwm(GPIO_TypeDef *GPIOx, vector<int> pins, int numOfPins, volatile uint32_t *CCR){
	this->GPIOx = GPIOx;
	this->numOfPins = pins.size();
	this->pins = (int*)malloc(numOfPins * sizeof(int));
	this->CCR = CCR;
	
	REP(numOfPins){
		this->pins[i] = pins[i];
	}
	setupPWM(GPIOx, this->pins, this->numOfPins);
	
}
/**
 setup pwm
 
 @param pins numbers. not pinx notation. eg: {1,2,3,4}
 @param numOfPins size of the pins array
 */
void pwm::setupPWM(GPIO_TypeDef *GPIOx, int *pins, int numOfPins) {
	GPIO_InitTypeDef			gpioStructure;
	TIM_TimeBaseInitTypeDef		timeBaseStructure;
	TIM_OCInitTypeDef			outputControlStrucure;
	
	// enable timer 4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	char asdf[50];
	uint32_t gpioPins = 0;
	REP(numOfPins){
		gpioPins |= (uint32_t)pow(2,pins[i]);
	}
	
	gpioStructure.GPIO_Pin = gpioPins;
	gpioStructure.GPIO_Mode = GPIO_Mode_AF;
	gpioStructure.GPIO_Speed = GPIO_Speed_100MHz;
	gpioStructure.GPIO_OType = GPIO_OType_PP;
	gpioStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOx, &gpioStructure);
	
	
	// assign alternate function
	REP(numOfPins){
	GPIO_PinAFConfig(GPIOx, (uint8_t) pins[i], GPIO_AF_TIM4);
	}
//	GPIO_PinAFConfig(GPIOx, GPIO_PinSource7, GPIO_AF_TIM4);
	
	
	uint16_t prescaler = (uint16_t)84;
	
	timeBaseStructure.TIM_Period		= 19999;
	timeBaseStructure.TIM_Prescaler		= prescaler;
	timeBaseStructure.TIM_ClockDivision	= 0;
	timeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;	// count 0 -> cnt
	
	TIM_TimeBaseInit(TIM4, &timeBaseStructure);
	
	// common timer settings
	// pwm mode 1: set on compare match
	// pwm mode 2: clear on compare match
	outputControlStrucure.TIM_OCMode		= TIM_OCMode_PWM1;
	outputControlStrucure.TIM_OutputState	= TIM_OutputState_Enable;
	outputControlStrucure.TIM_Pulse			= 0;
	outputControlStrucure.TIM_OCPolarity	= TIM_OCPolarity_High;
	
	TIM_OC1Init(TIM4, &outputControlStrucure);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
	
	TIM_OC2Init(TIM4, &outputControlStrucure);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	
	TIM_ARRPreloadConfig(TIM4, ENABLE);
	
	TIM_Cmd(TIM4, ENABLE);
}

void pwm::write(uint32_t value){
	*this->CCR = value;
}



/**
 * @brief  friendly function to get the period value to pass to timers
 * @param  realPeriod: period in means of real life measurements. like 1 is actually 1 second
 * @param  frequency: 1/realPeriod obviously. If you want to use this then set realPeriod to 0. or set this to 0 likewise
 * @retval period value to pass into your timer
 */
uint16_t getPeriod(double realPeriod, double frequency, uint32_t clockSpeed , uint16_t prescaler) {
	uint16_t period;
	
	if(frequency == 0) {
		period = realPeriod * clockSpeed / prescaler;
	} else if(realPeriod == 0) {
		period = clockSpeed / (prescaler * frequency);
	} else {
		// throw error
	}
	return period;
}
