#include <stdint.h>
#include "adc.h"
#include "../inc/tm4c123gh6pm.h"

// move to timer.c
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_RELOAD_CNT      0x00000800  // 100KHz

uint16_t num_to_bits(int pin_num);
uint8_t set_ADC_sampling_rate(uint32_t max_sampling_rate);
uint16_t get_sequencer_priorities(ADC_Sequencer seq);
uint8_t get_ssmux3_channel(Port_name portname, int pin);

/*
 * Initialize timer to sampling rate
 */
void timer_init(void) {
  NVIC_ST_CTRL_R = 0;                    // disable SysTick during setup
  NVIC_ST_RELOAD_R = NVIC_ST_RELOAD_CNT; // 100 KHz
  NVIC_ST_CURRENT_R = 0;                 // any write to current clears it
                                         // enable SysTick with core clock
  NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC;
}


/*
 * Convert pin or port number to bit number
 * Value used for ADC0_Init function
 */
uint16_t num_to_bits(int pin_num) {
	return 1<<pin_num;
}

/* 
 * Initialize ADC and some port e pin
 */
// TODO: add more configuration options and clean up code
void ADC0_Init(Port_name portname, int pin, 
	uint32_t max_sampling_rate) {
	// TODO: add ADC_Sequencer sequencer option

	uint16_t port = num_to_bits(portname);

	SYSCTL_RCGCADC_R |= 0x0001;         // activate ADC0
	SYSCTL_RCGCGPIO_R |= port;          // activate clock for Port E
	while((SYSCTL_PRGPIO_R&port) != port){};

	uint16_t port_bit = num_to_bits(pin);
	// TODO: move this code to port initialization function
	GPIO_PORTE_DIR_R &= ~port_bit;      // 2) make port num input
	GPIO_PORTE_AFSEL_R |= port_bit;     // 3) enable alternate function
	GPIO_PORTE_DEN_R &= ~port_bit;      // 4) disable digital I/O
	GPIO_PORTE_AMSEL_R |= port_bit;     // 5) enable analog functionality

	ADC0_PC_R &= ~0xF;              // clear max sample rate field
	ADC0_PC_R |= set_ADC_sampling_rate(max_sampling_rate);
	//ADC0_SSPRI_R = get_sequencer_priorities(sequencer);
	ADC0_SSPRI_R = 0x0123;          // set sequencer priorities
	ADC0_ACTSS_R &= ~0x0008;        // 9) disable sample sequencer 3
	ADC0_EMUX_R &= ~0xF000;         // 10) seq3 is software trigger
	ADC0_SSMUX3_R &= ~0x000F;       // 11) clear SS3 field
	ADC0_SSMUX3_R += get_ssmux3_channel(portname, pin);
	//ADC0_SSMUX3_R += 9;             //    set channel
	ADC0_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
	ADC0_IM_R &= ~0x0008;           // 13) disable SS3 interrupts
	ADC0_ACTSS_R |= 0x0008;         // 14) enable sample sequencer 3

	ADC0_SAC_R |= 0x0004;           // hardware averaging in ADC
                                  // 02 - 4x, 04 - 16x, 06 - 64x
}

/*
 * Pass in the upper bound on the sampling rate
 * This function returns what the ADCPC reg should be set to.
 * Note: the max sampling rate of the TI microcontroller is 1MHz
 */
uint8_t set_ADC_sampling_rate(uint32_t max_sampling_rate) {
	if(max_sampling_rate <= 125000) {
		return 0x01;
	} else if(max_sampling_rate <= 250000) {
		return 0x03;
	} else if(max_sampling_rate <= 500000) {
		return 0x05;
	} else {
		return 0x07;
	}
}

/*
 * Pass in the top priority sequencer
 */
uint16_t get_sequencer_priorities(ADC_Sequencer seq) {
	switch(seq) {
		case seq_zero: return 0x3210;
		case seq_one: return 0x2103;
		case seq_two: return 0x1032;
		case seq_three: return 0x0123;
		default: return 0;
	}
}

/*
 * Set the sequencer channel in the SSMUX
 */
uint8_t get_ssmux3_channel(Port_name portname, int pin) {
	switch(portname) {
		case PORT_E: switch(pin) {
			case 4: return 9;
			case 5: return 8;
			case 3: return 0;
			case 2: return 1;
			case 1: return 2;
			case 0: return 3;
			default: return 0xFF;
		}
		default: //TODO: implement other Analog inputs
			return 0xFF;
	}
}