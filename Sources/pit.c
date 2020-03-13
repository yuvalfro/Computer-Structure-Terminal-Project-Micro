
#include "pit.h"

/* Initializes the PIT module to produce an interrupt every second
 * 
 * */
void InitPIT(void)
{
	// Enable PIT clock
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
	
	// Enable Green and Red LEDs clock and MUX
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	PORTB_PCR18 = PORT_PCR_MUX(1);
	PORTB_PCR19 = PORT_PCR_MUX(1);
	GPIOB_PDDR |= (1 << LED_GREEN)+(1 << LED_RED);
	GPIOB_PSOR |= (1 << LED_GREEN);
	GPIOB_PCOR |= (1 << LED_RED);
	
	// Turn on PIT
	PIT_MCR = 0;
	
	// Configure PIT to produce an interrupt every 1s
	PIT_LDVAL0 = 0x1312CFF;	// 1/20Mhz = 50ns   (1s/50ns)-1= 19,999,999 cycles or 0x1312CFF
	PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK; // Enable interrupt and enable timer
	
	//Enable interrupt registers ISER and ICPR
	//enable_irq(INT_PIT - 16);
}

/*	Handles PIT interrupt if enabled
 * 
 * 	Starts conversion in ADC0 with single ended channel 8 (PTB0) as input
 * 
 * */
