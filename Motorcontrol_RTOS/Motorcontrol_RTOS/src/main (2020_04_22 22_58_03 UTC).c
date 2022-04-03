#include <asf.h>
#include <FreeRTOS.h>

#define pdMS_TO_TICKS( xTimeInMs ) ( ( TickType_t ) ( ( ( TickType_t ) ( xTimeInMs ) * ( TickType_t ) configTICK_RATE_HZ ) / ( TickType_t ) 1000 ) )


////RTOS Prototypes////
void vKeypad_Display_Task(void); 
void vPID_Control_Task(void);
void vCountEIC(void);

SemaphoreHandle_t xMutex = NULL;
//SemaphoreHandle_t xEICsemaphore;

////C Prototypes////
void clockSetup(void);
void wait(volatile int d);
void portSetup(void);
void Timer4Setup(void);
void poll(void);
void charConvert(char *a);
void Timer4Setup(void);
void Timer5Setup(void);
void Timer6Setup(void);
void compare(int a);
void EICsetup(void);
void convert(int t);
void display(char *a);


////Global Variables////
volatile int mode = 2;
volatile int keypad;
volatile int comPres;
volatile int temp2 = 0;	//for direction of encoder
volatile static int countEICs = 0;
volatile int rawSpeed;
volatile char ConArray[4];
volatile char *ConArrayPtr;
volatile int errorSpd;
volatile static int errorSpd1;
volatile float errorPos;
volatile static float errorPos1;
volatile int errorDisplay;
volatile static float integral;
volatile static float integralx;
volatile float Diff;
volatile float Diffx;
volatile float y;
volatile float x;
volatile int x_int;
volatile int normY;
volatile float temp;
volatile int temp1;
volatile float filterY = 0;
volatile float filterY1 = 0;
volatile float filterX1 = 0;
volatile int filterRawSpeed = 0;
volatile int i = 0;
volatile float key_position = 0;
volatile int ent_key=1; 
volatile char *keyptr2;
volatile char array2[4] = "0000";//write array




////RPM PID control Coefficients////
volatile const float Ts = .005;
volatile const float Kps = .001;
volatile const float Kis = 1.85;
volatile const float Kffs = 0;
volatile const float Kds = .0011;

////Position PID control Coefficients////
volatile const float Kpp = .09;		//.09
volatile const float Kip = .065;	//.001
volatile const float Kffp = .00;
volatile const float Kdp = .00;


int main (void){
	
	system_init();
	clockSetup();
	portSetup();
	Timer4Setup();
	EICsetup();

	ConArrayPtr = ConArray;
	keyptr2 = array2;

	NVIC->IP[1] = 0x0;	//EIC highest priority
//	NVIC->IP[5] = 0x40C0;	//TC5 lowest, TC6 mid priority

	xMutex = xSemaphoreCreateMutex();
	//xEICsemaphore = xSemaphoreCreateBinary();
	
 	if(xMutex !=NULL)
 	{
	
	xTaskCreate(vPID_Control_Task, "PID Control Task", 500, NULL, 2, NULL);
	xTaskCreate(vKeypad_Display_Task, "Keypad & Display Task", 500, NULL, 1, NULL);
	//xTaskCreate(vCountEIC, "EIC Counting Task", 500, NULL, 3, NULL);
	
	vTaskStartScheduler();
	
	}
	
	for( ;; );
	
}




// void vCountEIC(void){
// 	
// 	Port *por = PORT;
// 	PortGroup *porB = &(por->Group[1]);
// 	// 	porB->DIRSET.reg = PORT_PB16;
// 	// 	porB->OUTTGL.reg = PORT_PB16;		//test frequency
// 	
// 	for( ;; ){
// 		
// 		xSemaphoreTake(xEICsemaphore, portMAX_DELAY);
// 		
// 			if(porB->IN.reg & PORT_PB14){	//CCW
// 				//temp2 = 1;
// 				countEICs++;
// 			}
// 			else if(!(porB->IN.reg & PORT_PB14)){	//CW
// 				//temp2 = 0;
// 				countEICs--;
// 			}
// 		
// 	}
// }


