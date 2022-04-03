#include <asf.h>

void clock_setup(void);
void Port_setup(void);
void ADC_setup(void);
void convert_to_digit(int);										//Function to identify each specific digit 
void display(int *a);											//Function to choose which ss to display on
void number_display(int a);										//Function to find specific number to display
void wait(int t);

volatile int digit_array[3];
volatile int *arrpnt;

int main (void)
{	
	volatile int ADC_output;
	volatile int actual_volt;
	
	arrpnt = digit_array;										//Pointer to array hold`ing decimal digits
	
	clock_setup();
	Port_setup();
	ADC_setup();
	
	while(1)
	{
		if(ADC->INTFLAG.bit.RESRDY)            //Poll RESRDY bit to determine when conversion is complete
		{	
			while(ADC->STATUS.reg == ADC_STATUS_SYNCBUSY){}    //Read result register after synchronization complete
			ADC_output = ADC->RESULT.reg;					   //read result register
		}

		actual_volt = (330*ADC_output)/4095;				 //Equation to convert voltages 
		
		convert_to_digit(actual_volt);						//Function to separate digits for display
			
		display(arrpnt);									//Function to display digits on seven segment
		
		
	
	}
}

void clock_setup(){

//setting up the clock source

	/* Various bits in the INTFLAG register can be set to one at startup.
	   This will ensure that these bits are cleared. User should clear bits before using corresponding interrupts p.176  */
	SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET | SYSCTRL_INTFLAG_DFLLRDY;


//Setting the clock source 

	SYSCTRL->OSC8M.reg |= SYSCTRL_OSC8M_ENABLE; //ENABLE 8mHZ CLOCK
	SYSCTRL->OSC8M.reg |= SYSCTRL_OSC8M_FRANGE_2; //Frequency range contol 2= 6MHz-8MHz
	SYSCTRL->OSC8M.bit.PRESC = 0; //Prescaler factor, will divide main clock by this factor  0 = divide by 1
	SYSCTRL->OSC8M.reg |= SYSCTRL_OSC8M_ONDEMAND; // Will run main clock only when requested by peripheral p.152
	 
	
//Setting up the generic clock synchronization	
	GCLK->CTRL.reg = GCLK_CTRL_SWRST; //Software reset started
	while(GCLK->STATUS.reg == GCLK_STATUS_SYNCBUSY){}
	
	
//Configuring generic clock generator	
	GCLK->GENDIV.reg = 0<<1 | 1<<8;
							//GENERATOR 0 SELECTION
							//1 division bit set -> divide source by 1 p.114
							
			   
	GCLK->GENCTRL.reg = 0<<1 | GCLK_GENCTRL_RUNSTDBY | 0<<20 | GCLK_GENCTRL_OE | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSC8M; 
							//Generic clock generator 0 ID selected
							//run in standby 
							//0<<20 = divide Selection is generator source divided by GENDIV.DIV     
							//Output enable   
							//enable gen Clk ctr 
							//8MHz clock source		
									
					
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_EIC | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0;
							//generic clock configured for EIC
							//generic clock enable
							//generic clock generator 0 selected

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0;
							//generic clock configured for ADC
							//generic clock enable
							//generic clock generator 0 selected	

	//GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_SERCOM4_CORE | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0;
							//generic clock configured for Sercom4
							//generic clock enable
							//generic clock generator 0 selected
							
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_DAC | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0;						
							//Generic clock setup for DAC
	
	while(GCLK_STATUS_SYNCBUSY==1){}
							//wait for synchronization of clocks to complete	
	
	
//Setting up the Power Manager (CPU and clock buses AHB AND APB)
	PM->SLEEP.reg = PM_SLEEP_IDLE_APB;  
	//CPU, AHB, AND APB Clock domains stopped until called
	
	PM->CPUSEL.reg = PM_CPUSEL_CPUDIV_DIV1;
	//PRESCALER : DIVIDES THE MAIN CLOCK SOURCE BY 1, THIS IS CLOCK THAT RUNS CPU
	PM->APBASEL.reg = PM_APBASEL_APBADIV_DIV1;
	//PRESCALER: DIVIDES THE  MAIN CLOCK BY 1, THIS CLOCK RUNS THE APBA TO PERIPHERAL
	PM->APBBSEL.reg = PM_APBBSEL_APBBDIV_DIV1; //no division
	PM->APBCSEL.reg = PM_APBCSEL_APBCDIV_DIV1; //no division
	
	PM->APBAMASK.reg = PM_APBAMASK_EIC;  //Enable EIC clock
	PM->APBBMASK.reg = PM_APBBMASK_PORT; //Enable Port io clock
	//PM->APBCMASK.reg = PM_APBCMASK_SERCOM4; //Enable Sercom 4
	PM->APBCMASK.reg |= 1<<16; //ADC clock enable
	PM->APBCMASK.reg |= 1<<21; //DAC clock enable
	
}

