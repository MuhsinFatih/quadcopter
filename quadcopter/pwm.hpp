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
	uint32_t _periodInTicks;
	uint32_t prescaler;
	uint32_t clockSpeed = 84000000;
public:
	
	double period() const { return _period; }
	double frequency() const { return _frequency; }
	uint32_t periodInTicks() const {return _periodInTicks;}
	
	/**
	 set period in real world terms

	 @param period period in seconds
	 */
	void period(const double &period) {
		_frequency = 1.0 / period;
		_period = period;
		_periodInTicks = period * clockSpeed / prescaler;
		TIM4->ARR = _periodInTicks;
		TIM4->EGR = TIM_PSCReloadMode_Immediate;
	}
	
	/**
	 set frequency in real world terms

	 @param frequency frequency (times / sec)
	 */
	void frequency(const double &frequency) {
		_frequency = frequency;
		_period = 1.0 / frequency;
		_periodInTicks = clockSpeed / (prescaler * frequency);
		TIM4->ARR = _periodInTicks;
		TIM4->EGR = TIM_PSCReloadMode_Immediate;
	}
	

	/**
	 set period of pwm timer precisely
	 
	 @param periodInTicks period in ticks. how many ticks until reset?
	 */
	void periodInTicks(const uint32_t &periodInTicks) {
		_frequency = clockSpeed / (prescaler * periodInTicks);
		_period = periodInTicks * prescaler / clockSpeed;
		_periodInTicks = periodInTicks;
		TIM4->ARR = periodInTicks;
		TIM4->EGR = TIM_PSCReloadMode_Immediate;
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
