
#include "usart.hpp"

using namespace std;


// MARK: usart setup
//FIXME: fix rx, tx params. currently stm picks one magically for rx and tx
void usart::setup_USART() {
	GPIO_InitTypeDef gpioStructure;
	USART_InitTypeDef usartStructure;
	NVIC_InitTypeDef nvicStructure;
	
	int pins[2] = {(int)pow(2,rx),(int)pow(2,tx)};
	// Enable the periph clock for usart1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	// Enable the GPIOA clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	// Setup the gpio pins for Tx and Rx
	gpioStructure.GPIO_Pin = pins[0] | pins[1];
	gpioStructure.GPIO_Mode = GPIO_Mode_AF;
	gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
	gpioStructure.GPIO_OType = GPIO_OType_PP;
	gpioStructure.GPIO_PuPd = GPIO_PuPd_UP;
	
	GPIO_Init(GPIOA, &gpioStructure);
	
	GPIO_PinAFConfig(GPIOA, this->rx, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, this->tx, GPIO_AF_USART2);
	
	
	usartStructure.USART_BaudRate = this->baudrate;
	usartStructure.USART_WordLength = USART_WordLength_8b;
	usartStructure.USART_StopBits = USART_StopBits_1;
	usartStructure.USART_Parity = USART_Parity_No;
	usartStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usartStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2,&usartStructure);
	
	// enable interrupt for receive event on usart
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	//IRQ: interrupt request
	nvicStructure.NVIC_IRQChannel = USART2_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0; // 0: highest priority. (lowest=15)
	nvicStructure.NVIC_IRQChannelSubPriority = 0;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
	
	USART_Cmd(USART2, ENABLE);
	
	
}

usart::usart(){
	
}
usart::usart(USART_TypeDef *USARTx, GPIO_TypeDef *GPIOx, int rx, int tx, int baudrate) {
	this->USARTx = USARTx; // not sure if this is safe
	this->GPIOx = GPIOx; // not sure if this is safe
	this->rx = rx;
	this->tx = tx;
	this->baudrate = baudrate;
	setup_USART();
}

void usart::printf(const char *format, ...) {
	const int size = 512;
	// following lines are c magic
	char buffer[size];
	va_list args;
	va_start (args, format);
	vsnprintf (buffer,size,format, args);
	perror (buffer);
	va_end (args);
	usart_puts(this->USARTx, buffer);
}

//volatile char* read(){
//	return null;
//}


void usart::usart_puts(USART_TypeDef *USARTx, const char *str){
	while(*str){
		// Wait for the TC (Transmission Complete) Flag to be set
		// while(!(USARTx->SR & 0x040));
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
		USART_SendData(USARTx, *str);
		*str++;
	}
}

//MARK: read usart input
#define MAX_WORDLEN		255
volatile char buffer[MAX_WORDLEN + 1];
char receivedString[MAX_WORDLEN + 1];
volatile bool newDataIn = false;
extern void usart_puts(USART_TypeDef *USARTx, char *str);
// Interrupt request handler for all usart2 interrupts
// This interrupt handler will be executed each time a char is received in usart2
extern "C" {
void USART2_IRQHandler(){
	// make sure it was usart2 and we didnt screw up things
	if(USART_GetITStatus(USART2, USART_IT_RXNE)){
		static int count = 0;
		char ch = USART2->DR;
		if((ch != '\n') && (count < MAX_WORDLEN)){
			buffer[count++] = ch;
		} else {
			buffer[count] = '\n';
			count = 0;
			REP(MAX_WORDLEN){
				receivedString[i] = buffer[i];
			}
			newDataIn = true;
//			usart_puts(USART2, receivedStr);
		}
	}\
	
}
}
void * __dso_handle; // workaround
//string receivedStr;

char readString[MAX_WORDLEN + 1];

// DONT FORGET TO DEALLOCATE AFTER READING!!
char* usart::read() {
	newDataIn = false;
	int size = 0;
	REP(MAX_WORDLEN){
		++size;
		if(buffer[i] == '\n') break;
	}
	
	char* ret = (char*)malloc(size * sizeof(char)); // allocate just enough space
	if(!ret) return NULL;
	
	REP(size) ret[i] = buffer[i]; // copy received volatile string to 'ret'
	return ret;
}












