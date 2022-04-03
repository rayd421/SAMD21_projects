#include <asf.h>

void EICsetup(void);
void clockSetup(void);
void wait(volatile int d);
void portSetup(void);
void ADCsetup(void);
void convert(int t);
void Timer6Setup(void);
void Timer4Setup(void);
void waveGen(void);
void display(int *a);

volatile uint8_t result = 0;
volatile int temp = 0;
volatile int temp2 = 0;
volatile static int countEICs = 0;
volatile int temp1 = 0;
volatile float y=0;
volatile float y1=0;
volatile float y2=0;
volatile float x=0;
volatile float x1=0; 
volatile float x2=0;
volatile int array[4];
volatile int *ptr;

int main (void){
	
	system_init();
	clockSetup();
	portSetup();
	ADCsetup();
	Timer6Setup();
	EICsetup();
	Timer4Setup();

	////////////priorities/////////////
	NVIC->IP[1] = 0x80;	//EIC lowest priority
	NVIC->IP[4] = 0x40000000;	//TC5 highest, TC6 mid priority
	
	ptr = array;
	Port *por = PORT;
	PortGroup *porB = &(por->Group[1]);
	PortGroup *porA = &(por->Group[0]);
	porA->OUTSET.reg = PORT_PA12;	//powering POT
	porA -> OUTSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;//turn off SSDs
	//porB->DIRSET.reg = PORT_PB16;	//for testing
	ADC->SWTRIG.bit.START = 1;	//start conversion
	while(1){
		waveGen();
		
		
		//if(temp2 == 1){
		//porA -> OUTCLR.reg = PORT_PA04;//turn on left SSD
		//porB -> OUTSET.reg = PORT_PB07;//display "0"
		//porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;//display "0"
		//porA -> OUTSET.reg = PORT_PA04;//turn on left SSD
		//
		//}
		//if(temp2 == 0){
		//porA -> OUTCLR.reg = PORT_PA07;//turn on left SSD
		//porB -> OUTSET.reg = PORT_PB07 | PORT_PB06;//display "0"
		//porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05;//display "0"
		//porA -> OUTSET.reg = PORT_PA07;//turn on left SSD
		//
		//}
		
		
		if(temp2 == 1){
			porB -> OUTSET.reg = PORT_PB09;	//turn off LED
			wait(1);
		}
		if(temp2 == 0){
			porB -> OUTCLR.reg = PORT_PB09;	//turn on LED
			wait(1);
		}
		convert(temp1);
		
		porA->OUTCLR.reg = 	PORT_PA07;
		display(ptr);
		wait(1);
		porA->OUTSET.reg = 	PORT_PA07;

		porA->OUTCLR.reg = 	PORT_PA06;
		display(ptr + 1);
		wait(1);
		porA->OUTSET.reg = 	PORT_PA06;
		
		porA->OUTCLR.reg = 	PORT_PA05;
		display(ptr + 2);
		wait(1);
		porA->OUTSET.reg = 	PORT_PA05;
		
		porA->OUTCLR.reg = 	PORT_PA04;
		display(ptr + 3);
		wait(1);
		porA->OUTSET.reg = 	PORT_PA04;
		
		
	}
}

