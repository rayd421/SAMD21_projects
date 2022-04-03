#include <asf.h>

//Function Declarations
void wait(int t);
void simp_clock(void); 
void Debounce(void);
void write(char *text);		// Finish writing this function definition
void Power_Row(int);
char Which_Key(int, int);
void pow_readData(char *read_pnt, int count);
void disp_Data(char *arr_pnt);
void convert(int t);
void countKey(char *a);
void arithmetic(int c1, int c2, int op, int *p);
void Clear_char_array(char *b, int i);
void EICsetup(void);
void Timer4Setup(void);
void sleep(void);
void restartTimer(void);


//Global variables
volatile char Equation_array[10];
volatile char id_op = 0;
volatile char array[5];		//initialize 4-digit array
volatile char *arrpnt;					//array for key press
volatile int state=0;
volatile int cnt=0;
volatile int rowcnt=0;
volatile int j=0;
volatile int rowpow=0;	
volatile int key;
volatile int n=10;
volatile int i=0;
volatile char nxt;
volatile int key2;
volatile int slee = 0;
volatile int tim = 0;

int main (void)
{
	simp_clock();
	//EICsetup();
	//Timer4Setup();
	//
	//Tc *tc = TC4;
	//TcCount16 *tcc = &tc->COUNT16;
	//tcc->CTRLA.reg |= 1<<1;	
	//while(tcc->STATUS.bit.SYNCBUSY){}
	
	
	while(1){
	
		Debounce();
		
	}
	
}




void Debounce()
{	
		
		arrpnt = array;
		
		volatile char *equation_pnt;
		equation_pnt = Equation_array;
		
		Port *ports = PORT_INSTS;
		PortGroup *porA = &(ports->Group[0]);  //assigns pointer to group A ports
		
		porA->DIRSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;	// rows set as outputs
		porA->DIRCLR.reg = PORT_PA16 | PORT_PA17 | PORT_PA18 | PORT_PA19;  // set column of keypad as inputs
		porA -> PINCFG[16].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;		// configure port inputs for each column
		porA -> PINCFG[17].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
		porA -> PINCFG[18].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
		porA -> PINCFG[19].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
		
		Power_Row(rowcnt);								//function to power rows individually
		
		if(porA->IN.reg & PORT_PA19){					//check for column input
			key=1;
		}
		else if(porA->IN.reg & PORT_PA18){
			key= 2;
		}
		else if(porA->IN.reg & PORT_PA17){
			key= 3;
		}
		else if(porA->IN.reg & PORT_PA16){
			key= 4;
		}
		else {
			key = NULL;
		}
		switch(state) {
			
			case 0:
			if(key == NULL){					//if key is null (no key press),
				if(rowcnt==3){					//check if 4th row is powered and reset count to power first row
					rowcnt=0;					// if not, increment cnt to power next row
				}
				else{
					rowcnt++;
				}
				key2=0;
			}
			else{
				key2 = 1;
				state = 1;
			}
			break;
			
			case 1:										//counter to implement debouncing
			if(key2==1){
				cnt++;
				if(cnt>n){
					state=2;
				}
			}
			else {
				state=0;
			}
			break;
			
			case 2:									//find which key is pressed
			
			if(id_op == 3)
			{
				Clear_char_array(arrpnt, 5);
				Clear_char_array(equation_pnt, 10);
				id_op = 0;			
			}
			
			
			nxt = Which_Key(rowcnt, key);			//function returning character given row and column argument				
			Equation_array[j++] = nxt;
												
			if(id_op == 1)									//*****THIS LIMITATION IS CODE PROGRAM SPECIFIC*****, (For this program: is detecting operands and equals key press)
			{											//fill array with '----' when operand is pressed.
				Clear_char_array(arrpnt, 5);
				i=-1;
				id_op = 0;
				nxt = 0;
			}
				
			if(id_op == 2)
			{	
				Clear_char_array(arrpnt, 5);
				countKey(equation_pnt);           //Calculate function which inputs calculated number into orginal debouncing array__
				i=0;
				j=0;
				id_op = 3;
			}	
			
			else	
			{		
				if(i>=5){						
					i=0;
				}							//rewrite and display array elements if more than 4 elements

				array[i++] = nxt;			
			}
			cnt=0;
			state=3;
			break;
			
			case 3:							// to detect if new key pressed
			if(key==NULL){
				cnt++;
				if(cnt>n){
					cnt=0;
					state=0;
				}
				
			}
			else{
				cnt=0;
			}
			break;
		}		

		
		pow_readData(arrpnt, 1);			// power row and display data on ss for key press

		pow_readData(arrpnt, 2);
	
		pow_readData(arrpnt, 3);
		
		pow_readData(arrpnt, 4);
		
	
}


