//
//  timer.cpp
//  quadcopter
//
//  Created by Muhsin Fatih Yorulmaz on 15/02/2017.
//  Copyright © 2017 mekatrotekno. All rights reserved.
//

#include "timer.hpp"

volatile uint32_t ticks;
static bool asyncTimerOn = false;
uint32_t offset = 0;
timer::timer(){
	if(asyncTimerOn)
		offset = ticks;
}

// microsecond resolution
static void setSysTick() {
	if (SysTick_Config(SystemCoreClock / 1000000)) {
		//		usart_puts(USART2, "error in setSysTick()");
	}
}
void enableSysTick() {
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; // enable
}
void disableSysTick() {
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; // disable
}
void SysTick_Handler() {
	++ticks;
}

// MARK: delay
static void delay(__IO uint32_t milliseconds) { // millisecond
	uint32_t microseconds = 0;
	if(!asyncTimerOn) ticks = 0; // make sure we don't run out of 32 bits at random times
	else microseconds += ticks;
	enableSysTick(); // start as soon as possible
	microseconds += milliseconds * 1000; //clarity
	while (ticks < microseconds);
	disableSysTick();
}
static void delay_micro(__IO uint32_t microseconds) {
	if(!asyncTimerOn) ticks = 0; // make sure we don't run out of 32 bits at random times
	else microseconds += ticks;
	enableSysTick(); // start as soon as possible
	while (ticks < microseconds);
	disableSysTick();
}

// MARK: async stopwatch
void timer::startAsyncStopwatch() {
	if (asyncTimerOn) offset = ticks; // if timer is already on then abstractly another timer is working. Don't reset that timer
	asyncTimerOn = true;
	ticks = 0;
	enableSysTick();
}
static void stopAsyncTimer(){ disableSysTick();}

uint32_t timer::elapsedTime(timeinterval interval){
	int ret = floor(ticks / interval);
	//	sprintf(msg, "%u ticks %i interval %i ticks/interval %u offset\n", ticks, interval, ret, offset);
	//	usart_puts(USART2, msg);
	return ret - offset;
}
