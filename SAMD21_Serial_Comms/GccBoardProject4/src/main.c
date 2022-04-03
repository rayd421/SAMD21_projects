#include <asf.h>

void Simple_Clk_Init(void);
void Power_Clk_Init(void);
void PortInit(void);		// Enable Peripheral Multiplexing (PMUX) for SERCOM4 at PA10/11
void UartInit(void);		// Finish setting the initialization values
void write(char *text);		// Finish writing this function definition

volatile int state =0;
volatile char key[300]={0};
volatile int CNT = 0;
volatile int A = 0;
volatile int B = 0;
volatile int RNG = 0;
volatile char test[300]={0};
volatile int C = 0;

int main(void)
{
    
    Simple_Clk_Init();
    Power_Clk_Init();
    PortInit();
    UartInit();      // set up how we going to attach the bin
    
    write("Press any KEY to start the game \n"); // Test the serial connection on startup
    Port *ports = PORT_INSTS;
    PortGroup *portAs = &(ports->Group[0]);
    PortGroup *portBs = &(ports->Group[1]);
    portAs -> DIRSET.reg = PORT_PA07|PORT_PA06|PORT_PA05|PORT_PA04;
    portAs -> DIRCLR.reg = PORT_PA19|PORT_PA18|PORT_PA17|PORT_PA16;
    portAs -> OUTCLR.reg = PORT_PA19|PORT_PA18|PORT_PA17|PORT_PA16;
    portAs -> OUTCLR.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
    portAs -> PINCFG[19].reg = PORT_PINCFG_INEN|PORT_PINCFG_PULLEN;
    portAs -> PINCFG[18].reg = PORT_PINCFG_INEN|PORT_PINCFG_PULLEN;
    portAs -> PINCFG[17].reg = PORT_PINCFG_INEN|PORT_PINCFG_PULLEN;
    portAs -> PINCFG[16].reg = PORT_PINCFG_INEN|PORT_PINCFG_PULLEN;
 

						while (1)
						{
	
								B++;
								portAs -> OUTCLR.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
								if (portAs->IN.reg & ( PORT_PA19|PORT_PA18|PORT_PA17|PORT_PA16 ) )
									{		
										while (C<1000) 
										{C++;}			
										break;
									}
						}
							B = B%100;
 							test[0] = B;
							write(test);
							write("game start!! \n");
	  
	 
	
				while(1)
				{
					switch(state)
					{
						case 0:
						 portAs -> OUTCLR.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
							if (portAs->IN.reg & ( PORT_PA19|PORT_PA18|PORT_PA17|PORT_PA16 ) )
							{
					
								state = 1;
							}
							break;
                
						case 1:
                
							if  (portAs->IN.reg & (PORT_PA19|PORT_PA18|PORT_PA17|PORT_PA16 ) )
							{
								CNT ++;
							}
							else
							{
								CNT=0;
							}
                
							if (CNT <60000)
							{
								state =0;
							}
                
							else
							{
								CNT =0;
								state = 2;
							}
							 portAs -> OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
							break;
                
						case 2:
							portAs -> OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
							portAs -> OUTCLR.reg = PORT_PA07;
							if (portAs->IN.reg & PORT_PA19 ) //1
							{
					
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg = PORT_PA04|PORT_PA03| PORT_PA05| PORT_PA06| PORT_PA07|PORT_PA00;
								portBs -> OUTCLR.reg = PORT_PB01 | PORT_PB02;
					
								key[A] = '1';
								A ++;
								state = 0;
							}
							if (portAs->IN.reg & PORT_PA18 ) //2
							{
					
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg =PORT_PA05| PORT_PA02| PORT_PA07;
								portBs -> OUTCLR.reg = PORT_PB00 | PORT_PB01| PORT_PB06| PORT_PB04| PORT_PB03;
					
					
								key[A] = '2';
								A ++;
								state = 0;
							}
			
							if (portAs->IN.reg & PORT_PA17 ) //3
							{
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg =PORT_PA05| PORT_PA04| PORT_PA07;
								portBs -> OUTCLR.reg = PORT_PB00 | PORT_PB01| PORT_PB06| PORT_PB02| PORT_PB03;
								key[A] = '3';
								A ++;
								state = 0;
							}
							if (portAs->IN.reg & PORT_PA16 ) //A
							{
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg = PORT_PA07|PORT_PA03;
								portBs -> OUTCLR.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB04|PORT_PB05|PORT_PB06;
								key[A] = 'a';
								A ++;
								state = 0;
							}
							 portAs -> OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
                
         
                
							portAs -> OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
							portAs -> OUTCLR.reg = PORT_PA06;
							if (portAs->IN.reg & PORT_PA19 ) //4
							{
				
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg = PORT_PA00| PORT_PA07|PORT_PA04|PORT_PA03;
								portBs -> OUTCLR.reg = PORT_PB06 | PORT_PB05| PORT_PB01| PORT_PB02;
								CNT++;
					
								key[A] = '4';
								A ++;
								state = 0;
							}
							if (portAs->IN.reg & PORT_PA18 ) //5
							{
					
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg = PORT_PA01|PORT_PA04| PORT_PA07;
								portBs -> OUTCLR.reg = PORT_PB00 | PORT_PB05| PORT_PB06| PORT_PB02| PORT_PB03;
								CNT++;
					
								key[A] = '5';
								A ++;
								state = 0;
							}
							if (portAs->IN.reg & PORT_PA17 ) //6
							{
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg = PORT_PA01| PORT_PA07;
								portBs -> OUTCLR.reg = PORT_PB06 | PORT_PB05|  PORT_PB02|PORT_PA04|PORT_PA03| PORT_PB00;
								key[A] = '6';
								A ++;
								state = 0;
							}
							if (portAs->IN.reg & PORT_PA16 ) //b
							{
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg = PORT_PA01|PORT_PA07|PORT_PA00;
								portBs -> OUTCLR.reg = PORT_PB05 | PORT_PB04|PORT_PB03|PORT_PB02|PORT_PA06;
								key[A] = 'b';
								A ++;
								state = 0;
							}
						   portAs -> OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
                
                
                
							portAs -> OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
							portAs -> OUTCLR.reg = PORT_PA05;
							if (portAs->IN.reg & PORT_PA19 ) //7
							{
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg = PORT_PA03|PORT_PA04| PORT_PA07| PORT_PB06;
								portBs -> OUTCLR.reg = PORT_PB00 | PORT_PB05| PORT_PB01| PORT_PB02;
								key[A] = '7';
								A ++;
								state = 0;
							}
							if (portAs->IN.reg & PORT_PA18 ) //8
							{
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg =PORT_PA07;
								portBs -> OUTCLR.reg = PORT_PB00 | PORT_PB05| PORT_PB01| PORT_PB02| PORT_PB06|PORT_PA04|PORT_PA03;
								key[A] = '8';
								A ++;
								state = 0;
							}
							if (portAs->IN.reg & PORT_PA17 ) //9
							{
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg = PORT_PA03|PORT_PA04| PORT_PA07;
								portBs -> OUTCLR.reg = PORT_PB00 | PORT_PB05| PORT_PB06| PORT_PB02| PORT_PB01;
								key[A] = '9';
								A ++;
								state = 0;
							}
							if (portAs->IN.reg & PORT_PA16 ) //c
							{
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg = PORT_PA06|PORT_PA01|PORT_PA02|PORT_PA07;
								portBs -> OUTCLR.reg = PORT_PB00 | PORT_PB05|PORT_PB04|PORT_PB03;
								key[A] = 'c';
								A ++;
								state = 0;
							}
							portAs -> OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
            
  
                
                
							portAs -> OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
							portAs -> OUTCLR.reg = PORT_PA04;
							if (portAs->IN.reg & PORT_PA18 ) //0
							{
								portBs -> DIRSET.reg = PORT_PB00 | PORT_PB01|PORT_PB02|PORT_PB03|PORT_PB04|PORT_PB05|PORT_PB07|PORT_PB06;
								portBs -> OUTSET.reg = PORT_PA06| PORT_PA07;
								portBs -> OUTCLR.reg = PORT_PB01 | PORT_PB02| PORT_PB03| PORT_PB05|PORT_PB04;
								key[A] = '0';
								A ++;
								state = 0;
							}
							if (portAs->IN.reg & PORT_PA16 ) //D
							{
								state = 3;
							}
							 portAs -> OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
							break;
                
                
						case 3:
							key[A] = 0;
							write (key);
							portAs -> OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
							portAs -> OUTCLR.reg = PORT_PA04;
							if (portAs->IN.reg & PORT_PA16 ) //D
							{
								A = 0;
					
								state = 0;
							}
						   portAs -> OUTSET.reg = PORT_PA04|PORT_PA05|PORT_PA06|PORT_PA07;
							break;
                
					}
				}
}



