
#include "timer.hpp"
using namespace std;
// MARK: - global variables 
bool asyncTimerOn = false;
volatile uint32_t ticks;

//vector<timer> openTimers;

// microsecond resolution
void setSysTick() {
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
extern "C" {
	void SysTick_Handler() {
		++ticks;
	}
}

// MARK: delay
void delay(__IO uint32_t milliseconds) { // millisecond
	uint32_t microseconds = ticks;
	enableSysTick(); // start as soon as possible
	microseconds += milliseconds * 1000; //clarity
	while (ticks < microseconds);
	disableSysTick();
}
void delay_micro(__IO uint32_t microseconds) {
	microseconds += ticks;
	enableSysTick(); // start as soon as possible
	while (ticks < microseconds);
	disableSysTick();
}

timer::timer(){
	if(asyncTimerOn) offset = ticks;
}

// MARK: async stopwatch
void timer::start() {
	if (asyncTimerOn) this->offset = ticks; // if timer is already on then abstractly another timer is working. Don't reset that timer
	asyncTimerOn = true;
	ticks = 0;
	enableSysTick();
}

void timer::stop() {
}
void stopAsyncTimer(){ disableSysTick();}

uint32_t timer::elapsedTime(timeinterval interval){
	int ret = floor(ticks / interval);
	//	sprintf(msg, "%u ticks %i interval %i ticks/interval %u offset\n", ticks, interval, ret, offset);
	//	usart_puts(USART2, msg);
	return ret - offset;
}
