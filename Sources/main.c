/*
 * main implementation: use this 'C' sample to create your own application
 *
 */


//#include "derivative.h" /* include peripheral declarations */
# include "TFC.h"
#include<stdio.h>
#include <stdlib.h>
#include <math.h>
#define MUDULO_REGISTER  0x2EE0
#define ADC_MAX_CODE     4095
#define CORE_CLOCK 48000000
#define MIN(a,b) (((a)<(b))?(a):(b))
volatile unsigned int state = 0; // 1 = Files transfer mode
volatile unsigned int flagPTD7 = 0x01;
volatile unsigned int topFile = 0;
volatile unsigned int topContent = 1;
char pcmessage[40]; 			 // Array to receive string from command mode
char filesname[6][14];			 // Files transfer mode: saving files name
int filessize [6];				 // Files transfer mode: saving files size
char filessizeC[6][4];	    	 // Files transfer mode: saving files size in chars for printing in LCD
int fs = 0;       			 	 // Files transfer mode: single file size
int numoffiles = 0;				 // Files transfer mode: how many files we have
int arridx = 0;   				 // UART0 interrupt: index to insert chars to array 
int dataready = 0;  		   	 // UART0 interrupt: Change to 1 if the data is ready
int firstin = 0;				 // Files transfer mode: oldest file 
char files[6][2000];			 // Files transfer mode: files content
int scroll = 0;					 // Files transfer mode: 0 - scroll files, 1 - scroll file content
void adc_init();
void set_color();
void set_files_menu();
void LCDconf();
void dma_init();
void receive_file(int index);

//unsigned int LightIntensity = MUDULO_REGISTER/2;  // Global variable


