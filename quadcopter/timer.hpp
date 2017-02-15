//
//  timer.hpp
//  quadcopter
//
//  Created by Muhsin Fatih Yorulmaz on 15/02/2017.
//  Copyright © 2017 mekatrotekno. All rights reserved.
//

#ifndef timer_hpp
#define timer_hpp

#include <stdio.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include <math.h>


// timer is at microseconds resolution. enum values can be used to multiply with ticks to get human readable results
typedef enum timeinterval{
	microseconds = 1, milliseconds = 1000, seconds = 1000000
} timeinterval;

class timer {
	static int blabla;
private:
	
	
	
public:
	timer();
	uint32_t elapsedtime(timeinterval interval);
	void startAsyncStopwatch();
	
};



static void stopAsyncTimer();

static void delay(__IO uint32_t milliseconds);
static void delay_micro(__IO uint32_t microseconds);

void enableSysTick();
void disableSysTick();
static void setSysTick();

#endif /* timer_hpp */
