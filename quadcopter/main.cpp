
#include "main.hpp"

#define readPin(idr,pin) (idr & pin)
#define enableFloatingPoint() (*((int*)0xE000ED88))|=0x0F00000;  // Floating Point donanimini aktiflestir.

using namespace std;


// MARK: button click with interrupt
int btnOffset = 0;
int btnElapsed = 0;
void EXTI0_IRQHandler() {
//	timer tmr = timer();
//	btnElapsed = elapsedTime(0, milliseconds);
	
	if (EXTI_GetITStatus(EXTI_Line0) && btnElapsed - btnOffset > 300) {
		btnOffset = btnElapsed;
		//		usart_puts(USART2, "you pressed the button\n");
		GPIO_ToggleBits(GPIOD, pin14);
		//		delay(200);
		
	}
	EXTI_ClearITPendingBit(EXTI_Line0); // Clear the flag
}
void setup_button() {
	
	// connect exti0 to gpioA. Since it is exti0 the interrupt will use pin0
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0 /*exti0*/ );
	
	//Configure the exti0 line
	EXTI_InitTypeDef extiStructure;
	
	extiStructure.EXTI_Line = EXTI_Line0;
	extiStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	// set up when interrupt will trigger. Since we set up the button to pull down (gpioA pin0. pupd_down it is),
	// when button is not pressed it will be logic LOW. If we set trigger to rising then it will trigger when the
	// button is pressed, if we set it to falling then it will trigger after releasing the button
	extiStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	extiStructure.EXTI_LineCmd = ENABLE;
	
	EXTI_Init(&extiStructure);
	
	// configure the nvic
	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = EXTI0_IRQn;	// interrupt for exti0
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 1;
	nvicStructure.NVIC_IRQChannelSubPriority = 1;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
	
//	startAsyncStopwatch();
	
	
}

const char *byte_to_binary(int x)
{
	static char b[9];
	b[0] = '\0';
	
	int z;
	for (z = 128; z > 0; z >>= 1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}
	
	return b;
}

timer timer1 = timer();
usart usart1 = usart();
pwm pwm1 = pwm();
void setup() {
	enableFloatingPoint();
	setSysTick();
	setup_button();
	int pwmpins[] = {2,3};
	// enable GPIOx clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
//	pwm pwm1 = pwm();
	
	gpio(GPIOD, (pin12 | pin13 | pin14 | pin15) , OUTPUT, NOPULL);
	gpio(GPIOA, pin0, INPUT, GPIO_PuPd_DOWN);
	
	GPIO_SetBits(GPIOD, pin12); // indicate stm's working fine
	//	GPIO_SetBits(GPIOD, pin14);
	
	//	usart_puts(USART2, "hello world!\n");
//	startAsyncTimer();
	
	timer1.start();
	usart1 = usart(USART2, GPIOA, 2,3, 230400);
	usart1.begin();
	pwm1 = pwm(GPIOB, 6);
	pwm pwm2 = pwm(GPIOB, 7);
	TIM4->CCR1 = 0;
	TIM4->CCR2 = 0;
//	pwm1.write();
}

uint16_t prescaler = 8400;
double frequency = 50;

bool buttonReleased = true;
uint32_t offset = 0;
uint32_t elapsed = 0;

char *order; bool islo = true, run = false;
int waittime = 1500, pwmOffset = 0;
int lo = 0, hi = 0; bool newSignal = false;
void loop() {
	if (timer1.elapsedTime(milliseconds) > waittime) {
		timer1.start();
		if(usart1.available()){
			order = usart1.read();
			usart1.printf("read: %s\n", order);
			run = true;
			if(lo == 0) pwmOffset = 0;
			else
				newSignal = true;
		}
		if(!run) return;
		
		// for servo motor, edge pwm duty cycles are: 620 and 2400 / 20000 at 50 hz
		
		int i = 0;
		bool step = false;
//		GPIO_SetBits(GPIOD, pin15);
		char lotext[20], hitext[20];
		int loRef = 0, hiRef = 0;
		while(order[i] != '\n' && i < 50) {
			if(order[i] == ' ') {step = true; ++i;}
			if(!step) {
				lotext[loRef++] = order[i];
			} else {
				hitext[hiRef++] = order[i];
			}
			++i;
		}
		if(order[i-1] == '.') newSignal = false;
		if(newSignal){
			pwmOffset = lo - atoi(lotext) * 20;
			newSignal = false;
		}
		
		lo = atoi(lotext) * 20;
		if(step) {
			hi = atoi(hitext) * 20;
			waittime = 1500;
		}
		else {
			hi = atoi(lotext) * 20;
			waittime = 50;
		}
//		usart1.printf("lo: %i: hi: %i islo: %d\n",lo, hi, islo);
		usart1.printf("%ssend between 40 and 90\npwm at: %i / 20000. ----> %i / 1000  pwmOffset: %i\n", (islo ? "_" : "+"), lo + pwmOffset, (lo + pwmOffset) / 20, pwmOffset);
//		usart1.printf("pwm at: %i / 20000. ----> %i / 1000\n", (islo ? lo : hi), (islo ? lo : hi) / 20);
		GPIO_ToggleBits(GPIOD, pin13);
		
		
		TIM4->CCR1 = (islo ? lo : hi) + pwmOffset;
		TIM4->CCR2 = (islo ? lo : hi) + pwmOffset;
//		TIM4->CCR2 = (((islo ? lo : hi) * 100) % (hi - lo) + lo);
		islo = !islo;
//		usart1.printf("hello%i", 5);
		
		if(pwmOffset > 0) pwmOffset -= 10;
		else if(pwmOffset < 0) pwmOffset += 10;
		if(pwmOffset > -10 && pwmOffset < 10) pwmOffset = 0;
		
	}
//	int elapsed = elapsedTime(offset, microseconds);
//	offset = elapsed;
//	if( elapsed > 1000000){
//		GPIO_ToggleBits(GPIOD, pin15);
//	}
	
	//	elapsed = elapsedTime(0, seconds);
	////	msg[0] = '\0';
	//
	//	if(elapsed - offset > 0){
	//		sprintf(msg, "%i passed\n", elapsed);
	//		usart_puts(USART2, msg);
	//		offset = elapsed;
	//	}
	//	usart_puts(USART2, msg);
	
	//	uint32_t period = getPeriod(0, frequency, 84 * 1000000, prescaler);
	
	
	//	for(int i=900; i<2000; ++i) {
	//		TIM4->CCR1 = i;
	//		TIM4->CCR2 = i;
	//		delay_micro(4000);
	//		char qwe[20];
	//		sprintf(qwe,"%i\n", i);
	//		usart_puts(USART2,qwe);
	//
	//	}
	//	TIM4->CCR1 = 19999;
	//	TIM4->CCR2 = 19999;
	//	delay(30*1000);
	//	char* in;
	//
	//	if (newDataIn) {
	////		TIM4->CCR1 = atoi(readUsart());
	//		TIM4->CCR2 = atoi(readUsart());
	//	}
//	usart usart2(USART2, GPIOA, 6, 7);
//	usart2.printf("asdf");
	
}