int main(void){
	
	ClockSetup();
	InitGPIO();
	InitPIT();
	LCDconf();
	dma_init();
	InitUARTs(9600);
	RGB_LED_OFF;
	LCDcommand (0x01);
	UARTprintf(UART0_BASE_PTR,"\n");
	while(1){
	
		if (dataready)
		{ 
			dataready = 0;
			
			//---------------------------------------------------------------------------------------------------------------------
			// Changing connection parameters
			// --------------------------------------------------------------------------------------------------------------------
			if (strncmp(pcmessage,"ConnectionParams", 16) == 0){
				int j;
				UARTprintf(UART0_BASE_PTR,"ack\n");
				for (j=10000; j>0; j--);
				UART0_C2 &= 0xF3;			   // stop bit only be changed when the transmitter(bit3) and receiver(bit2) are both disabled
				int baud = 9600;
				int i = 0;
			
				switch (pcmessage[17]){    //Check baud rate
					case '9': baud = 9600; break;
					case '2': baud = 2400; break;
					case '1': baud = 19200; i+=1; break;
					case '3': baud = 38400; i+=1; break;
				}

 				InitUARTs(baud);
			
				switch (pcmessage[22+i]){   //Check parity bit
					case 'N': UART0_C1 &= 0xFD; i+=1 ; break;                         // Parity = none  UART0_C1.PE = 0
					case 'E': UART0_C1 |= 0x02; UART0_C1 &= 0xFE; i+=1 ; break;       // Parity = even  UART0_C1.PE = 1 + UART0_C1.PT = 0
					case 'O': UART0_C1 |= 0x02; UART0_C1 |= 0x01; break;	          // Parity = odd   UART0_C1.PE = 1 + UART0_C1.PT = 1
				}
				
				switch (pcmessage[26+i]){	//Check stop bits									
				   case '1': UART0_BDH &= 0xDF; break;							      // Stop bit = 1	UART0_BDH.SBNS = 0
				   case '2': UART0_BDH |= 0x20; break;								  // Stop bit = 2   UART0_BDH.SBNS = 1
				}
				
				UART0_C2 = UARTLP_C2_RE_MASK | UARTLP_C2_TE_MASK | UART_C2_RIE_MASK; // Enable Transmitter, Receiver, Receive interrupt					                            // Call the function that set the baud rate
				memset(pcmessage,0,40);  //memset - clears the array
				//while (dataready == 0); //? omer said it help
			}
			
			//---------------------------------------------------------------------------------------------------------------------
			// Command mode
			//---------------------------------------------------------------------------------------------------------------------
			if ((pcmessage[0] == 's') & (pcmessage[1] == 'e') & (pcmessage[2] == 't') ){ //set color 
				set_color();
				memset(pcmessage,0,40);  //memset - clears the array
			}
			if (strcmp(pcmessage,"clear rgb\r") == 0){ // clear RGB
				RGB_LED_OFF;
				memset(pcmessage,0,40);  //memset - clears the array
			}
			//---------------------------------------------------------------------------------------------------------------------
			// Chat mode
			//---------------------------------------------------------------------------------------------------------------------
			if ((pcmessage[0] == 'm') & (pcmessage[1] == 's') & (pcmessage[2] == 'g') ){ //message received 
				LCDcommand (0x01);               
				int i = 4;
				int j;
				while (pcmessage[i] != '\r')
				{
					for (j=10000; j>0; j--);	 	  // Delay
					if (i==20){
						LCDcommand(0xc0);        	  //second line
						for (j=10000; j>0; j--);	  // Delay
					}
					lcd_Data(pcmessage[i]);           //print char
					i+=1;
				}
				memset(pcmessage,0,40);               //memset - clears the array
			}
			
			//---------------------------------------------------------------------------------------------------------------------
			// Files mode
			//---------------------------------------------------------------------------------------------------------------------
			if (strncmp(pcmessage,"filer", 5) == 0){
				state = 1;
				//FIFO Mechanism
				if(numoffiles==6){//when numoffiles reaches 6 it stops incrementing and firstin is the pointer
					 receive_file(firstin);
					 if(firstin == 5){
						firstin=0; 
					 }
					 else
						firstin +=1;
				}
				else{
					receive_file(numoffiles);
					numoffiles += 1;
				}     
			}
			
			//---------------------------------------------------------------------------------------------------------------------
			// Files mode - print on LCD
			//---------------------------------------------------------------------------------------------------------------------
			if (strncmp(pcmessage,"FilesLCD", 8) == 0){
				set_files_menu();
				memset(pcmessage,0,40);                  //memset - clears the array
			}
			
			//---------------------------------------------------------------------------------------------------------------------
			// Files mode - Exit
			//---------------------------------------------------------------------------------------------------------------------
			if (strncmp(pcmessage,"FilesExit", 9) == 0){
				state = 0;
				LCDcommand (0x01);
				
				memset(pcmessage,0,40);                  //memset - clears the array
			}
			
			//---------------------------------------------------------------------------------------------------------------------
			// Files mode - AckFile
			//---------------------------------------------------------------------------------------------------------------------
			if (strncmp(pcmessage,"ackfile",7) == 0){
				DMA_DSR_BCR1 = DMA_DSR_BCR_BCR(filessize[topFile]+1);       // number of bytes yet to be transferred
				DMA_SAR1 = (uint32_t)&files[topFile];
				UART0_C5 |= UART0_C5_TDMAE_MASK;        				  // Enable DMA request for UART0 transmitter  
				DMAMUX0_CHCFG1 |= DMAMUX_CHCFG_ENBL_MASK;				  // Enable DMA channel 
				disable_irq(INT_UART0-16);  			                  // Disable UART0 interrupt 
				
				memset(pcmessage,0,40);                                   //memset - clears the array
			}
			
		}
	} // End of while(1)
	return 0;
}