void clockSetup(void){

	SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET | SYSCTRL_INTFLAG_DFLLRDY;
	SYSCTRL->OSC8M.bit.PRESC = 0;	//divide the clock by 1
	SYSCTRL->OSC8M.bit.ONDEMAND = 1;	//clock is off is no peripheral request
	SYSCTRL->OSC8M.bit.RUNSTDBY = 0;	//clock is off in sleep mode
	SYSCTRL->OSC8M.reg |= 1<<1;	//enable clock
	//SYSCTRL->OSC8M.bit.FRANGE = 2;	//yet another way to control manipulate the clock freq
	
	SYSCTRL->OSC32K.bit.STARTUP = 0;	//start up asap
	SYSCTRL->OSC32K.bit.ONDEMAND = 1;	//clock is off if no peripheral request
	SYSCTRL->OSC32K.bit.RUNSTDBY = 1;	//clock is on in sleep mode
	SYSCTRL->OSC32K.bit.EN32K = 1;	//enable output
	SYSCTRL->OSC32K.reg |= 1<<1;	//enable clock
	
	GCLK->CTRL.bit.SWRST = 1;	//reset the generators
	while (GCLK->STATUS.bit.SYNCBUSY){}	//waiting for the reset to complete
	
	GCLK->GENDIV.reg = GCLK_GENDIV_ID(0) | GCLK_GENDIV_DIV(1);	//divide generator0 by 1
	GCLK->GENDIV.reg = GCLK_GENDIV_ID(1) | GCLK_GENDIV_DIV(1);	//divide generator1 by 1

	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_OE |
	GCLK_GENCTRL_SRC_OSC8M | GCLK_GENCTRL_RUNSTDBY;
	
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(1) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_OE |
	GCLK_GENCTRL_SRC_OSC32K | GCLK_GENCTRL_RUNSTDBY;
	
	while (GCLK->STATUS.bit.SYNCBUSY){}	//waiting for sync to complete
	GCLK->CLKCTRL.reg |= 0<<14;	//disable clock

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC | GCLK_CLKCTRL_GEN_GCLK0 | 1<<14;	//setup genclk for ADC
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TC4_TC5 | GCLK_CLKCTRL_GEN_GCLK0 | 1<<14;	//setup genCLK for TC5
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_EIC | GCLK_CLKCTRL_GEN_GCLK0 | 1<<14;	//setup genclk for EIC...this is a single write method i believe
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TC6_TC7 | GCLK_CLKCTRL_GEN_GCLK0 | 1<<14;	//setup genCLK for TC6

	while (GCLK->STATUS.bit.SYNCBUSY==1){}	//waiting for sync to complete
	
	PM->CPUSEL.bit.CPUDIV = 0;	//divide CPU clock by 1	pg 15
	PM->APBASEL.bit.APBADIV = 0;	//divide apba bus by 1
	PM->APBBSEL.bit.APBBDIV = 0;	//divide apbb bus by 1
	PM->APBCSEL.bit.APBCDIV = 0;	//divide apbc bus by 1
	PM->APBAMASK.reg |= 1<<3;	//enable the GCLK clock DONT THINK NECESSARY they should be enabled by default pg 159
	PM->APBCMASK.reg |= 1<<16;	//enable the ADC APB
	PM->APBAMASK.reg |= 1<<6;	//enable the EIC APB
	PM->APBCMASK.reg |= PM_APBCMASK_TC4;	//enable the TC5 APB
	PM->APBCMASK.reg |= PM_APBCMASK_TC6;	//enable the TC6 APB

}

void portSetup(void){
	Port *por = PORT;
	PortGroup *porA = &(por->Group[0]);
	PortGroup *porB = &(por->Group[1]);
	porA->DIRSET.reg = 1<<12;	//setting up PA12 as output
	porA->PMUX[5].bit.PMUXO = 0;	//mux the ADC to pin PA11 (6=2*n)	AIN[4]
	porA->PINCFG[11].bit.PMUXEN =1;	//enable the MUX
	porB->PMUX[6].bit.PMUXE = 4;	//mux the TC wavegen PB12	PWM output
	porB->PINCFG[12].bit.PMUXEN = 1;	//enable the mux
	porB->PMUX[6].bit.PMUXO = 4;	//mux the TC wavegen PB13	PWM output
	porB->PINCFG[13].bit.PMUXEN = 1;	//enable the mux
	porA->PMUX[14].bit.PMUXO = 0;	//mux the EIC to pin PA28 EXTINT[8] for encoder
	porA->PINCFG[28].bit.PMUXEN =1;	//enable the MUX
	porB->DIRCLR.reg = 1<<14;	//set as input for channel B for encoder
	porB->PINCFG[14].bit.INEN =1;	//enable the MUX
	porB->DIRSET.reg = PORT_PB09;	//sign LED
	porA -> DIRSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;//outputs
	porB -> DIRSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06 | PORT_PB07;//LED segments
}

void EICsetup(void){
	//might need to disable EIC first
	EIC->CTRL.bit.SWRST = 1;	//reset the EIC and disables the EIC
	while(EIC->STATUS.bit.SYNCBUSY){}	//wait for synchronization
	EIC->INTENSET.reg = 1<<8;	//enables PA28 for external interrupt EXTINT[8]
	EIC->CONFIG[1].bit.SENSE0 = 2;	//select falling edge as triggering ...config[n]..(n*8+x)=y...EXT[y]....SENSEx...n=0|1
	EIC->CTRL.reg |= 1<<1;	//enable the EIC at the end
	NVIC->ISER[0] = 1<<4;	//enable the NVIC handler
}

