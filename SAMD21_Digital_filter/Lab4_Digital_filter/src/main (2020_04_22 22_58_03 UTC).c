#include <asf.h>

void clock_setup(void);
void DAC_setup(void);
void ADC_setup(void);
void Port_setup(void);
void Timer4Setup(void);
void TC4_Handler(void);
void wait(volatile int d);

volatile float y=0;				//y=output
volatile float y1=0;
volatile float y2=0;
volatile float y_int=0;           //used for conversion of y float to y int

volatile float x=0;				//x=input
volatile float x1=0;
volatile float x2=0;
volatile float x_int=0;


int main (void)
{	
	clock_setup();
	Port_setup();
	ADC_setup();
	DAC_setup();
	Timer4Setup();
	
	while(1){	
	}

}



void clock_setup(){


	SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET | SYSCTRL_INTFLAG_DFLLRDY;

//Setting the clock source 

	
	//SYSCTRL->OSC8M.reg |= SYSCTRL_OSC8M_FRANGE_2; //Frequency range contol 2= 6MHz-8MHz
	SYSCTRL->OSC8M.bit.PRESC = 0; //Prescaler factor, will divide main clock by this factor  0 = divide by 1
	SYSCTRL->OSC8M.reg |= SYSCTRL_OSC8M_ONDEMAND; // Will run main clock only when requested by peripheral p.152
	SYSCTRL->OSC8M.reg |= SYSCTRL_OSC8M_ENABLE; //ENABLE 8mHZ CLOCK 
	
//Setting up the generic clock synchronization	
	GCLK->CTRL.reg = GCLK_CTRL_SWRST; //Software reset started
	while(GCLK->STATUS.reg == GCLK_STATUS_SYNCBUSY){}
	
	
//Configuring generic clock generator	
	GCLK->GENDIV.reg = 0<<0 | 1<<8;
							//GENERATOR 0 SELECTION
							//1 division bit set -> divide source by 1 p.114
							
			   
	GCLK->GENCTRL.reg =  GCLK_GENCTRL_ID(0)  | GCLK_GENCTRL_RUNSTDBY | 0<<20 | GCLK_GENCTRL_OE | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSC8M; 
							//Generic clock generator 0 ID selected
							//run in standby 
							//0<<20 = divide Selection is generator source divided by GENDIV.DIV     
							//Output enable   
							//enable gen Clk ctr 
							//8MHz clock source		
		
	while (GCLK->STATUS.bit.SYNCBUSY){}	//waiting for sync to complete
	GCLK->CLKCTRL.reg |= 0<<14;	//disable clock	
									
					
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_EIC | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0;
							//generic clock configured for EIC
							//generic clock enable
							//generic clock generator 0 selected

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0;
							//generic clock configured for ADC
							//generic clock enable
							//generic clock generator 0 selected	
							
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_DAC | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0;						
							//Generic clock setup for DAC
	
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TC4_TC5 | GCLK_CLKCTRL_GEN_GCLK0 | 1<<14;	
							//setup genCLK for TC4
	
	while(GCLK->STATUS.reg == GCLK_STATUS_SYNCBUSY){}
							//wait for synchronization of clocks to complete	
	
	
//Setting up the Power Manager (CPU and clock buses AHB AND APB)
	//PM->SLEEP.reg = PM_SLEEP_IDLE_APB;  
	//CPU, AHB, AND APB Clock domains stopped until called
	
	PM->CPUSEL.reg = PM_CPUSEL_CPUDIV_DIV1;
	//PRESCALER : DIVIDES THE MAIN CLOCK SOURCE BY 1, THIS IS CLOCK THAT RUNS CPU
	PM->APBASEL.reg = PM_APBASEL_APBADIV_DIV1; //PRESCALER: DIVIDES THE  MAIN CLOCK BY 1, THIS CLOCK RUNS THE APBA TO PERIPHERAL
	PM->APBBSEL.reg = PM_APBBSEL_APBBDIV_DIV1; //divide by 1
	PM->APBCSEL.reg = PM_APBCSEL_APBCDIV_DIV1; //divide by 1
	
	PM->APBAMASK.reg |= PM_APBAMASK_EIC;  //Enable EIC clock
	PM->APBBMASK.reg |= PM_APBBMASK_PORT; //Enable Port io clock
	PM->APBCMASK.reg |= 1<<16; //ADC clock enable
	PM->APBCMASK.reg |= PM_APBCMASK_DAC; //Enable ADC clock
	PM->APBCMASK.reg |= PM_APBCMASK_TC4;	//enable the TC4 APB
	
}

void DAC_setup()
{
	DAC->CTRLA.reg = DAC_CTRLA_SWRST;	//Software reset
	while(DAC->STATUS.bit.SYNCBUSY);
	
	DAC->CTRLB.reg = DAC_CTRLB_REFSEL_AVCC | DAC_CTRLB_EOEN;
	//AVCC reference
	//Drive output to Vout pin
	
	DAC->CTRLA.reg = 1<<1;  //Enable DAC 
	while(DAC->STATUS.reg == DAC_STATUS_SYNCBUSY);
}