void vKeypad_Display_Task(void){	
			
	Port *por = PORT;
	PortGroup *porA = &(por->Group[0]);
	PortGroup *porB = &(por->Group[1]);

	
	for( ;; ){
		
		
		vTaskDelay(pdMS_TO_TICKS(1000/60));		//Block to run Task at 60Hz

		xSemaphoreTake(xMutex, 0);     //Take semaphore, do not block/wait.
			
		
		poll();		//read keypad

		if(ent_key==0){
			porA->OUTCLR.reg = 	PORT_PA07;
			display(keyptr2);
			wait(1);
			porA->OUTSET.reg = 	PORT_PA07;

			porA->OUTCLR.reg = 	PORT_PA06;
			display(keyptr2 + 1);
			wait(1);
			porA->OUTSET.reg = 	PORT_PA06;
	
			porA->OUTCLR.reg = 	PORT_PA05;
			display(keyptr2 + 2);
			wait(1);
			porA->OUTSET.reg = 	PORT_PA05;
	
			porA->OUTCLR.reg = 	PORT_PA04;
			display(keyptr2 + 3);
			wait(1);
			porA->OUTSET.reg = 	PORT_PA04;
		}
	
		else if(ent_key==1){
		
			if(mode == 0){
					convert(filterRawSpeed);
			}
	
			else{
					convert(errorDisplay);
			}
	
			porA->OUTCLR.reg = 	PORT_PA07;
			display(ConArrayPtr);
			wait(1);
			porA->OUTSET.reg = 	PORT_PA07;

			porA->OUTCLR.reg = 	PORT_PA06;
			display(ConArrayPtr + 1);
			wait(1);
			porA->OUTSET.reg = 	PORT_PA06;
	
			porA->OUTCLR.reg = 	PORT_PA05;
			display(ConArrayPtr + 2);
			wait(1);
			porA->OUTSET.reg = 	PORT_PA05;
	
			porA->OUTCLR.reg = 	PORT_PA04;
			display(ConArrayPtr + 3);
			wait(1);
			porA->OUTSET.reg = 	PORT_PA04;
		}

	xSemaphoreGive(xMutex);			//Give back semaphore after complete 
	
	}
}