void wait(volatile int d){
	int count = 0;
	while (count < d*1000){
		count++;
	}
}

void ADCsetup(void){
	ADC->CTRLA.reg = 0<<1;	//disable so that we can reset
	while (ADC->STATUS.bit.SYNCBUSY==1){}	//wait for disable to complete
	ADC->CTRLA.bit.SWRST = 1;	//reset
	while (ADC->STATUS.bit.SYNCBUSY==1){}	//wait for reset to complete
	ADC->REFCTRL.reg = ADC_REFCTRL_REFSEL_INTVCC1;	//internal reference = .5VDDann
	//ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV512 | ADC_CTRLB_RESSEL_8BIT | ADC_CTRLB_FREERUN | 0<<0 | ADC_CTRLB_CORREN;
	ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV512 | ADC_CTRLB_RESSEL_8BIT | 0<<2 | 0<<0;
	while (ADC->STATUS.bit.SYNCBUSY==1){}	//wait for sync to complete
	ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXNEG_GND | ADC_INPUTCTRL_MUXPOS_PIN19;	//pin19=AIN19=PA11
	ADC->INPUTCTRL.bit.GAIN = 0xF;	//gain = 1/2
	while (ADC->STATUS.bit.SYNCBUSY==1){}	//wait for sync to complete
	
	//loads calibration values from NVM
	short *NVM = (short *) 0x806023;	//address of the ADC calib
	short temp1 = *NVM;
	short temp2 = temp1 & 0x7F8;	//masking unnecessary bits
	temp2 = temp2 >>3;	//shift to the right location
	ADC->CALIB.bit.LINEARITY_CAL = temp2;
	short temp3 = temp1 & 0x3800;	//masking unnecessary bits
	temp3 = temp3 >>11;	//shift to the right location
	ADC->CALIB.bit.BIAS_CAL = temp3;
	ADC->CALIB.bit.LINEARITY_CAL = 0x2;
	ADC->CALIB.bit.BIAS_CAL = 0x7;
	
	ADC->SWTRIG.bit.START = 1;	//start conversion
	while (ADC->STATUS.bit.SYNCBUSY==1){}	//wait for sync to complete
	ADC->INTENSET.reg = ADC_INTENSET_RESRDY;	//setup interrupt when reg is ready to be read
	ADC->CTRLA.reg |= 1<<1;	//enable ADC
	while (ADC->STATUS.bit.SYNCBUSY==1){}	//wait for enable to complete
	NVIC->ISER[0] |= 1<<23;	//enable the NVIC handler
	ADC->OFFSETCORR.reg = 0b000000110100;	//shift down by 52, 2's comp
	ADC->GAINCORR.reg =   0b100010100000;	//when corren is enabled it enables gain comp too, fractional
}

void ADC_Handler(void){
	Port *por = PORT;
	PortGroup *porB = &(por->Group[1]);
	PortGroup *porA = &(por->Group[0]);
	result = ADC->RESULT.reg;
	if(result > 250){
		result = 250;
	}
	if(result < 5){
		result = 5;
	}
	if(result >= 150 || result <= 106 ){
		temp = 1;
	}
	else{
		temp = 2;
	}

	
	
}

void convert(int t){
	volatile int a = t;
	int i = 1000;   //divisor
	int j = 0;  //array counter
	//int a = 4312;   // our value
	//volatile char arrADC[4];
	//volatile char *arrPtr;
	//arrPtr = arrADC;
	int m = 1;  //counter
	int n = 1000;    //increment ot divisor

	while(j <= 3){
		int b = a % i;
		if(b == a) {
			int p = (m-1);
			switch(p) {
				case 0:
				array[j++] = 0;
				break;
				case 1:
				array[j++] = 1;
				break;
				case 2:
				array[j++] = 2;
				break;
				case 3:
				array[j++] = 3;
				break;
				case 4:
				array[j++] = 4;
				break;
				case 5:
				array[j++] = 5;
				break;
				case 6:
				array[j++] = 6;
				break;
				case 7:
				array[j++] = 7;
				break;
				case 8:
				array[j++] = 8;
				break;
				case 9:
				array[j++] = 9;
				break;
				default:
				array[j++] = 99;
				break;
			}
			a = a - (n*(m-1));
			m = 1;

			if(j == 1){
				i = 100;
				n = 100;
			}
			if(j == 2){
				i = 10;
				n = 10;
			}
			if(j == 3){
				i = 1;
				n = 1;
			}
		}
		else{
			m++;
			i = i + n;
		}
	}
}