void ADC_setup()
{
	ADC->CTRLA.reg |= 0<<1;	
	while(ADC->STATUS.reg == ADC_STATUS_SYNCBUSY);
	ADC->CTRLA.reg = ADC_CTRLA_SWRST;
		//reset ADC software
	while(ADC->STATUS.reg == ADC_STATUS_SYNCBUSY);
		//synchronization must be complete to enable ADC
	ADC->REFCTRL.reg = ADC_REFCTRL_REFCOMP | ADC_REFCTRL_REFSEL_INTVCC1;
		//ADC reference buffer offset compensation enable
		//1/2 VDDANA reference selected
	//ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1;
		//number of samples collected = 1
	ADC->SAMPCTRL.reg = 0x1;
		//Sampling time = (SAMPLEN +1)(ADC_clock/2) = ADC_clock since SAMPLEN set to 1.
	ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 | ADC_CTRLB_RESSEL_12BIT;
		//ADC clock prescaler 8mhz/8=1Mhz
		//conversion result resolution = 12 bits
		//freerun mode
	ADC->INPUTCTRL.reg = ADC_INPUTCTRL_GAIN_DIV2 | ADC_INPUTCTRL_MUXNEG_GND | ADC_INPUTCTRL_MUXPOS_PIN1;
		//1/2(x) gain selected
		//negative mux input = GND
			//positive mux input = pin 1 -> PA03
	ADC->DBGCTRL.reg = ADC_DBGCTRL_DBGRUN;
		//ADC will run in debug mode
	
	//ADC->INTENSET.reg = ADC_INTENSET_RESRDY;
		//interrupt enable
	ADC->SWTRIG.bit.START = 1;
	while(ADC->STATUS.bit.SYNCBUSY== 1){}                                                                                                                                    
		//Needed to start the first conversion.
	ADC->CTRLA.reg = ADC_CTRLA_ENABLE | ADC_CTRLA_RUNSTDBY;
	while(ADC->STATUS.reg == ADC_STATUS_SYNCBUSY){}
		//Enable ADC
		//Configured to run in standby
	
}


void Port_setup()
{
	Port *ports = PORT_INSTS;
	PortGroup *porA = &(ports->Group[0]);
	PortGroup *porB = &(ports->Group[0]);

	//porA->DIRCLR.reg = PORT_PA03;  //set to input terminal for ADC output 
	porA->PMUX[1].bit.PMUXO = 0x1; //set PA03 to Peripheral B which includes ADC
	porA->PINCFG[3].bit.PMUXEN = 0x1; //Enable mux to PA03 
	
	//DAC output setup
	//porA->DIRSET.reg = PORT_PA02;		//Set VOUT terminal for DAC output signal
	porA->PMUX[1].bit.PMUXE = 0x1;		//Peripheral B enabled for DAC
	porA->PINCFG[2].bit.PMUXEN = 0x1;	//Routed to PA02
	
	porA->DIRSET.reg |= PORT_PA12;
}



void Timer4Setup(void)
{	
	
	NVIC->ISER[0] |= 1<<19;  //Enable Interrupt for TC4	
	
	Tc *tc = TC4;
	TcCount8 *tcc = &tc->COUNT8;						//tcc->_______.reg equivalent to writing TC4->COUNT8.________.reg=
	tcc->CTRLA.reg = 0;	//disable the TC4
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
	tcc->CTRLA.bit.PRESCALER = 0x5;	//divide by 64;
	tcc->CTRLA.bit.WAVEGEN = 0x1;	//match frequency CC0=period, toggles on overflow
	tcc->CTRLA.bit.MODE = 0x1;	//8 bit mode
	tcc->CC[0].reg = 0xF0;			//= 125 decimal
	tcc->INTENSET.bit.OVF = 0x1;  //Enable overflow interrupt
	tcc->CTRLBSET.bit.ONESHOT = 0;	//turn off one shot mode
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync to complete
	tcc->CTRLA.reg |= 1<<1;	//enable the TC4
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync of disabled
}

void TC4_Handler(void)
{
	
	Port *ports = PORT_INSTS;
	PortGroup *porA = &(ports->Group[0]);
	PortGroup *porB = &(ports->Group[0]); 
	
	//porA->OUTSET.reg = PORT_PA12;
	
	Tc *tc = TC4;
	TcCount8 *tcc = &tc->COUNT8;
	
	
	
	
	ADC->SWTRIG.bit.START = 1;
	while(ADC->STATUS.bit.SYNCBUSY== 1){}

	while(ADC->STATUS.bit.SYNCBUSY== 1){}     //Read result register after synchronization complete
	x_int = ADC->RESULT.reg;								//read result register
	
	x = (3.30*x_int)/4095;					//convert x to floating point 
	
	
	y = (x) - (1.48*x1) + (0.9891*x2) + (1.373*y1) - (0.8819*y2);			//difference equation
	
	
	y_int = (1023*y)/3.3;										//convert float y to int for DAC input
	
	
	DAC->DATA.reg = y_int;							//output data to DAC
				
	y2=y1;
	y1=y; 
	x2=x1;
	x1=x;
	
	
	tcc->INTFLAG.bit.OVF = 1;	//clear the interrupt
	
	//porA->OUTCLR.reg = PORT_PA12;
}


void wait(volatile int d){
	int count = 0;
	while (count < d*10){
		count++;
	}
}