//-----------------------------------------------------------------
//  PORTD - ISR = Interrupt Service Routine
//-----------------------------------------------------------------
void PORTD_IRQHandler(void){
	volatile unsigned int i;
	// check that the interrupt was for switch
	if (PORTD_ISFR & SW7_POS) { //PTD7 - Choosing file
		//Debounce or using PFE field
		while(!(GPIOD_PDIR & SW7_POS) );	// wait of release the button
		for(i=10000 ; i>0 ; i--); 			//delay, button debounce
		
		PORTD_ISFR |= 0x00000080; 			 // clear interrupt flag bit of PTD7
		if ((state==1) & (numoffiles >= 1)) { 
			flagPTD7 ^= 0x01;
			scroll = 1;
			if (flagPTD7 == 0){ //first press - show file on LCD
				LCDcommand (0x01);
				int i, j;
				for (i=0; i<MIN(filessize[topFile],32);i++){
					for (j=10000; j>0; j--);	 	  // Delay
					if (i==16){
						LCDcommand(0xc0);        	  //second line
						for (j=10000; j>0; j--);	  // Delay
					}
					lcd_Data(files[topFile][i]);
				}
			}
		
			else { // second press - send file to PC
				files[topFile][filessize[topFile]] = '|';
				char str[24];
				memset(str,0,24);                  //memset - clears the array
				sprintf(str,"filename %s\n",filesname[topFile]);
				set_files_menu();
				scroll = 0;
				topContent = 1;
				UARTprintf(UART0_BASE_PTR,str);
				
			}
		}
	}
	
	if (PORTD_ISFR & 0x40){  //PTD6 - Move between files in LCD
		if ((state == 1) & (numoffiles >= 2)){
			if (scroll == 0){
				LCDcommand (0x01);
				topFile += 1;
				if (topFile == numoffiles)
					topFile = 0;
				if(topFile==numoffiles-1){
						//print last file
						int k = 0;
						int j;
						while (filesname[topFile][k] != '.')
						{
							for (j=10000; j>0; j--);	 	        // Delay
							lcd_Data(filesname[topFile][k]);        //print char
							k+=1;
						}
					
						k = 0;
						for (j=10000; j>0; j--);	 	      // Delay
						LCDcommand (0x14);					  // Move cursor right
		
						while (filessizeC[topFile][k] != 'F')
						{
							for (j=10000; j>0; j--);	 	   // Delay
							lcd_Data(filessizeC[topFile][k]);        //print char
							k+=1;
						}
						
						k=0;
						for (j=10000; j>0; j--);	 	      // Delay
						lcd_Data('B');
						for (j=10000; j>0; j--);	          // Delay
						if (i==0)
						{
							LCDcommand(0xc0);        	      //second line
							for (j=10000; j>0; j--);	     // Delay
						}
						//print first file
						while (filesname[0][k] != '.')
						{
							for (j=10000; j>0; j--);	 	  // Delay
							lcd_Data(filesname[0][k]);        //print char
							k+=1;
						}
					
						k = 0;
						for (j=10000; j>0; j--);	 	      // Delay
						LCDcommand (0x14);					  // Move cursor right
		
						while (filessizeC[0][k] != 'F')
						{
							for (j=10000; j>0; j--);	 	   // Delay
							lcd_Data(filessizeC[0][k]);        //print char
							k+=1;
						}
						
						for (j=10000; j>0; j--);	 	      // Delay
						lcd_Data('B');
						for (j=10000; j>0; j--);	
				} // end of if(topFile==numoffiles-1)
				else{	
					int i ;
					int j = 0;
					int k = 0;
					for (i=topFile; i<=1+topFile; i++){
						while (filesname[i][k] != '.')
						{
							for (j=10000; j>0; j--);	 	  // Delay
							lcd_Data(filesname[i][k]);        //print char
							k+=1;
						}
					
						k = 0;
						for (j=10000; j>0; j--);	 	      // Delay
						LCDcommand (0x14);					  // Move cursor right
		
						while (filessizeC[i][k] != 'F')
						{
							for (j=10000; j>0; j--);	 	   // Delay
							lcd_Data(filessizeC[i][k]);        //print char
							k+=1;
						}
						
						k=0;
						for (j=10000; j>0; j--);	 	      // Delay
						lcd_Data('B');
						for (j=10000; j>0; j--);	          // Delay
						if (i==topFile)
						{
							LCDcommand(0xc0);        	      //second line
							for (j=10000; j>0; j--);	     // Delay
						}
					} // end of for
				} // end of else 
			} // end of if (scroll == 0)
			else{ //scroll = 1
				int i, j;
				if (filessize[topFile]>32*topContent){
					LCDcommand (0x01);
					for (i=32*topContent; i<MIN(filessize[topFile],32*topContent+32); i++){
						for (j=10000; j>0; j--);	 	  // Delay
						if (i==32*topContent+16){
							LCDcommand(0xc0);        	  //second line
							for (j=10000; j>0; j--);	  // Delay
						}
						lcd_Data(files[topFile][i]);      //print char
					}
					topContent += 1;
				}
			} // end of if ISFR
		} // end of if state = 1
		//Debounce or using PFE field
		while(!(GPIOD_PDIR & SW6_POS) );// wait of release the button
		for(i=10000 ; i>0 ; i--); //delay, button debounce
		
		PORTD_ISFR |= 0x00000040;  // clear interrupt flag bit of PTD7
	}
}
//-----------------------------------------------------------------
//  UART0 - ISR = Interrupt Service Routine
//-----------------------------------------------------------------
void UART0_IRQHandler(){
	
	uint8_t Temp;
		
	if(UART0_S1 & UART_S1_RDRF_MASK){ // RX buffer is full and ready for reading
		
		Temp = UART0_D;
		if (Temp != '\n'){         //insert chars to array until pressing Enter
			pcmessage[arridx] = Temp;   
			arridx += 1;
		}
		else{
			arridx = 0;
			dataready = 1;
		}	
	}
}