//Power_Row: Powers on each row of 4x4 keypad
void Power_Row(int row){						//function to power rows individually
	
	Port *ports = PORT_INSTS;
	PortGroup *porA = &(ports->Group[0]);
	PortGroup *porB = &(ports->Group[1]);
	
	porA->DIRSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;	// rows set as outputs
	porA->OUTSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;	// initilize off
	
	porB->DIRSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;  //set ss as outputs
	porB->OUTSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;  //reset display
	
	switch(row){										// switch statement to power rows individually
		
		case 0:
		porA->OUTSET.reg = PORT_PA04;				//row 4 power off
		porA->OUTCLR.reg = PORT_PA07;				//row 1 power on
		break;
		
		case 1:
		porA->OUTSET.reg = PORT_PA07;
		porA->OUTCLR.reg = PORT_PA06;
		break;
		
		case 2:
		porA->OUTSET.reg = PORT_PA06;
		porA->OUTCLR.reg = PORT_PA05;
		break;
		
		case 3:
		porA->OUTSET.reg = PORT_PA05;
		porA->OUTCLR.reg = PORT_PA04;
		break;
	}
	
}

//Which_key: returns key pressed given row and column values
char Which_Key(int row, int col){				//function which returns key pressed given row and column argument

	if(row == 0){					//check first row digits
		
		if(col == 1){
			return '1';
		}
		if(col == 2){
			return '2';
		}
		if(col == 3){
			return '3';
		}
		if(col == 4){
			 id_op = 1;
			return 'p';
		}
	}
	
	
	else if(row == 1){					//check second row digits
		
		if(col == 1){
			return '4';
		}
		if(col == 2){
			return '5';
		}
		if(col == 3){
			return '6';
		}
		if(col == 4){
			 id_op = 1;
			return 'm';
		}
	}
	
	
	else if(row == 2){					//check third row digits
		
		if(col == 1){
			return '7';
		}
		if(col == 2){
			return '8';
		}
		if(col == 3){
			return '9';
		}
		if(col == 4){
			 id_op = 1;
			return 'x';
		}
	}
	
	
	else if(row == 3){					//check fourth row digits
		
		if(col == 1){
			 id_op = 2;
			return 'e';
		}
		if(col == 2){
			return '0';
		}
		if(col == 4){
			 id_op = 1;
			return 'd';
		}
	}
}

//pow_readData: Displays specific key value stored in array from key press. (count is the specific seven segment 0-4 from right to left)
void pow_readData(char *read_pnt, int count){				//reads data from uart and uses convert function to return integer
	
	Port *ports = PORT_INSTS;
	PortGroup *porA = &(ports->Group[0]);
	porA->DIRSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;	// rows set as outputs
	porA->OUTSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;	// initialize off
	
	switch(count){										// switch statement to power rows individually
		
		case 1:
		disp_Data(read_pnt);
		porA->OUTCLR.reg = PORT_PA07;				//power on first row and show first element in read array
		wait(1);
		porA->OUTSET.reg = PORT_PA07;
		break;
		
		case 2:
		disp_Data((read_pnt+1));
		porA->OUTCLR.reg = PORT_PA06;
		wait(1);
		porA->OUTSET.reg = PORT_PA06;
		break;
		
		case 3:
		disp_Data((read_pnt+2));
		porA->OUTCLR.reg = PORT_PA05;
		wait(1);
		porA->OUTSET.reg = PORT_PA05;
		break;
		
		case 4:
		disp_Data((read_pnt+3));
		porA->OUTCLR.reg = PORT_PA04;
		wait(1);
		porA->OUTSET.reg = PORT_PA04;
		break;
	}
	
}

//disp_Data: Displays SS number given pressed key character
void disp_Data(char *arr_pnt){					//function to display the correct number on SS
	
	Port *ports = PORT_INSTS;
	PortGroup *porB = &(ports->Group[1]);
	porB->DIRSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;  //set ss as outputs
	porB->OUTSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;  //reset display

	switch(*arr_pnt){
		
		case '0':
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05;  //display 0
		break;
		
		case '1':
		porB->OUTCLR.reg = PORT_PB01 | PORT_PB02;		//display 1
		break;
		
		case '2':
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB03 | PORT_PB04 | PORT_PB06;  //display 2
		break;
		
		case '3':
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB06;		//display 3
		break;
		
		case '4':
		porB->OUTCLR.reg = PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;  // display 4
		break;
		
		case '5':
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB05 | PORT_PB06 | PORT_PB02 | PORT_PB03;		//display 5
		break;
		
		case '6':
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB05 | PORT_PB04 | PORT_PB03 | PORT_PB02 | PORT_PB06;  //display 6
		break;
		
		case '7':
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02;			// display 7
		break;
		
		case '8':
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;  //display 8
		break;
		
		case '9':
		porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;	//display 9
		break;
		
		case ' ':
		porB->OUTSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;  //clear display
		break;

		case '-':
		porB->OUTCLR.reg = PORT_PB03;  
		break;
	}
}

