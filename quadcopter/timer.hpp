
#ifndef timer_hpp
#define timer_hpp

#include <stdio.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include <math.h>
#include <vector>

// timer is at microseconds resolution. enum values can be used to multiply with ticks to get human readable results
typedef enum timeinterval{
	microseconds = 1, milliseconds = 1000, seconds = 1000000
} timeinterval;


class timer {
private:
	uint32_t offset = 0;
	
public:
	timer();
	uint32_t elapsedTime(timeinterval interval);
	void start();
	/**
	 Doesnt do anything at all. a timer does not actually count. all timers have an offset and use same counter to get elapsed time
	 */
	void stop();
};



/**
 busy wait milliseconds
 */
void delay(__IO uint32_t milliseconds);

/**
 buys wait microseconds
 */
void delay_micro(__IO uint32_t microseconds);


/**
 set systick to microsecond resolution and enable
 */
void setSysTick();
void enableSysTick();
void disableSysTick();

#endif /* timer_hpp */