//-----------------------------------------------------------------
// PIT - ISR = Interrupt Service Routine
//-----------------------------------------------------------------
void PIT_IRQHandler(){
}

//-----------------------------------------------------------------
// Function - Set color
//-----------------------------------------------------------------
void set_color(){
		
	if (strcmp(pcmessage,"set blue\r") == 0){
		RGB_LED_OFF;
		BLUE_LED_ON;
	}
	if (strcmp(pcmessage,"set cyan\r") == 0){
		RGB_LED_OFF;
		BLUE_LED_ON;
		GREEN_LED_ON;
	}
	if (strcmp(pcmessage,"set green\r") == 0){
		RGB_LED_OFF;
		GREEN_LED_ON;
	}
	if (strcmp(pcmessage,"set magenta\r") == 0){
		RGB_LED_OFF;
		BLUE_LED_ON;
	    RED_LED_ON;
	}
	if (strcmp(pcmessage,"set red\r") == 0){
		RGB_LED_OFF;
		RED_LED_ON;
	}
	if (strcmp(pcmessage,"set white\r") == 0){
		RGB_LED_OFF;
		RED_LED_ON;
		BLUE_LED_ON;
	    GREEN_LED_ON;
	}
	if (strcmp(pcmessage,"set yellow\r") == 0){
		RGB_LED_OFF;
		RED_LED_ON;
	    GREEN_LED_ON;  
	}
}

void set_files_menu(){
	LCDcommand (0x01);
	int i = 0;
	int j = 0;
	int k = 0;
	for (i=0; i<MIN(numoffiles,2); i++){
		while (filesname[i][k] != '.')
		{
			for (j=10000; j>0; j--);	 	  // Delay
			lcd_Data(filesname[i][k]);        //print char
			k+=1;
		}
	
		k = 0;
		for (j=10000; j>0; j--);	 	      // Delay
		LCDcommand (0x14);					  // Move cursor right

		while (filessizeC[i][k] != 'F')
		{
			for (j=10000; j>0; j--);	 	   // Delay
			lcd_Data(filessizeC[i][k]);        //print char
			k+=1;
		}
		
		k=0;
		for (j=10000; j>0; j--);	 	      // Delay
		lcd_Data('B');
		for (j=10000; j>0; j--);	          // Delay
		if (i==0)
		{
			LCDcommand(0xc0);        	      //second line
			for (j=10000; j>0; j--);	     // Delay
		}
	}
}

void receive_file(int index){
		int i = 6;  		// index for pcmessage array
		int j = 0;			// index for filesname array
		int power = 0;		// calculate power for size
		int startsize = 0;  // temporary
		
		while (!((pcmessage[i] == '.') & (pcmessage[i+1] == 't') & 
				 (pcmessage[i+2] == 'x') & (pcmessage[i+3] == 't'))){
			filesname[index][j] = pcmessage[i];
			j+=1;
			i+=1;
		}
		filesname[index][j] = '.'; j+=1;
		filesname[index][j] = 't'; j+=1;
		filesname[index][j] = 'x'; j+=1;
		filesname[index][j] = 't'; i+=5; j=0;
		
		startsize = i;
		while (pcmessage[i] != '\r'){
			power += 1;
			i+=1;
		}
		
		i = startsize;
		j = 0;
		while (pcmessage[i] != '\r'){
			fs = fs + (pow(10,power-1)*(pcmessage[i]-'0'));
			filessizeC[index][j] = pcmessage[i];
			power -= 1; 
			i += 1;
			j += 1;
		}
		
		filessizeC[index][j] = 'F';
		memset(pcmessage,0,40);                   //memset - clears the array
		filessize[index] = fs;
		fs=0;
		
		//receiving part:
		DMA_DAR0 = (uint32_t)&files[index];       				//destination
		DMA_DSR_BCR0 = DMA_DSR_BCR_BCR(filessize[index]);       // number of bytes yet to be transferred
		DMAMUX0_CHCFG0 |= DMAMUX_CHCFG_ENBL_MASK; 				// Enable DMA channel 
		disable_irq(INT_UART0-16);               			    // Disable UART0 interrupt
		UART0_C5 |= UART0_C5_RDMAE_MASK;          				// Enable DMA request for UART0 receiver
		UARTprintf(UART0_BASE_PTR,"ack\n");  
}