// Power & GCLK management
void Power_Clk_Init(void){
    
    PM->APBCMASK.reg |= 0x1u << 6; // enable SERCOM4
    
    uint32_t temp = 0x11; // set SERCOM ID
    temp |= 0<<8;
    GCLK->CLKCTRL.reg=temp; // write ID to generic clock
    GCLK->CLKCTRL.reg |= 0x1u << 14;    // enable it.
    
}

// Port initialization
void PortInit(void)
{
    Port *por = PORT_INSTS;
    PortGroup *porB = &(por->Group[1]);
    
    // Setup Port 10 and 11 for SERCOM4
    // Enable PMUX with the PINCFG register
    porB->PINCFG[10].bit.PMUXEN = 0x01;  // 2n=10 for even , porB->PINCFG[10].bit.PMUXEN = n
    porB->PINCFG[11].bit.PMUXEN = 0x01;  // 2n+1=11 for  odd , porB->PINCFG[11].bit.PMUXEN = n
    
    // Enable Peripheral function group for SERCOM4 on PA10/11
    porB->PMUX[5].bit.PMUXE = 0x3;   //PA10 group D (SERCOM- ALT)
    porB->PMUX[5].bit.PMUXO = 0x3;   //PA11 group D (SERCOM- ALT)
}

// SERCOM4 UART initialization
void UartInit(void){
    
    Sercom *ser = SERCOM4;
    SercomUsart *uart = &(ser->USART);
    
    uart->CTRLA.bit.MODE = 1;	// 1= UART mode with internal clock, 0= orther system
    
    uart->CTRLA.bit.CMODE = 0;	// Communication mode 1 = synchronous, 0 = Asynchronous
    uart->CTRLA.bit.RXPO = 0x3; // Making pad[3] (PB11) the receive data pin
    uart->CTRLA.bit.TXPO = 0x1; // Making pad[1] (PB10) the transmit data pin
    
    /*
     Set the CTRLB Character Size to 8 bits
     */
    uart->CTRLB.bit.CHSIZE = 0x0;
    
    
    uart->CTRLA.bit.DORD = 0x1; // Data order set which bit is sent first LSB = 1, MSB = 0
    
    /*
     Set the CTRLB Stop Bit to one stop bit
     */
    uart->CTRLB.bit.SBMODE = 0;
    
    uart->CTRLB.bit.SFDE = 0x1; // Start of Frame Detection Enabled
    
    /*
     Program the BAUD register to a reasonable baud rate that can be selected from the Termite software
     */
    
    uart->BAUD.reg = 63019;  //65536(1- 16(19200/8x10^(6))
    
    while(uart->STATUS.bit.SYNCBUSY == 1){}
    
    uart->CTRLB.bit.RXEN = 0;	// Receiver enabled
    uart->CTRLB.bit.TXEN = 1;	// Transmitter enabled
    
    while(uart->STATUS.bit.SYNCBUSY == 1){}
    
    uart->CTRLA.reg |= 0x2; // Enable the UART peripheral
}

