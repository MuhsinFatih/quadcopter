
#include "main.hpp"
#define read(idr,pin) (idr & pin)
#define enableFloatingPoint() (*((int*)0xE000ED88))|=0x0F00000;  // Floating Point donanimini aktiflestir.
#define setup() main()

void loop();

static char msg[255];

// MARK: timer
// microsecond resolution
void setSysTick() {
	if (SysTick_Config(SystemCoreClock / 1000000)) {
		//usart_puts(USART2, "error in setSysTick()");
	}
}
void enableSysTick() {
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk; // enable
}
void disableSysTick() {
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; // disable
}
volatile uint32_t ticks;
static bool asyncTimerOn = false;
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


// timer is at microseconds resolution. enum values can be used to multiply with ticks to get human readable results
typedef enum timeinterval{
	microseconds = 1, milliseconds = 1000, seconds = 1000000
} timeinterval;

// MARK: async stopwatch
// aslında daha iyi bir fikrim var: startAsyncTimer a her sayaç için özel elapsedTime değişkeni yapalım, startAsyncTimer fonksiyonuda bu değişkenlerin
// hepsinin pointer ını depolasın ve yeni timer gelince bütün timerların elapsedTime'ları geçen süre kadar artırılsın. Tabi bu iş için C++ la compile
// etmeyi halledersem çok güzel olur :)
static uint32_t startAsyncStopwatch() {
	if (asyncTimerOn) return ticks; // if timer is already on then abstractly another timer is working. Don't reset that timer
	asyncTimerOn = true;
	ticks = 0;
	enableSysTick();
	return 0; // 0 elapsed
}
#define stopAsyncTimer disableSysTick();
static uint32_t elapsedTime(uint32_t offset, timeinterval interval){
	int ret = floor(ticks / interval);
	//	sprintf(msg, "%u ticks %i interval %i ticks/interval %u offset\n", ticks, interval, ret, offset);
	//	usart_puts(USART2, msg);
	return ret - offset;
}


void gpio(GPIO_TypeDef* GPIOx, uint32_t pin, GPIOMode_TypeDef mode, GPIOPuPd_TypeDef PuPd) {
	GPIO_InitTypeDef initStructure;
	
	initStructure.GPIO_Pin = pin;
	initStructure.GPIO_Mode = mode;
	initStructure.GPIO_OType = GPIO_OType_PP;
	initStructure.GPIO_Speed = GPIO_Speed_100MHz;
	initStructure.GPIO_PuPd = PuPd;
	GPIO_Init(GPIOx, &initStructure);
}



// MARK: button click with interrupt
int btnOffset = 0;
int btnElapsed = 0;
void EXTI0_IRQHandler() {
	btnElapsed = elapsedTime(0, milliseconds);
	
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
	
	startAsyncStopwatch();
	
	
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
/**
 setup pwm
 
 @param pins numbers. not pinx notation. eg: {1,2,3,4}
 @param numOfPins size of the pins array
 */
void setupPWM(GPIO_TypeDef *GPIOx, int *pins, int numOfPins) {
	GPIO_InitTypeDef			gpioStructure;
	TIM_TimeBaseInitTypeDef		timeBaseStructure;
	TIM_OCInitTypeDef			outputControlStrucure;
	
	// enable timer 4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	char asdf[50];
	uint32_t gpioPins = 0;
	REP(numOfPins){
		gpioPins |= (uint32_t)pow(2,pins[i]);
		sprintf(asdf,"gpioPins= %s\n", byte_to_binary(gpioPins));
		//		usart_puts(USART2,asdf);
	}
	
	gpioStructure.GPIO_Pin = gpioPins;
	gpioStructure.GPIO_Mode = GPIO_Mode_AF;
	gpioStructure.GPIO_Speed = GPIO_Speed_100MHz;
	gpioStructure.GPIO_OType = GPIO_OType_PP;
	gpioStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOx, &gpioStructure);
	
	
	// assign alternate function
	GPIO_PinAFConfig(GPIOx, GPIO_PinSource6, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOx, GPIO_PinSource7, GPIO_AF_TIM4);
	
	
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

// setup
int setup() {
	
	enableFloatingPoint();
	setSysTick();
	setup_button();
	int pwmpins[] = {6,7};
	// enable GPIOx clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	//	setup_USART(2,3);
	setupPWM(GPIOB, pwmpins, 2);
	
	gpio(GPIOD, (pin12 | pin13 | pin14 | pin15) , OUTPUT, NOPULL);
	gpio(GPIOA, pin0, INPUT, GPIO_PuPd_DOWN);
	
	GPIO_SetBits(GPIOD, pin12); // indicate stm's working fine
	//	GPIO_SetBits(GPIOD, pin14);
	
	//	usart_puts(USART2, "hello world!\n");
	startAsyncStopwatch();
	
	while(true) loop();
	return 0;
}

uint16_t prescaler = 8400;
double frequency = 50;

bool buttonReleased = true;
uint32_t offset = 0;
uint32_t elapsed = 0;
void loop() {
	int elapsed = elapsedTime(offset, microseconds);
	offset = elapsed;
	if( elapsed > 1000000){
		GPIO_ToggleBits(GPIOD, pin15);
	}
	
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
	usart usrt(USART2, 6, 7);
//	usrt.printf(<#const char *, ...#>)
	
}