void vPID_Control_Task(void){

	Tc *tcc = TC4;
	TcCount8 *tc4 = &tcc->COUNT8;
			
	for( ;; ){
		
	vTaskDelay(pdMS_TO_TICKS(1000/200));		//Delay to run Task at 200Hz
		
	xSemaphoreTake(xMutex, 0);		//Take semaphore when available. If not available, imedietly return (0 indicates no block/waiting)
		
		rawSpeed = countEICs * (60*200/400);// raw RPM calculation
	
		////digital filter//////
		filterY = (0.9691*filterY1) + (0.03093*filterX1);
		filterY1 = filterY;
		filterX1 = rawSpeed;
		filterRawSpeed = filterY;
	

		switch(mode){
		
			case 0:	//spd ctrl
				i = 0;
		
				errorSpd = keypad - rawSpeed;
				integral = integral + (Ts * errorSpd);
				Diff = ((errorSpd - errorSpd1) / Ts);
				y = (Kps * errorSpd) + (Kis * integral) + (Kds * Diff) + (Kffs * keypad);
				errorSpd1 = errorSpd;
		
				if(y > 5000){
					y = 5000;
				}
		
				if(y < 0){
					y = 0;
				}
		
				normY = (((y * 128)/5000)+127);
				tc4->CC[0].reg = (normY);
				tc4->CC[1].reg = 255-normY;
				while(tc4->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
		
				countEICs = 0;
				break;	
		
		
			case 1: //pos ctrl
				key_position = keypad*400/360; //normalize keypad angle input
				if(i == 0){
					countEICs = 0;
				}
		
				errorPos = countEICs-(int)key_position;
		
				temp = ((float)countEICs / 400);
		
				///To handle full rotations//
				if(temp < 0){
					errorDisplay = (countEICs*360)/400 * -1;
				}
		
				else if(temp != 0){
					errorDisplay = (countEICs*360)/400;
				}
		
				integralx = integralx + (Ts * errorPos);
				Diffx = ((errorPos - errorPos1) / Ts);
		
				x = (Kpp * errorPos) + (Kip * integralx) + (Kdp * Diffx);
				x_int = x;
				errorPos1 = errorPos;
		
				tc4->CC[0].reg = 255-(x_int+127);
				tc4->CC[1].reg = (x_int+127);
				while(tc4->STATUS.bit.SYNCBUSY){}
		
				i++;
				break;
		
				default:
				tc4->CC[0].reg = 255-127;
				tc4->CC[1].reg = 127;
				while(tc4->STATUS.bit.SYNCBUSY){}
				break;
		}
		
		xSemaphoreGive(xMutex);			//Give back semaphore after complete
		}
	
}

//// vTaskDelayUntil to run Keypad and Display////
/*void vKeypad_Display_Task(void){			
	Port *por = PORT;
	PortGroup *porA = &(por->Group[0]);
	PortGroup *porB = &(por->Group[1]);
	TickType_t xLastWakeTime;
	
	
	xLastWakeTime = xTaskGetTickCount();	//Used as vtaskdelayuntil starting point for precise frequency control

	for( ;; ){
		
		//porB->DIRSET.reg = PORT_PB16;
		//porB->OUTTGL.reg = PORT_PB16;		//test frequency
		
		poll();		//read keypad

		if(mode == 0){
			convert(filterRawSpeed);
		}
	
		else{
			convert(errorDisplay);
		}
	
		porA->OUTCLR.reg = 	PORT_PA07;
		display(ConArrayPtr);
		wait(1);
		porA->OUTSET.reg = 	PORT_PA07;

		porA->OUTCLR.reg = 	PORT_PA06;
		display(ConArrayPtr + 1);
		wait(1);
		porA->OUTSET.reg = 	PORT_PA06;
		
		porA->OUTCLR.reg = 	PORT_PA05;
		display(ConArrayPtr + 2);
		wait(1);
		porA->OUTSET.reg = 	PORT_PA05;
		
		porA->OUTCLR.reg = 	PORT_PA04;
		display(ConArrayPtr + 3);
		wait(1);
		porA->OUTSET.reg = 	PORT_PA04;
		
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000/200) );	//Delay to run at (1000ms/300cycle) -> 200Hz
	}
}
*/

//// vTaskDelayUntil Task code to run PID control////
/*void vPID_Control_Task(void){

	Tc *tcc = TC4;
	TcCount8 *tc4 = &tcc->COUNT8;
	TickType_t xLastWakeTime;		
	
	
	xLastWakeTime = xTaskGetTickCount();	//Used as vtaskdelayuntil starting point for precise frequency control
	for( ;; ){
		
		rawSpeed = countEICs * (60*200/400);// raw RPM calculation
	
		////digital filter//////
		filterY = (0.9691*filterY1) + (0.03093*filterX1);
		filterY1 = filterY;
		filterX1 = rawSpeed;
		filterRawSpeed = filterY;
	

		switch(mode){
		
			case 0:	//spd ctrl
				i = 0;
		
				errorSpd = keypad - rawSpeed;
				integral = integral + (Ts * errorSpd);
				Diff = ((errorSpd - errorSpd1) / Ts);
				y = (Kps * errorSpd) + (Kis * integral) + (Kds * Diff) + (Kffs * keypad);
				errorSpd1 = errorSpd;
		
				if(y > 5000){
					y = 5000;
				}
		
				if(y < 0){
					y = 0;
				}
		
				normY = (((y * 128)/5000)+127);
				tc4->CC[0].reg = (normY);
				tc4->CC[1].reg = 255-normY;
				while(tc4->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
		
				countEICs = 0;
				break;	
		
		
			case 1: //pos ctrl
				key_position = keypad*400/360; //normalize keypad angle input
				if(i == 0){
					countEICs = 0;
				}
		
				errorPos = countEICs-(int)key_position;
		
				temp = ((float)countEICs / 400);
		
				///To handle full rotations//
				if(temp < 0){
					errorDisplay = (countEICs*360)/400 * -1;
				}
		
				else if(temp != 0){
					errorDisplay = (countEICs*360)/400;
				}
		
				integralx = integralx + (Ts * errorPos);
				Diffx = ((errorPos - errorPos1) / Ts);
		
				x = (Kpp * errorPos) + (Kip * integralx) + (Kdp * Diffx);
				x_int = x;
				errorPos1 = errorPos;
		
				tc4->CC[0].reg = 255-(x_int+127);
				tc4->CC[1].reg = (x_int+127);
				while(tc4->STATUS.bit.SYNCBUSY){}
		
				i++;
				break;
		
				default:
				tc4->CC[0].reg = 255-127;
				tc4->CC[1].reg = 127;
				while(tc4->STATUS.bit.SYNCBUSY){}
				break;
		}
		
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000/60) );	//Delay to run at (1000ms/60cycle)-> 60Hz
	}
}
*/


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

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC | GCLK_CLKCTRL_GEN_GCLK1 | 1<<14;	//setup genclk for ADC
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TC4_TC5 | GCLK_CLKCTRL_GEN_GCLK0 | 1<<14;	//setup genCLK for TC4/TC5
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_EIC | GCLK_CLKCTRL_GEN_GCLK0 | 1<<14;	//setup genclk for EIC...this is a single write method i believe
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_TC6_TC7 | GCLK_CLKCTRL_GEN_GCLK0 | 1<<14;	//setup genCLK for TC6/TC7

	while (GCLK->STATUS.bit.SYNCBUSY==1){}	//waiting for sync to complete
	
	PM->CPUSEL.bit.CPUDIV = 0;	//divide CPU clock by 1	pg 15
	PM->APBASEL.bit.APBADIV = 0;	//divide apba bus by 1
	PM->APBBSEL.bit.APBBDIV = 0;	//divide apbb bus by 1
	PM->APBCSEL.bit.APBCDIV = 0;	//divide apbc bus by 1
	PM->APBAMASK.reg |= 1<<3;	//enable the GCLK clock DONT THINK NECESSARY they should be enabled by default pg 159
	PM->APBCMASK.reg |= 1<<16;	//enable the ADC APB
	PM->APBAMASK.reg |= 1<<6;	//enable the EIC APB
	PM->APBCMASK.reg |= PM_APBCMASK_TC4;	//enable the TC4 APB
	PM->APBCMASK.reg |= PM_APBCMASK_TC5;	//enable the TC5 APB
	PM->APBCMASK.reg |= PM_APBCMASK_TC6;	//enable the TC6 APB

}

