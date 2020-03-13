#include "TFC.h"
#include "mcg.h"


void LCDconf(){

	//enable Clocks to all ports - page 206, enable clock to Ports
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
	SIM_SCGC6 |= SIM_SCGC6_DAC0_MASK;
	
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK; //Enable the Clock to the PIT Modules
	// Timer 0
	PIT_TCTRL0 = PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK; //enable PIT0 and its interrupt
	PIT_MCR |= PIT_MCR_FRZ_MASK; // stop the pit when in debug mode
	//enable_irq(INT_PIT-16); //  //Enable PIT IRQ on the NVIC
	//set_irq_priority(INT_PIT-16,0);  // Interrupt priority = 0 = max
	
	DAC0_C0 |= DAC_C0_DACEN_MASK + DAC_C0_DACRFS_MASK + DAC_C0_DACTRGSEL_MASK + DAC_C0_LPEN_MASK; 
	DAC0_DAT0L =0xff;
	DAC0_DAT0H =0x05;
	
  	PIT_MCR &= ~PIT_MCR_MDIS_MASK;
  	
	PORTE_PCR1 = PORT_PCR_MUX(1);
	PORTA_PCR16 = PORT_PCR_MUX(1);
	PORTD_PCR3 = PORT_PCR_MUX(1);
	PORTC_PCR17 = PORT_PCR_MUX(1);
	PORTD_PCR2 = PORT_PCR_MUX(1);
	PORTC_PCR16 = PORT_PCR_MUX(1);
	PORTD_PCR0 = PORT_PCR_MUX(1);
	PORTC_PCR13 = PORT_PCR_MUX(1);
	PORTD_PCR5 = PORT_PCR_MUX(1);
	PORTC_PCR12 = PORT_PCR_MUX(1);
	PORTA_PCR13 = PORT_PCR_MUX(1);
		
	GPIOE_PDDR|= PORT_LOC(1);
	GPIOA_PDDR|= PORT_LOC(16);
	GPIOD_PDDR|= PORT_LOC(3);
	GPIOC_PDDR|= PORT_LOC(17);
	GPIOD_PDDR|= PORT_LOC(2);
	GPIOC_PDDR|= PORT_LOC(16);
	GPIOD_PDDR|= PORT_LOC(0);
	GPIOC_PDDR|= PORT_LOC(13);
	GPIOD_PDDR|= PORT_LOC(5);
	GPIOC_PDDR|= PORT_LOC(12);
	GPIOA_PDDR|= PORT_LOC(13);
	GPIOE_PDDR|= PORT_LOC(30);
	
	//-----------------------
	
	GPIOE_PCOR = PORT_LOC(1);
	GPIOA_PCOR = PORT_LOC(16);
	GPIOD_PCOR = PORT_LOC(3);
	
	delay1mn(15);

	dataByte(0x3f);
	LCDstrobeRoutine ();
	delay1mn(5);
	dataByte(0x3f);
	LCDstrobeRoutine ();
	delay1mn(1);
	dataByte(0x3f);
	LCDstrobeRoutine ();
	LCDcommand (0x3c);	
	LCDcommand (0x0f);	
	LCDcommand (0x01);	
	LCDcommand (0x06);	
	LCDcommand (0x80);	
	LCDcommand (0x02);	
}


void LCDstrobeRoutine (){
	GPIOD_PSOR = PORT_LOC(3);
	nop();
	nop();
	GPIOD_PCOR = PORT_LOC(3);
}

void delay1mn(int n){
	if (n!=1){
		PIT_LDVAL0 =  0x5DC0*n;} // setup timer 0 for 1msec counting period
	if(n==1){
		PIT_LDVAL0 =  0x5DC0/5;}
	PIT_TCTRL0 = PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK;
	//wait();
}

void LCDcommand (int command){
	delay1mn(5);
	dataByte(command);
	LCDstrobeRoutine ();
}

void dataByte(int data) {
	int DByte = data;
		if(DByte%2==0){
			GPIOC_PCOR = PORT_LOC(17);}
		else GPIOC_PSOR = PORT_LOC(17);
		DByte = DByte/2;
		
		if(DByte%2==0){
			GPIOD_PCOR = PORT_LOC(2);}
		else GPIOD_PSOR = PORT_LOC(2);
		DByte = DByte/2;
		
		if(DByte%2==0){
			GPIOC_PCOR = PORT_LOC(16);}
		else GPIOC_PSOR = PORT_LOC(16);
		DByte = DByte/2;
		
		if(DByte%2==0){
			GPIOD_PCOR = PORT_LOC(0);}
		else GPIOD_PSOR = PORT_LOC(0);
		DByte = DByte/2;
		
		if(DByte%2==0){
			GPIOC_PCOR = PORT_LOC(13);}
		else GPIOC_PSOR = PORT_LOC(13);
		DByte = DByte/2;
		
		if(DByte%2==0){
			GPIOD_PCOR = PORT_LOC(5);}
		else GPIOD_PSOR = PORT_LOC(5);
		DByte = DByte/2;
		
		if(DByte%2==0){
			GPIOC_PCOR = PORT_LOC(12);}
		else GPIOC_PSOR = PORT_LOC(12);
		DByte = DByte/2;
		
		if(DByte%2==0){
			GPIOA_PCOR = PORT_LOC(13);}
		else GPIOA_PSOR = PORT_LOC(13);
}

void nop(){
	
}

void lcd_Data(char c)
{
	delay1mn(5);
	dataByte(0x00);
	GPIOE_PSOR = PORT_LOC(1);
	dataByte(c);
	LCDstrobeRoutine ();
	GPIOE_PCOR = PORT_LOC(1);
}
