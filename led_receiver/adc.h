#ifndef ADC_H
#define ADC_H

#define PORTE_INIT 0x10

typedef enum adc_sequencer {
	seq_zero,
	seq_one,
	seq_two,
	seq_three
} ADC_Sequencer;

/* TODO: Move to TM4CWare */
typedef enum port_name {
	PORT_A,
	PORT_B,
	PORT_C,
	PORT_D,
	PORT_E,
	PORT_F
} Port_name;

/* Main init functions */
void ADC0_Init(enum port_name portname, int pin, 
	uint32_t max_sampling_rate);
void timer_init(void);

#endif