void portSetup(void){
	Port *por = PORT;
	PortGroup *porA = &(por->Group[0]);
	PortGroup *porB = &(por->Group[1]);
	porA -> DIRSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;//outputs
	porB -> DIRSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06 | PORT_PB07;//LED segments
	porA -> DIRCLR.reg = PORT_PA16 | PORT_PA17 | PORT_PA18 | PORT_PA19;//inputs keypad
	porA -> PINCFG[16].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA -> PINCFG[17].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA -> PINCFG[18].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA -> PINCFG[19].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA -> OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;	//turn on SSDs
	porB->DIRSET.reg = PORT_PB09;	//sign LED
	porB->PMUX[6].bit.PMUXE = 4;	//mux the TC wavegen PB12	PWM output
	porB->PINCFG[12].bit.PMUXEN = 1;	//enable the mux
	porB->PMUX[6].bit.PMUXO = 4;	//mux the TC wavegen PB13	PWM output
	porB->PINCFG[13].bit.PMUXEN = 1;	//enable the mux
	porA->PMUX[14].bit.PMUXO = 0;	//mux the EIC to pin PA28 EXTINT[8] for encoder
	porA->PINCFG[28].bit.PMUXEN =1;	//enable the MUX
	porB->DIRCLR.reg = 1<<14;	//set as input for channel B for encoder
	porB->PINCFG[14].bit.INEN =1;	//enable the MUX
}

void wait(volatile int d){
	int count = 0;
	while (count < d*1000){
		count++;
	}
}