void countKey(char *a){
	volatile int storeArray[10];
	volatile int *ptr;
	ptr = storeArray;
	volatile int k = 0;
	volatile int operator = 0;
	volatile int count1 = 0;	//# of digits of first value
	volatile int count2 = 0;	//# of digits of second value
	while(k < 10){
		switch(*a){
			case'0':
			storeArray[k] = 0;
			a++;
			k++;
			break;
			
			case'1':
			storeArray[k] = 1;
			a++;
			k++;
			break;
			
			case'2':
			storeArray[k] = 2;
			a++;
			k++;
			break;
			
			case'3':
			storeArray[k] = 3;
			a++;
			k++;
			break;
			
			case'4':
			storeArray[k] = 4;
			a++;
			k++;
			
			case'5':
			storeArray[k] = 5;
			a++;
			k++;
			break;
			
			case'6':
			storeArray[k] = 6;
			a++;
			k++;
			break;
			
			case'7':
			storeArray[k] = 7;
			a++;
			k++;
			break;
			
			case'8':
			storeArray[k] = 8;
			a++;
			k++;
			break;
			
			case'9':
			storeArray[k] = 9;
			a++;
			k++;
			break;
			
			case'p':
			count1 = k;
			operator = 1;
			a++;
			break;
			
			case'm':
			count1 = k;
			operator = 2;
			a++;
			break;
			
			case 'x':
			count1 = k;
			operator = 3;
			a++;
			break;
			
			case'd':
			count1 = k;
			operator = 4;
			a++;
			break;
			
			case'e':
			count2 = k - count1;
			k = 10;
			break;
			
			default:
			storeArray[k] = 9;
		}
	}
	k=0;
	arithmetic(count1, count2, operator, ptr);
}


void arithmetic(int c1, int c2, int op, int *p){
	volatile	int a = 0;	//first value
	volatile	int b = 0;	//second value
	volatile	int c = 0;	//result
	
	switch(c1){	//gets first value
		case 0:
		a = 0;
		break;
		
		case 1:
		a = *p++;
		break;
		
		case 2:
		a = *p++ *10;
		a += *p++;
		break;
		
		case 3:
		a = *p++ *100;
		a += *p++ *10;
		a += *p++;
		break;
		
		case 4:
		a = *p++ *1000;
		a += *p++ *100;
		a += *p++ *10;
		a += *p++;
		break;
		
		default:
		a = 99;
	}
	
	switch(c2){	//gets second value
		case 0:
		b = 0;
		break;
		
		case 1:
		b = *p++;
		break;
		
		case 2:
		b = *p++ *10;
		b += *p++;
		break;
		
		case 3:
		b = *p++ *100;
		b += *p++ *10;
		b += *p++;
		break;
		
		case 4:
		b = *p++ *1000;
		b += *p++ *100;
		b += *p++ *10;
		b += *p++;
		break;
		
		default:
		b = 99;
	}
	
	switch(op){
		case 1:
		c = a + b;
		break;
		
		case 2:
		c = a - b;
		break;
		
		case 3:
		c = a * b;
		break;
		
		case 4:
		c = a / b;
		break;
		
		default:
		a = 99;
	}
	convert(c);
}