// Write text to data reg
void write(char *text)
{
    Sercom *ser = SERCOM4;
    SercomUsart *uart = &(ser->USART);
    
    char *textPtr;
    textPtr = text;
    
    while(*textPtr)
    {
        while(!(uart->INTFLAG.reg & SERCOM_USART_INTFLAG_DRE)){}
        /*
         
         
         
         Write code to set the uart->DATA.reg to each pointer value of textPtr.
         You will need to iterate through the character array pointer at each cycle.
         
         
         
         */
        uart->DATA.bit.DATA = *textPtr;
        textPtr++;
        while(!(uart->INTFLAG.reg & SERCOM_USART_INTFLAG_TXC)){}
    }
}

// Simple clock initialization	*Do Not Modify*
void Simple_Clk_Init(void)
{
    /* Various bits in the INTFLAG register can be set to one at startup.
     This will ensure that these bits are cleared */
    
    SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET |
    SYSCTRL_INTFLAG_DFLLRDY;
    
    //system_flash_set_waitstates(0);  //Clock_flash wait state =0
    
    SYSCTRL_OSC8M_Type temp = SYSCTRL->OSC8M;      /* for OSC8M initialization  */
    
    temp.bit.PRESC    = 0;    // no divide, i.e., set clock=8Mhz  (see page 170)
    temp.bit.ONDEMAND = 1;    //  On-demand is true
    temp.bit.RUNSTDBY = 0;    //  Standby is false
    
    SYSCTRL->OSC8M = temp;
    
    SYSCTRL->OSC8M.reg |= 0x1u << 1;  //SYSCTRL_OSC8M_ENABLE bit = bit-1 (page 170)
    
    PM->CPUSEL.reg = (uint32_t)0;		// CPU and BUS clocks Divide by 1  (see page 110)
    PM->APBASEL.reg = (uint32_t)0;		// APBA clock 0= Divide by 1  (see page 110)
    PM->APBBSEL.reg = (uint32_t)0;		// APBB clock 0= Divide by 1  (see page 110)
    PM->APBCSEL.reg = (uint32_t)0;		// APBB clock 0= Divide by 1  (see page 110)
    
    PM->APBAMASK.reg |= 01u<<3;   // Enable Generic clock controller clock (page 127)
    
    /* Software reset Generic clock to ensure it is re-initialized correctly */
    
    GCLK->CTRL.reg = 0x1u << 0;   // Reset gen. clock (see page 94)
    while (GCLK->CTRL.reg & 0x1u ) {  /* Wait for reset to complete */ }
    
    // Initialization and enable generic clock #0
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY){}
    *((uint8_t*)&GCLK->GENDIV.reg) = 0;  // Select GCLK0 (page 104, Table 14-10)
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY){}
    GCLK->GENDIV.reg  = 0x0100;   		 // Divide by 1 for GCLK #0 (page 104)
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY){}
    GCLK->GENCTRL.reg = 0x030600;  		 // GCLK#0 enable, Source=6(OSC8M), IDC=1 (page 101)
}