void poll(void){
	Port *por = PORT;
	PortGroup *porA = &(por->Group[0]);
	PortGroup *porB = &(por->Group[1]);
	
	static volatile int i=0;//counter
	static volatile int x=0; //state (state machine)
	static volatile  int CNT=0;  //counter (state machine)
	static volatile  char T1;	//(state machine)
	static volatile  int N=1;	//delay (state machine)
	static volatile char key='9';//NULL char key (state machine)
	static volatile char array[4] = "0000";//write array
	static volatile char *keyptr;//write array pointer


	keyptr = array;

	key='D';
	///////////check first row////////////////
	porA -> OUTCLR.reg = PORT_PA07;//turn on left SSD/top row of keypad
	porA -> OUTSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06;//turn off other SSD/top row of keypad
	//display(arrPtr);

	
	if(porA->IN.reg&PORT_PA19){//checking PA19 for input
		key='1';
	}
	if(porA->IN.reg&PORT_PA18){//checking PA18 for input
		key='2';
	}
	if(porA->IN.reg&PORT_PA17){//checking PA17 for input
		key='3';
	}
	if(porA->IN.reg&PORT_PA16)//checking PA16 for input 'A'
	{
		key='A';
		ent_key=1;
	}
	//	wait(1);//this wait is here because the LEDs are on/off so fast that i cannot even see them
	porA -> OUTSET.reg = PORT_PA07;//turn off left SSD/top row of keypad
	wait(1);//this wait is so the numbers don't get mixed up


	////////////check second row////////////////
	porA -> OUTCLR.reg = PORT_PA06;//turn on left SSD/top row of keypad
	//display((arrPtr+1));


	if(porA->IN.reg&PORT_PA19)//checking PA19 for input
	key='4';
	
	if(porA->IN.reg&PORT_PA18)//checking PA18 for input
	key='5';
	
	if(porA->IN.reg&PORT_PA17)//checking PA17 for input
	key='6';
	
	
	//	wait(1);//this wait is here because the LEDs are on/off so fast that i cannot even see them
	porA -> OUTSET.reg = PORT_PA06;//turn off left SSD/top row of keypad
	wait(1);//this wait is so the numbers don't get mixed up



	////////////check third row////////////////
	porA -> OUTCLR.reg = PORT_PA05;//turn on left SSD/top row of keypad
	//display((arrPtr+2));


	if(porA->IN.reg&PORT_PA19)//checking PA19 for input
	key='7';
	
	if(porA->IN.reg&PORT_PA18)//checking PA18 for input
	key='8';
	
	if(porA->IN.reg&PORT_PA17)//checking PA17 for input
	key='9';
	
	
	//	wait(1);//this wait is here because the LEDs are on/off so fast that i cannot even see them
	porA -> OUTSET.reg = PORT_PA05;//turn off left SSD/top row of keypad
	wait(1);//this wait is so the numbers don't get mixed up


	////////////check fourth row////////////////
	porA -> OUTCLR.reg = PORT_PA04;//turn on left SSD/top row of keypad
	//display((arrPtr+3));

	if(porA->IN.reg&PORT_PA17){//checking PA17 for input '#'
		key='#';
		ent_key=0;
	}
	if(porA->IN.reg&PORT_PA18){//checking PA18 for input '0'
		key='0';
	}
	if(porA->IN.reg&PORT_PA19){//checking PA19 for input '*'
		key = '*';
		ent_key=0;
	}
	
	//	wait(1);//this wait is here because the LEDs are on/off so fast that i cannot even see them
	porA -> OUTSET.reg = PORT_PA04;//turn off left SSD/top row of keypad
	wait(1);//this wait is so the numbers don't get mixed up
	
	switch(x){
		case 0:				//state 0 checks for key pressed
		if(key=='D'){
		break;}
		T1=key;
		CNT=0;
		x=1;
		break;
		
		case 1:				//state 1 denouncing initial press
		if(key!=T1){
			x=0;
		break;}
		CNT++;
		if(CNT>N){	//pressed long enough
			x=2;
		break;}
		break;
		
		case 2:				//state 2 process input
		if(key!='D'){
			CNT=0;
			if(key == '#'){	//if spd key is detected reset array set mode
				array[0] = '0';
				array[1] = '0';
				array[2] = '0';
				array[3] = '0';
				i = 0;
				porB -> OUTSET.reg = PORT_PB09;	//turn off LED
				mode = 0;	//set to speed ctrl
			}
			else if(key == '*'){	//if pos key is detected reset array set mode
				array[0] = '0';
				array[1] = '0';
				array[2] = '0';
				array[3] = '0';
				i = 0;
				porB -> OUTCLR.reg = PORT_PB09;	//turn on LED
				mode = 1;	//set to pos ctrl
			}
			else if( key == 'A'){	//'A' is the enter key
				charConvert(keyptr);
				array[0] = '0';
				array[1] = '0';
				array[2] = '0';
				array[3] = '0';
				i = 0;
			}
			else if(key != 'B' | 'C'){
				array[i]=key;	//reset array to loop back on itself
				array2[i++]=key;
				if(i>=4){
					i=0;
				}
			}
		}
		CNT=0;
		x=3;
		break;
		
		case 3:				//state 3 denouncing release
		if(key == 'D'){
			CNT++;
			if(CNT>N){	//released long enough then reset
				CNT=0;
				x=0;
			}
			break;
		}
		CNT = 0;
		break;
		
		default:
		x=0;
		break;
	}
}

