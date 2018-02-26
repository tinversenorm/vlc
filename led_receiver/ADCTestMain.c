// ADCTestMain.c
// Runs on TM4C123
// This program periodically samples ADC channel 0 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.
// Daniel Valvano
// September 5, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// center of X-ohm potentiometer connected to PE3/AIN0
// bottom of X-ohm potentiometer connected to ground
// top of X-ohm potentiometer connected to +3.3V 
#include <stdint.h>
#include "ADCSWTrigger.h"
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"

#define PF2             (*((volatile uint32_t *)0x40025010))
#define PF1             (*((volatile uint32_t *)0x40025008))
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

int getTimeJitter();
void plotADCPmf();

volatile uint32_t ADCvalue;
// This debug function initializes Timer0A to request interrupts
// at a 100 Hz frequency.  It is similar to FreqMeasure.c.
void Timer0A_Init100HzInt(void){
  volatile uint32_t delay;
  DisableInterrupts();
  // **** general initialization ****
  SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = 0;                // configure for 32-bit timer mode
  // **** timer0A initialization ****
                                   // configure for periodic mode
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER0_TAILR_R = 799999;         // start value for 100 Hz interrupts
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// enable timeout (rollover) interrupt
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// clear timer0A timeout flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 32-b, periodic, interrupts
  // **** interrupt initialization ****
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = 1<<19;              // enable interrupt 19 in NVIC
}

void Timer1_Init(void){
  volatile uint32_t delay;
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
  delay = SYSCTL_RCGCTIMER_R;   // allow time to finish activating
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, down-count 
  TIMER1_TAILR_R = 0xFFFFFFFF;  // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
}

void Timer2_Init(void){
  volatile uint32_t delay;
  SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate TIMER2
  delay = SYSCTL_RCGCTIMER_R;   // allow time to finish activating
  TIMER2_CTL_R = 0x00000000;    // 1) disable TIMER2A during setup
  TIMER2_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER2_TAMR_R = 0x00000002;   // 3) configure for periodic mode, down-count 
  TIMER2_TAILR_R = 7999999;     // 4) reload value for 10KHz interrupt
  TIMER2_TAPR_R = 0;            // 5) bus clock resolution
  TIMER2_CTL_R = 0x00000001;    // 10) enable TIMER2A
}

void Timer3_Init(void){
  volatile uint32_t delay;
  SYSCTL_RCGCTIMER_R |= 0x08;   // 0) activate TIMER3
  delay = SYSCTL_RCGCTIMER_R;   // allow time to finish activating
  TIMER3_CTL_R = 0x00000000;    // 1) disable TIMER3A during setup
  TIMER3_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER3_TAMR_R = 0x00000002;   // 3) configure for periodic mode, down-count 
  TIMER3_TAILR_R = 7999999;     // 4) reload value for 10KHz interrupt
  TIMER3_TAPR_R = 0;            // 5) bus clock resolution
  TIMER3_CTL_R = 0x00000001;    // 10) enable TIMER3A
}

int32_t Time_dump[1000];
int32_t ADC_dump[1000];
/* Skip first measurement because of initialization time */
int Debug_ctr = -1;
void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;    // acknowledge timer0A timeout
  if(Debug_ctr < 0) {
    Debug_ctr++;
    return;
  }
  PF2 ^= 0x04;                   // profile
  PF2 ^= 0x04;                   // profile
  ADCvalue = ADC0_InSeq3();
  PF2 ^= 0x04;                   // profile
  if(Debug_ctr < 1000) { // collect 1000 samples of ADC measurements
    Time_dump[Debug_ctr] = TIMER1_TAR_R;
    ADC_dump[Debug_ctr] = ADCvalue;
    Debug_ctr++;
  }
}
int main_1(void){
  PLL_Init(Bus80MHz);                   // 80 MHz
  SYSCTL_RCGCGPIO_R |= 0x20;            // activate port F
  ADC0_InitSWTriggerSeq3_Ch9();         // allow time to finish activating
  Timer0A_Init100HzInt();               // set up Timer0A for 100 Hz interrupts

  Timer1_Init();                        // initialize time 1
  ST7735_InitR(INITR_REDTAB);           // init LCD screen
  //Timer2_Init();                        // init timer 2 to change time jitter
  //Timer3_Init();                        // init timer 3

  GPIO_PORTF_DIR_R |= 0x06;             // make PF2, PF1 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x06;          // disable alt funct on PF2, PF1
  GPIO_PORTF_DEN_R |= 0x06;             // enable digital I/O on PF2, PF1
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
  PF2 = 0;                      // turn off LED
  EnableInterrupts();
  while(1){
    PF1 ^= 0x02;  // toggles when running in main
    int test = 2 / 3;
    if(Debug_ctr == 1000) {
      int time_jitter = getTimeJitter();
      ST7735_SetCursor(0, 1); 
      ST7735_OutString("Time Jitter: ");
      ST7735_OutUDec(time_jitter);
      plotADCPmf();
      Debug_ctr++;
    }
  }
}

/**************getTimeJitter***************
 Iterates through Time dump array and gets the difference 
 between the largest and smallest values.
 Inputs: None
 Outputs: the numerical value of the time jitter
 assumes that the Time_dump array is filled
*/
int getTimeJitter() {
  int largest_diff = Time_dump[0] - Time_dump[1];
  int smallest_diff = Time_dump[0] - Time_dump[1];
  for(int i = 0; i < 999; i++) {
    int current = Time_dump[i] - Time_dump[i+1];
    if(current > largest_diff) largest_diff = current;
    if(current < smallest_diff) smallest_diff = current;
  }
  return largest_diff - smallest_diff;
}

int Histogram[4096];
void plotADCPmf() {
  int min = 4096;
  int max = 0;
  for(int i = 0; i < 1000; i++) {
    if(ADC_dump[i] >= max) {
      max = ADC_dump[i];
    } else if(ADC_dump[i] <= min) {
      min = ADC_dump[i];
    }
  }

  ST7735_SetCursor(0, 0); ST7735_OutString("ADC PMF Plot");
  int Histogram_len = max - min + 1;
  for(int i = 0; i < Histogram_len; i++) {
    Histogram[i] = 0;
  }
  for(int i = 0; i < 1000; i++) {
    Histogram[ADC_dump[i]-min]++;
  }
  ST7735_PlotClear(0,4095);
  int averaging_lvl = Histogram_len/128 <= 1 ? 1 : Histogram_len/128;
  for(int i = 0; i < Histogram_len; i++) {
    int j; int cur_total = 0;
    for(j = 0; j < averaging_lvl && i < Histogram_len; j++, i++) {
      cur_total += Histogram[i];
    }
    i--;
    ST7735_PlotBar((cur_total/j)*50); /* scale by 50 so the plot is more visible */
    ST7735_PlotNext();
  }
  ST7735_SetCursor(0, 2); 
  ST7735_OutString("PMF Length: "); 
  ST7735_OutUDec(Histogram_len);
}