void display(int *a){
	int i = 0;
	Port *ports = PORT_INSTS;
	PortGroup *porA = &(ports->Group[0]);
	PortGroup *porB = &(ports->Group[1]);
	
	switch(*a){
		
		case 0:
		porB -> OUTSET.reg = PORT_PB07 | PORT_PB06;//display "0"
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05;//display "0"
		a++;
		break;
		
		case 1:
		porB -> OUTSET.reg = PORT_PB00 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06 | PORT_PB07;//display "1"
		porB -> OUTCLR.reg = PORT_PB01 | PORT_PB02;//display "1"
		a++;
		break;
		
		case 2:
		porB -> OUTSET.reg =  PORT_PB02 | PORT_PB05 | PORT_PB07;//display "2"
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB03 | PORT_PB04 | PORT_PB06;//display "2"
		a++;
		break;
		
		case 3:
		porB -> OUTSET.reg = PORT_PB04 | PORT_PB05 | PORT_PB07;//display "3"
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB06;//display "3"
		a++;
		break;
		
		case 4:
		porB -> OUTSET.reg = PORT_PB00 | PORT_PB03 | PORT_PB04 | PORT_PB07;//display "4"
		porB -> OUTCLR.reg = PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;//display "4"
		a++;
		break;
		
		case 5:
		porB -> OUTSET.reg = PORT_PB01 | PORT_PB04 | PORT_PB07;//display "5"
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB03 | PORT_PB05 | PORT_PB06;//display "5"
		a++;
		break;
		
		case 6:
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;//display "6"
		porB -> OUTSET.reg = PORT_PB01 | PORT_PB07;//display "6"
		a++;
		break;
		
		case 7:
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02;//display "7"
		porB -> OUTSET.reg = PORT_PB03 | PORT_PB04| PORT_PB05 | PORT_PB06 | PORT_PB07;//display "7"
		a++;
		break;
		
		case 8:
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04| PORT_PB05 | PORT_PB06;//display "8"
		porB -> OUTSET.reg = PORT_PB07;//display "8"
		a++;
		break;
		
		case 9:
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;//display "9"
		porB -> OUTSET.reg = PORT_PB03 | PORT_PB04 | PORT_PB07;//display "9"
		a++;
		break;
		
		//case 99:
		//porB -> OUTSET.reg = PORT_PB06 | PORT_PB05 | PORT_PB04 | PORT_PB03 | PORT_PB02 | PORT_PB01 | PORT_PB00;//display "."
		//porB -> OUTCLR.reg = PORT_PB07;//display "."
		//a++;
		//break;
		
		default:
		porB -> OUTSET.reg = PORT_PB03 | PORT_PB07;//display "A"
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB04 | PORT_PB05 | PORT_PB06;//display "A"
		break;
	}
}

void Timer4Setup(void){	//for wavegen only
	Tc *tc = TC4;
	TcCount8 *tcc = &tc->COUNT8;
	tcc->CTRLA.reg = 0;	//disable the TC4
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
	tcc->CTRLA.bit.PRESCALER = 0;	//divide by 1;
	tcc->CTRLA.bit.WAVEGEN = 2;	//normal PWM frequency per=period, CC1/CC0=compare value
	tcc->CTRLA.bit.MODE = 1;	//8 bit mode
	tcc->PER.reg = 0xff;
	tcc->CTRLBSET.bit.ONESHOT = 0;	//turn off one shot mode
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync to complete
	//tcc->INTENSET.bit.OVF = 1;	//enable the overflow interrupt
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync to complete
	tcc->CTRLA.reg |= 1<<1;	//enable the TC6
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync of enable
	//NVIC->ISER[0] |= 1<<19;	//enable the NVIC handler for TC4
	//tcc->CC[0].reg = 0xF;	//period
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
	//tcc->CC[1].reg = 0xF;	//compare match
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
	tcc->CTRLC.bit.INVEN1 = 1;
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync

}