void charConvert(char *a){	//gives value of keypad for spd/pos ctrl
	
	keypad = ((*a-48) * 1000);
	a++;
	keypad += ((*a-48) * 100);
	a++;
	keypad += ((*a-48) * 10);
	a++;
	keypad += ((*a-48) * 1);
	if(keypad > 5000){	//normalize it should throw an error
		keypad = 5000;
		//display(err1);
	}
	//display(a);
}

void EICsetup(void){
	EIC->CTRL.bit.SWRST = 1;	//reset the EIC and disables the EIC
	while(EIC->STATUS.bit.SYNCBUSY){}	//wait for synchronization
	EIC->INTENSET.reg = 1<<8;	//enables PA28 for external interrupt EXTINT[8]
	EIC->CONFIG[1].bit.SENSE0 = 2;	//select falling edge as triggering ...config[n]..(n*8+x)=y...EXT[y]....SENSEx...n=0|1
	EIC->CTRL.reg |= 1<<1;	//enable the EIC at the end
	NVIC->ISER[0] = 1<<4;	//enable the NVIC handler
}

void Timer4Setup(void){	//for wavegen only should be in timer int 2
	Tc *tcc = TC4;
	TcCount8 *tc4 = &tcc->COUNT8;
	tc4->CTRLA.reg = 0;	//disable the TC4
	while(tc4->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
	tc4->CTRLA.bit.PRESCALER = 6;	//divide by 1;
	tc4->CTRLA.bit.WAVEGEN = 2;	//normal PWM frequency per=period, CC1/CC0=compare value
	tc4->CTRLA.bit.MODE = 1;	//8 bit mode
	tc4->PER.reg = 0xff;
	tc4->CTRLBSET.bit.ONESHOT = 0;	//turn off one shot mode
	while(tc4->STATUS.bit.SYNCBUSY){}	//wait for sync to complete
	while(tc4->STATUS.bit.SYNCBUSY){}	//wait for sync to complete
	tc4->CTRLA.reg |= 1<<1;	//enable the TC4
	while(tc4->STATUS.bit.SYNCBUSY){}	//wait for sync of enable
	tc4->CC[0].reg = 0;
	while(tc4->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
	tc4->CC[1].reg = 0;
	while(tc4->STATUS.bit.SYNCBUSY){}	//wait for sync of disable
	tc4->CTRLC.bit.INVEN1 = 0;
	while(tc4->STATUS.bit.SYNCBUSY){}	//wait for sync
}


void EIC_Handler(void){
	
	
	Port *por = PORT;
	PortGroup *porB = &(por->Group[1]);
	 	// 	porB->DIRSET.reg = PORT_PB16;
	 	// 	porB->OUTTGL.reg = PORT_PB16;		//test frequency
	
	 			if(porB->IN.reg & PORT_PB14){	//CCW
	 				//temp2 = 1;
	 				countEICs++;
	 			}
	 			else if(!(porB->IN.reg & PORT_PB14)){	//CW
	 				//temp2 = 0;
	 				countEICs--;
	 			}
	EIC->INTFLAG.reg = 1<<8;	//clear the int flag
	}
	
// 	BaseType_t xHigherPriorityTaskWoken; 
// 			
// 	xHigherPriorityTaskWoken = pdFALSE;				//initialize the state of 
// 	
// 	xSemaphoreGiveFromISR(xEICsemaphore, &xHigherPriorityTaskWoken);
// 	
// 	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
// 	
// 	EIC->INTFLAG.reg = 1<<8;	//clear the int flag


void convert(int t){
	volatile int a = t;
	int i = 1000;   //divisor
	int j = 0;  //array counter
	//int a = 4312;   // our value
	//volatile char arrADC[4];
	//volatile char *arrPtr;
	//arrPtr = arrADC;
	int m = 1;  //counter
	int n = 1000;    //increment to divisor

	while(j <= 3){
		int b = a % i;
		if(b == a) {
			int p = (m-1);
			switch(p) {
				case 0:
				ConArray[j++] = '0';
				break;
				case 1:
				ConArray[j++] = '1';
				break;
				case 2:
				ConArray[j++] = '2';
				break;
				case 3:
				ConArray[j++] = '3';
				break;
				case 4:
				ConArray[j++] = '4';
				break;
				case 5:
				ConArray[j++] = '5';
				break;
				case 6:
				ConArray[j++] = '6';
				break;
				case 7:
				ConArray[j++] = '7';
				break;
				case 8:
				ConArray[j++] = '8';
				break;
				case 9:
				ConArray[j++] = '9';
				break;
				default:
				ConArray[j++] = 'A';
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

void display(char *a){
	int i = 0;
	Port *ports = PORT_INSTS;
	PortGroup *porA = &(ports->Group[0]);
	PortGroup *porB = &(ports->Group[1]);
	
	switch(*a){
		
		case '0':
		porB -> OUTSET.reg = PORT_PB07 | PORT_PB06;//display "0"
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05;//display "0"
		//a++;
		break;
		
		case '1':
		porB -> OUTSET.reg = PORT_PB00 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06 | PORT_PB07;//display "1"
		porB -> OUTCLR.reg = PORT_PB01 | PORT_PB02;//display "1"
		//a++;
		break;
		
		case '2':
		porB -> OUTSET.reg =  PORT_PB02 | PORT_PB05 | PORT_PB07;//display "2"
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB03 | PORT_PB04 | PORT_PB06;//display "2"
		//a++;
		break;
		
		case '3':
		porB -> OUTSET.reg = PORT_PB04 | PORT_PB05 | PORT_PB07;//display "3"
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB06;//display "3"
		//a++;
		break;
		
		case '4':
		porB -> OUTSET.reg = PORT_PB00 | PORT_PB03 | PORT_PB04 | PORT_PB07;//display "4"
		porB -> OUTCLR.reg = PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;//display "4"
		//a++;
		break;
		
		case '5':
		porB -> OUTSET.reg = PORT_PB01 | PORT_PB04 | PORT_PB07;//display "5"
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB03 | PORT_PB05 | PORT_PB06;//display "5"
		//a++;
		break;
		
		case '6':
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;//display "6"
		porB -> OUTSET.reg = PORT_PB01 | PORT_PB07;//display "6"
		//a++;
		break;
		
		case '7':
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02;//display "7"
		porB -> OUTSET.reg = PORT_PB03 | PORT_PB04| PORT_PB05 | PORT_PB06 | PORT_PB07;//display "7"
		//a++;
		break;
		
		case '8':
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04| PORT_PB05 | PORT_PB06;//display "8"
		porB -> OUTSET.reg = PORT_PB07;//display "8"
		//a++;
		break;
		
		case '9':
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;//display "9"
		porB -> OUTSET.reg = PORT_PB03 | PORT_PB04 | PORT_PB07;//display "9"
		//a++;
		break;
		
		default:
		porB -> OUTSET.reg = PORT_PB03 | PORT_PB07;//display "A"
		porB -> OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB04 | PORT_PB05 | PORT_PB06;//display "A"
		break;
	}
}