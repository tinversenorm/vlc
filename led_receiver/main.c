#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "adc.h"
#include "ST7735.h"

//#define DEBUG 1
#define SAMPLING_RATE 100000  //100 KHz sampling rate for ADC

const Port_name analog_port = PORT_E;
const int analog_pin = 4;

void lcd_init(void);
void timer_init(void);
void PortF_Init(void);

int main(void) {
	PLL_Init(Bus80MHz);
	DisableInterrupts();
	timer_init();
#ifdef DEBUG
	lcd_init();
#endif
	PortF_Init();
	ADC0_Init(analog_port, analog_pin, SAMPLING_RATE);
	EnableInterrupts();
	while(1) {
	}
}

void lcd_init(void) {
  ST7735_InitR(INITR_REDTAB);
  ST7735_FillScreen(ST7735_BLACK);
}

// time dump
uint16_t time_dump[1000];

void PortF_Init(void){ 
  SYSCTL_RCGCGPIO_R |= 0x20;    // activate port F
  while((SYSCTL_PRGPIO_R&0x20)==0){};

  GPIO_PORTF_DIR_R |= 0x04;     // make PF2 out (built-in blue LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;  // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;     // enable digital I/O on PF2
                                // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R & 0xFFFFF0FF) + 0x00000000;
  GPIO_PORTF_AMSEL_R = 0;  // disable analog functionality on PF
}