void Timer6Setup(void){
	Tc *tc = TC6;
	TcCount8 *tcc = &tc->COUNT8;
	tcc->CTRLA.reg = 0;	//disable the TC6
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
	tcc->CTRLA.bit.PRESCALER = 0x6;	//2^n; divide by 256
	tcc->CTRLA.bit.WAVEGEN = 3;	//normal PWM frequency per=period, CC1/CC0=compare value
	tcc->CTRLA.bit.MODE = 0;	//16 bit mode
	tcc->CTRLBSET.bit.ONESHOT = 0;	//turn off one shot mode
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync to complete
	tcc->INTENSET.bit.OVF = 1;	//enable the overflow interrupt
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync to complete
	tcc->CC[0].reg = 0x9c;	//period
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
	tcc->CTRLA.reg |= 1<<1;	//enable the TC6
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync of enable
	NVIC->ISER[0] |= 1<<21;	//enable the NVIC handler for TC6
}

void TC6_Handler(void){	//for RPM
	Port *por = PORT;
	PortGroup *porB = &(por->Group[1]);
	porB->DIRSET.reg = PORT_PB16;
	porB->OUTTGL.reg = PORT_PB16;		//test frequency
	
	x = countEICs*(60*200/400) ;//RPM calculation
	countEICs = 0;
	
	//y = (1.955*y1) - (0.9561*y2) + (0.0004861*x1) + (0.0004789*x2); //second order filter
	y = (0.9691*y1) + (0.03093*x1);				//first order filter
	temp1=y;
	
 	y1=y;
	//y2=y1;
 	x1=x;
	//x2=x1;
	
	Tc *tc = TC6;
	TcCount8 *tcc = &tc->COUNT8;
	tcc->INTFLAG.bit.OVF = 1;	//clear the interrupt
	//convert(temp1);S
}

void EIC_Handler(void){
	Port *por = PORT;
	PortGroup *porB = &(por->Group[1]);
	//porB->OUTTGL.reg = PORT_PB16;		//test frequency
	if(porB->IN.reg & PORT_PB14){
		temp2 = 1;
	}
	else{
		temp2 = 0;
	}
	EIC->INTFLAG.reg = 1<<8;	//clear the int flag
	countEICs++;
}

//void TC6_Handler(void){
//Port *port = PORT;
//PortGroup *porA = &(port->Group[0]);
//porA->DIRSET.reg = 1<<10;
//porA->OUTSET.reg = PORT_PA10;	//turn on power to sensor
//ADC->CTRLA.reg = 1<<1;	//enable ADC
//while (ADC->STATUS.bit.SYNCBUSY==1){}	//wait for enable to complete
//Tc *tc = TC6;
//TcCount8 *tcc = &tc->COUNT8;
//tcc->INTFLAG.bit.OVF = 1;	//clear the interrupt
//
//}

void waveGen(void){
	//Port *por = PORT;
	//PortGroup *porA = &(por->Group[0]);
	
	if(temp == 1){
		Tc *tc = TC4;
		TcCount8 *tcc = &tc->COUNT8;
		tcc->CTRLC.bit.INVEN1 = 1;	//clear the inversion
		while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync
		tcc->CC[1].reg = result;	//compare match
		tcc->CC[0].reg = result;	//compare match
		ADC->SWTRIG.bit.START = 1;	//start conversion
		//porA->OUTSET.reg = PORT_PA12;
		//wait(2);
		//ADC->CTRLA.reg |= 1<<1;	//re-enable ADC
		//while (ADC->STATUS.bit.SYNCBUSY==1){}	//wait for enable to complete
		//temp = 0;
	}
	
	if(temp == 2){
		Tc *tc = TC4;
		TcCount8 *tcc = &tc->COUNT8;
		tcc->CTRLC.bit.INVEN1 = 0;	//invert signal
		while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync
		tcc->CC[0].reg = 0;	//compare match
		tcc->CC[1].reg = 0;	//compare match
		ADC->SWTRIG.bit.START = 1;	//start conversion
		//porA->OUTSET.reg = PORT_PA12;
		//wait(2);
		//ADC->CTRLA.reg |= 1<<1;	//re-enable ADC
		//while (ADC->STATUS.bit.SYNCBUSY==1){}	//wait for enable to complete
		//temp = 0;
	}
	
}