void ADC_setup()
{	
	ADC->CTRLA.reg = ADC_CTRLA_SWRST; 
				//reset ADC software
	while(ADC->STATUS.reg == ADC_STATUS_SYNCBUSY);
				//synchronization must be complete to enable ADC 
	ADC->REFCTRL.reg = ADC_REFCTRL_REFCOMP | ADC_REFCTRL_REFSEL_INTVCC1;
				//ADC reference buffer offset compensation enable
				//1/2 VDDANA reference selected
	ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1;
				//number of samples collected = 1
	ADC->SAMPCTRL.reg = 0x1; 
				//Sampling time = (SAMPLEN +1)(ADC_clock/2) = ADC_clock since SAMPLEN set to 1. 			
	ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 | ADC_CTRLB_RESSEL_12BIT | ADC_CTRLB_FREERUN;
				//ADC clock prescaler 8mhz/8=1Mhz
				//conversion result resolution = 12 bits
				//freerun mode
	ADC->INPUTCTRL.reg = ADC_INPUTCTRL_GAIN_DIV2 | ADC_INPUTCTRL_MUXNEG_GND | ADC_INPUTCTRL_MUXPOS_PIN19;
				//1/2(x) gain selected
				//negative mux input = GND
				//positive mux input = pin 20 (AIN19) (PORT PA11 POTENTIOMETER WIPER)	
	ADC->DBGCTRL.reg = ADC_DBGCTRL_DBGRUN;
				//ADC will run in debug mode
				
	ADC->INTENSET.reg = ADC_INTENSET_RESRDY;	
				//interrupt enable		
	ADC->SWTRIG.bit.START = 1;
				//Needed to start the first conversion. 			
	ADC->CTRLA.reg = ADC_CTRLA_ENABLE | ADC_CTRLA_RUNSTDBY;
				//Enable ADC
				//Configured to run in standby
				
	
	
}

void convert_to_digit(int a)
{
				 a = a % 1000;
	   digit_array[0] = a/100;
	 	
				  a = a % 100;		
	    digit_array[1] = a/10;
				
				   a = a % 10;
	       digit_array[2] = a;	
}

void Port_setup()
{
	Port *ports = PORT_INSTS;
	PortGroup *porA = &(ports->Group[0]);
	
	//ADC port setup
	porA->DIRCLR.reg = PORT_PA11;  //set to input for incoming analog signal (wiper)
	porA->PMUX[2].bit.PMUXO = 0x1; //set PA04 to Peripheral B which includes ADC
	porA->PINCFG[4].bit.PMUXEN = 0x1; //Enable mux to PA04 
	
	
	PortGroup *porB = &(ports->Group[1]);
	
	//Seven segment display setup 
	porA->DIRSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;	// rows set as outputs

	porB->DIRSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06 |PORT_PB07;  //set ss as outputs
	porB->OUTSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06 |PORT_PB07;  //reset display
	
	//ADC/Potentiometer setup
	porA->DIRCLR.reg = PORT_PA11;  //set to input for incoming analog signal (wiper)
	porA->PINCFG[5].bit.PMUXEN = 0x1; //MUX wiper to ADC
	
	porA->DIRSET.reg = PORT_PA12;  //Set the top of Pot to output
	porA->OUTSET.reg = PORT_PA12;  //Set the pot output to 3.3V	
	
	wait(10); 
}

void display(int *a)
{	
	Port *ports = PORT_INSTS;
	PortGroup *porA = &(ports->Group[0]);
	PortGroup *porB = &(ports->Group[1]);
	porA->DIRSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;	// rows set as outputs

	porB->DIRSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06 |PORT_PB07;  //set ss as outputs
	porB->OUTSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06 |PORT_PB07;  //reset display
	
	volatile int count=0;

	while(count < 4)										// switch statement to power rows individually
	{
		switch(count++)
		{			
			case 0:
			number_display(*a++);
			porA->OUTCLR.reg = PORT_PA07;				//power on first row and show first element in read array
			wait(1);
			porA->OUTSET.reg = PORT_PA07;				//power off
			break;
		
			case 1:
			number_display('.');
			porA->OUTCLR.reg = PORT_PA06;
			wait(1);
			porA->OUTSET.reg = PORT_PA06;
			break;
		
			case 2:
			number_display(*a++);
			porA->OUTCLR.reg = PORT_PA05;
			wait(1);
			porA->OUTSET.reg = PORT_PA05;
			break;
		
		
			case 3:
			number_display(*a);
			porA->OUTCLR.reg = PORT_PA04;
			wait(1);
			porA->OUTSET.reg = PORT_PA04;
			break;	
		}
	}
	count=0;
}
		
void number_display(int a)
{	
	Port *ports = PORT_INSTS;
	PortGroup *porB = &(ports->Group[1]);
	porB->OUTSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06 | PORT_PA07;  //reset display
	
	
	switch(a)
	{	
		case 0:
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05;  //display 0	
		break;
		
		case 1:
		porB->OUTCLR.reg = PORT_PB01 | PORT_PB02;		//display 1
		break;
		
		case 2:
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB03 | PORT_PB04 | PORT_PB06;  //display 2
		break;
		
		case 3:
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB06;		//display 3
		break;
		
		case 4:
		porB->OUTCLR.reg = PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;  // display 4
		break;
		
		case 5:
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB05 | PORT_PB06 | PORT_PB02 | PORT_PB03;		//display 5
		break;
		
		case 6:
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB05 | PORT_PB04 | PORT_PB03 | PORT_PB02 | PORT_PB06;  //display 6
		break;
		
		case 7:
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02;			// display 7
		break;
		
		case 8:
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;  //display 8
		break;
		
		case 9:
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;	//display 9
		break;
		
		case '.':
		porB->OUTCLR.reg = PORT_PB07;   //display "."
		break;		
		
		default:
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01;
		break;
	}
}

void wait(int t)
{
	volatile int count=0;
	
	count = 0;
	while (count < t*1000)
	{
		count++;
	}
}