

#include <asf.h>

// Void functions declaration
void wait(int t);
void Simple_Clk_Init(void);

// Global Variables
volatile uint32_t count = 0;                // important to keep volatile (ask TA)

int main (void)
{
    // Set micro-controller clock to 8Mhz
    Simple_Clk_Init();

    // Set the base address for the Port structure to PORT_INSTS or 0x41004400
    Port *ports = PORT_INSTS;
    
 
    PortGroup *porA = &(ports->Group[0]);  //assigns pointer to group A ports
    PortGroup *porB = &(ports->Group[1]); //assigns pointer to group B ports
	
	porA->DIRSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;  //set as output for ss displays and rows of keypad
	porB->DIRSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;  //set ss as outputs
	porA->DIRCLR.reg = PORT_PA16 | PORT_PA17 | PORT_PA18 | PORT_PA19;  // set column of keypad as inputs
	porA->OUTCLR.reg = PORT_PA16 | PORT_PA17 | PORT_PA18 | PORT_PA19;  // initialize the inputs as logic low
	
	porA -> PINCFG[16].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;		// configure port inputs for each column
	porA -> PINCFG[17].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA -> PINCFG[18].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	porA -> PINCFG[19].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	
	
	
		
		
	for(int i = 0; i<=4; i++){
			if(i=0){			//restart loop
				porB->OUTSET.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;  //reset display
				porA->OUTSET.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;			//de energize each row
			}
			
			if(i=1){						//first row
				porA->OUTCLR.reg = PORT_PA07;					//energize first row
					if(porA->IN.reg & PORT_PA19){								//check if "1" is pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;	 // turn on all ss
						porB->OUTCLR.reg = PORT_PB01 | PORT_PB02;			// display "1" on ss
						wait(2);
						i=0;
					}
					else if(porA->IN.reg & PORT_PA18){						//check if "2" pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB03 | PORT_PB04 | PORT_PB06;  //display 2
						wait(2);
						i=0;
					}
					else if(porA->IN.reg & PORT_PA17){						//check if "3" pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB06;	//display 3
						wait(2);
						i=0;
					}
					else if(porA->IN.reg & PORT_PA16){						//check if "A" pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB04 | PORT_PB05 | PORT_PB06;	// display A
						wait(2);
						i=0;
					}
			
			}
			if(i=2){					//second row
				porA->OUTCLR.reg = PORT_PA06;					//energize first row
					if(porA->IN.reg & PORT_PA19){								//check if "4" is pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;			// display 4 on ss
						wait(2);
						i=0;
					}
					else if(porA->IN.reg & PORT_PA18){						//check if "5" pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB00 | PORT_PB05 | PORT_PB06 | PORT_PB02 | PORT_PB03;  //display 5
						wait(2);
						i=0;
					}
					else if(porA->IN.reg & PORT_PA17){						//check if "6" pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB00 | PORT_PB05 | PORT_PB04 | PORT_PB03 | PORT_PB02 | PORT_PB06;	//display 6
						wait(2);
						i=0;
					}
					else if(porA->IN.reg & PORT_PA16){						//check if "b" pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;	// display b
						wait(2);
						i=0;
					}
				}
			if(i=3){					//third row
				porA->OUTCLR.reg = PORT_PA05;					//energize first row
					if(porA->IN.reg & PORT_PA19){								//check if "7" is pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02;			// display 7 on ss
						wait(2);
						i=0;
					}
					else if(porA->IN.reg & PORT_PA18){						//check if "8" pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05 | PORT_PB06;  //display 8
						wait(2);
						i=0;
					}
					else if(porA->IN.reg & PORT_PA17){						//check if "9" pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB05 | PORT_PB06;	//display 9
						wait(2);
						i=0;
					}
					else if(porA->IN.reg & PORT_PA16){						//check if "C" pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB00 | PORT_PB05 | PORT_PB04 | PORT_PB03;	// display C
						wait(2);
						i=0;
					}
				}
			if(i=4){					//fourth row
				porA->OUTCLR.reg = PORT_PA04;					//energize first row
					if(porA->IN.reg & PORT_PA18){						//check if "0" pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg = PORT_PB00 | PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB05;  //display 
						wait(2);
						i=0;
					}
					else if(porA->IN.reg & PORT_PA16){						//check if "d" pressed
						porA->OUTCLR.reg = PORT_PA04 | PORT_PA05 | PORT_PA06 | PORT_PA07;
						porB->OUTCLR.reg =  PORT_PB01 | PORT_PB02 | PORT_PB03 | PORT_PB04 | PORT_PB06;	// display d
						wait(2);
						i=0;
					}
			
		}
	

	
}


}

// Time delay function
void wait(int t)
{
    count = 0;
    while (count < t*1000000)
    {
        count++;
    }
}

// Simple Clock Initialization - Do Not Modify - //
void Simple_Clk_Init(void)
{
    /* Various bits in the INTFLAG register can be set to one at startup.
       This will ensure that these bits are cleared */
    
    SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET |
            SYSCTRL_INTFLAG_DFLLRDY;
            
    system_flash_set_waitstates(0);             // Clock_flash wait state = 0

    SYSCTRL_OSC8M_Type temp = SYSCTRL->OSC8M;   // for OSC8M initialization 

    temp.bit.PRESC    = 0;                      // no divide, i.e., set clock=8Mhz  (see page 170)
    temp.bit.ONDEMAND = 1;                      // On-demand is true
    temp.bit.RUNSTDBY = 0;                      // Standby is false
    
    SYSCTRL->OSC8M       = temp;

    SYSCTRL->OSC8M.reg  |= 0x1u << 1;           // SYSCTRL_OSC8M_ENABLE bit = bit-1 (page 170)
    
    PM->CPUSEL.reg  = (uint32_t)0;              // CPU and BUS clocks Divide by 1  (see page 110)
    PM->APBASEL.reg = (uint32_t)0;              // APBA clock 0= Divide by 1  (see page 110)
    PM->APBBSEL.reg = (uint32_t)0;              // APBB clock 0= Divide by 1  (see page 110)
    PM->APBCSEL.reg = (uint32_t)0;              // APBB clock 0= Divide by 1  (see page 110)

    PM->APBAMASK.reg    |= 01u<<3;              // Enable Generic clock controller clock (page 127)

    /* Software reset Generic clock to ensure it is re-initialized correctly */

    GCLK->CTRL.reg  = 0x1u << 0;                // Reset gen. clock (see page 94)
    while (GCLK->CTRL.reg & 0x1u ) {  /* Wait for reset to complete */ }
    
    // Initialization and enable generic clock #0

    *((uint8_t*)&GCLK->GENDIV.reg) = 0;         // Select GCLK0 (page 104, Table 14-10)

    GCLK->GENDIV.reg     = 0x0100;              // Divide by 1 for GCLK #0 (page 104)

    GCLK->GENCTRL.reg    = 0x030600;            // GCLK#0 enable, Source=6(OSC8M), IDC=1 (page 101)
}


