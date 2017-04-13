
#include "main.hpp"
#include "i2c.hpp"
#include <stm32f4xx_i2c.h>

#define readPin(idr,pin) (idr & pin)
#define enableFloatingPoint() (*((int*)0xE000ED88))|=0x0F00000;  // Floating Point donanimini aktiflestir.

using namespace std;

void I2C_write(I2C_TypeDef* I2Cx, uint8_t data);
void i2ctest();
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

usart usart1 = usart();

timer timer1 = timer();

//pwm pwm1 = pwm();
//pwm pwm2 = pwm();
//pwm pwm3 = pwm();
//pwm pwm4 = pwm();

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
//	pwm1 = pwm(GPIOB, 6, &TIM4->CCR1);
//	pwm2 = pwm(GPIOB, 7, &TIM4->CCR2);
//	pwm3 = pwm(GPIOB, 8, &TIM4->CCR3);
//	pwm4 = pwm(GPIOB, 9, &TIM4->CCR4);
	i2ctest();
	
//	pwm1.write(0);
//	pwm2.write(0);
//	pwm3.write(0);
//	pwm4.write(0);
	
//	i2ctest();
	
}

double frequency = 50;

bool buttonReleased = true;
uint32_t offset = 0;
uint32_t elapsed = 0;

char *order; bool islo = true, run = false;
int waittime = 50, pwmOffset = 0;
int lo = 0, hi = 0; bool newSignal = false;
void simulateFlight();
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
		
//		simulateFlight();
		
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
		if(order[i-1] != '.') {
			pwmOffset = 0;
			newSignal = false;
		}
		if(newSignal){
			pwmOffset = lo - atoi(lotext) * 20;
			newSignal = false;
		}
		
		lo = atoi(lotext) * 20;
		usart1.printf("%ssend between 40 and 90\npwm at: %i / 20000. ----> %i / 1000  pwmOffset: %i\n", (islo ? "_" : "+"), lo + pwmOffset, (lo + pwmOffset) / 20, pwmOffset);
		GPIO_ToggleBits(GPIOD, pin13);
		
//		*pwm1.CCR = lo;
//		*pwm2.CCR = lo;
//		*pwm3.CCR = lo;
//		*pwm4.CCR = lo;
		
		islo = !islo;
		
		if(pwmOffset > 0) pwmOffset -= 100;
		else if(pwmOffset < 0) pwmOffset += 100;
		if(pwmOffset > -100 && pwmOffset < 100) pwmOffset = 0;
		
	}
}

class quadcopter {
public:
	double weight;
	
	struct body { // everything relative to center
		double centerOfMass;
		double accelPos;
		double gyroPos;
		double motorsPos;
	} body;
	
	struct gyro{ double X, Y, Z;} gyro;
	struct accel{ double X, Y, Z;} accel;
	
};

void simulateFlight(){
	// order is global
	quadcopter quadro = quadcopter();
	quadro.body.accelPos = 5;
}

void i2ctest() {
	GPIO_InitTypeDef gpioStruct;
	I2C_InitTypeDef i2cStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	usart1.printf("debug 1\n");
	
	
	gpioStruct.GPIO_Pin = pin6 | pin7;
	gpioStruct.GPIO_Mode = GPIO_Mode_AF;
	gpioStruct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioStruct.GPIO_OType = GPIO_OType_OD;			// set output to open drain --> the line has to be only pulled low, not driven high
	gpioStruct.GPIO_PuPd = GPIO_PuPd_UP;			// enable pull up resistors
	GPIO_Init(GPIOB, &gpioStruct);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1); // SCL
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); // SDA
	//	GPIO_PinAFConfig(GPIOB, EXTI_PinSource10, GPIO_AF_I2C1);
	//	GPIO_PinAFConfig(GPIOB, EXTI_PinSource11, GPIO_AF_I2C1);
	
	i2cStruct.I2C_ClockSpeed = 100000;
	i2cStruct.I2C_Mode = I2C_Mode_I2C;
	i2cStruct.I2C_DutyCycle = I2C_DutyCycle_2;	// 50% duty cycle --> standard
	i2cStruct.I2C_OwnAddress1 = 0x00;			// own address, not relevant in master mode
	i2cStruct.I2C_Ack = I2C_Ack_Enable;
	i2cStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // set address length to 7 bit addresses
	I2C_Init(I2C1, &i2cStruct);
	
	I2C_Cmd(I2C1, ENABLE);
	
	usart1.printf("debug 2\n");
	int ms = timer1.elapsedTime(microseconds);
	
	// wait while I2C1 is busy
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	usart1.printf("waited for I2C1 busy for %i\n", timer1.elapsedTime(microseconds) - ms);
	
	// Send I2C1 START condition
	I2C_GenerateSTART(I2C1, ENABLE);
	usart1.printf("debug 3\n");
	ms = timer1.elapsedTime(milliseconds);

	
//	I2C_write(I2C1, 0x75);
	usart1.printf("debug 4\n");
	
	// wait for I2C1 EV5 --> Slave has acknowledged start condition
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	usart1.printf("waited for I2C1 check for %i\n", timer1.elapsedTime(milliseconds) - ms);
	
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	usart1.printf("debug 5\n");
	ms = timer1.elapsedTime(milliseconds);
	
	// wait until one byte has been received
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
	usart1.printf("waited for I2C1 check for %i\n", timer1.elapsedTime(milliseconds) - ms);
	
	I2C_write(I2C1, 0x75);
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2C1);
	usart1.printf("data = %i\n", data);
	
	
	while(true){
		//		usart1.printf("id: %d\n", MPU6050_GetDeviceID());
		delay(500);
	}
}
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data)
{
	I2C_SendData(I2Cx, data);
	
	// wait for I2C1 EV8_2 --> byte has been transmitted
//	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}