void convert(int t){
	volatile int a = t;
	volatile int i = 1000;   //divisor
	volatile int j = 0;  //array counter
	volatile int k = 9;
	volatile int p = 0;
	volatile int m = 1;  //counter
	volatile int n = 1000;    //increment to divisor

	while(j <= 3){
		volatile int b = a % i;
		if(b == a) {
			int p = (m-1);
			switch(p) {
				case 0:
				array[j++] = '0';
				break;
				case 1:
				array[j++] = '1';
				break;
				case 2:
				array[j++] = '2';
				break;
				case 3:
				array[j++] = '3';
				break;
				case 4:
				array[j++] = '4';
				break;
				case 5:
				array[j++] = '5';
				break;
				case 6:
				array[j++] = '6';
				break;
				case 7:
				array[j++] = '7';
				break;
				case 8:
				array[j++] = '8';
				break;
				case 9:
				array[j++] = '9';
				break;
				default:
				array[j++] = 'G';
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


void simp_clock(){

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
	GCLK->GENDIV.reg = 1<<1 | 1<<8;
							//GENERATOR 1 SELECTION
							//1 division bit set -> divide source by 1 p.114
							
			   
	GCLK->GENCTRL.reg = 1<<1 | GCLK_GENCTRL_RUNSTDBY | 0<<20 | GCLK_GENCTRL_OE | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSC8M; 
							//Generic clock generator 1 ID selected
							//run in standby 
							//0<<20 = divide Selection is generator source divided by GENDIV.DIV     
							//Output enable   
							//enable gen Clk ctr 
							//8MHz clock source		
							
									
					
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_EIC | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK1;
							//generic clock configured for EIC
							//generic clock enable
							//generic clock generator 1 selected

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK1;
							//generic clock configured for ADC
							//generic clock enable
							//generic clock generator 1 selected	

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_SERCOM4_CORE | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK1;
							//generic clock configured for Sercom4
							//generic clock enable
							//generic clock generator 1 selected	
	
	while(GCLK_STATUS_SYNCBUSY==1){}
							//wait for synchronization of clocks to complete	
	
	
//Setting up the Power Manager (CPU and clock buses AHB AND APB)
	PM->SLEEP.reg = PM_SLEEP_IDLE_APB;  
	//CPU, AHB, AND APB Clock domains stopped until called
	
	PM->CPUSEL.reg = PM_CPUSEL_CPUDIV_DIV1;
	//PRESCALER : DIVIDES THE MAIN CLOCK SOURCE BY 1, THIS IS CLOCK THAT RUNS CPU
	PM->APBASEL.reg = PM_APBASEL_APBADIV_DIV1;
	//PRESCALER: DIVIDES THE  MAIN CLOCK BY 1, THIS CLOCK RUNS THE APBA TO PERIPHERAL
	PM->APBBSEL.reg = PM_APBBSEL_APBBDIV_DIV16;
	PM->APBCSEL.reg = PM_APBCSEL_APBCDIV_DIV1;
	
	PM->APBAMASK.reg = PM_APBAMASK_EIC;  //Enable EIC clock
	PM->APBBMASK.reg = PM_APBBMASK_PORT; //Enable Port io clock
	PM->APBCMASK.reg = PM_APBCMASK_SERCOM4; //Enable Sercom 4

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
	 
// Function to clear character array with pointer to array and i=number of elements to be cleared	
void Clear_char_array(char *b, int i)
{
	for (int a = 0; a < i; a++)
	{
		b[a] = 0;
	}
}


void restartTimer(void){
	tim = 1;
}


void sleep(void){
	slee = 1;
}

void EIC_Handler(void){
	slee = 0;
	EIC->INTFLAG.reg = 1<<2;	//clear the int flag
	Port *port = PORT;
	PortGroup *porA = &(port->Group[0]);
	porA->PINCFG[18].bit.PMUXEN =0;	//disable the MUX
	restartTimer();
}

void TC4_Handler(void){
	Tc *tc = TC4;
	TcCount16 *tcc = &tc->COUNT16;
	tcc->INTFLAG.bit.OVF = 1;	//clear the interrupt flag
	tcc->INTENCLR.bit.OVF = 1;	//clear the interrupt
	sleep();	//correct way to do it
}

void EICsetup(void){
	//might need to disable EIC first
	EIC->CTRL.bit.SWRST = 1;	//reset the EIC and disables the EIC
	while(EIC->STATUS.bit.SYNCBUSY){}	//wait for synchronization
	EIC->INTENSET.reg = 2<<1;	//enables PA18 for external interrupt (EXT2)
	EIC->CONFIG[0].bit.SENSE2 = 1;	//select rising edge as triggering ...config[n]..(n*8+x)=y...EXT[y]....SENSEx...n=0|1
	EIC->WAKEUP.reg = 2<<1;	//allows wakeup
	EIC->CTRL.reg |= 1<<1;	//enable the EIC at the end
	while(EIC->STATUS.bit.SYNCBUSY){}	//wait for synchronization
	NVIC->ISER[0] |= 1<<4;	//enable the NVIC handler
}


void Timer4Setup(void){
	Tc *tc = TC4;
	TcCount16 *tcc = &tc->COUNT16;
	tcc->CTRLA.reg = 0;	//disable the TC4
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
	tcc->CTRLA.bit.PRESCALER = 2;	//divide by 64;
	tcc->CTRLA.bit.WAVEGEN = 0;	//normal frequency
	tcc->CTRLA.bit.MODE = 0;	//16 bit mode
	tcc->CTRLBSET.bit.ONESHOT = 1;	//turn on one shot mode
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync to complete
	tcc->INTENSET.bit.OVF = 1;	//enable the overflow interrupt
	while(tcc->STATUS.bit.SYNCBUSY){}	//wait for sync to complete
	NVIC->ISER[0] |= 1<<19;	//enable the NVIC handler for TC4

}