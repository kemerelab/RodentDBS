//Code to use with DBS PCB Version 2
//Zongjun Zheng
//June 2015
//Kemere Lab, Rice University


/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/






// This code is written for the IPG PCB with HC 06 Bluetooth Module. Bluetooth Communication Terminator: Realterm
// To use the code: 1. Type and send 'A', 'W' and 'P' in realterm to setup 'A':Amplitude; 'W':Width; 'P': Period
//                  2. Type and send 'O' in realterm: enter Setup Over mode and enter timer interrupt
//                  3. If want to change 'A','W', or 'P' during experiments just repeat the first step, there's no need to repeat step 2.

#include <msp430.h>

#define S1 BIT3
#define S2 BIT2
#define S3 BIT1
#define S4 BIT4

//LED
#define PWM_cycle 200
#define PWM_duty 100

unsigned char *PTxData;                     // Pointer to TX data
unsigned char TXByteCtr;
unsigned char state=0;					    // status variable
unsigned char a;                            // value for TXData input
unsigned char TxData[];                     // Table of data to transmit (channel 1)
int width;                                  // width of each phase
int period;                                 // period of the biphasic current pattern
int range;                                  // period = range * 100; range is sent to MSP430 via bluetooth module
const unsigned char TxData1[] =             // Table of data to transmit  (channel 2)
{
  0xF9,
  0x12
};

void PWM_TA1_Setup(void){
	TA1CCR0 = PWM_cycle;	                //pwm period
	TA1CCTL2 = OUTMOD_7;		            //CCR2 reset/set
	TA1CCR2 = PWM_duty;			            //pwm duty cycle p2.5 brightness

	TA1CTL = TASSEL_2 + MC_1;	            //up till CCR0;

	P2DIR |= BIT5;			                //initialize P2.5
	P2SEL |= BIT5;			                //select P2.5 as PWM outputs
}

int main(void)
{
	//timer setup
	WDTCTL = WDTPW + WDTHOLD;               // Stop WDT
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;


    //PWM_Setup
    PWM_TA1_Setup();

   // Bluetoth Setup
    P1SEL = BIT1+BIT2+BIT4;                   // pins needed for bluetooth communication
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;
	UCA0CTL1 &= ~UCSWRST;
	P1SEL2 = BIT1 + BIT2+BIT4;



	//I2C setup
	P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
	UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	UCB0BR0 = 2.5;                            // fSCL = SMCLK/20 = ~400kHz
	UCB0BR1 = 0;
	UCB0I2CSA = 0x48;                         // Slave Address is 048h
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation

	IE2 |= UCA0RXIE;                          // turn on UCA0RX interrupt
	__bis_SR_register(LPM0_bits + GIE);       // Go into LPM0 mode

}

#pragma vector = TIMER0_A0_VECTOR	//says that the interrupt that follows will use the "TIMER0_A0_VECTOR" interrupt
__interrupt void Timer_A(void){
	switch(state) {
	case 0:
		P2OUT=S1+S4;					      //Step0: turn on S1 and S4
		CCR0+=width;				          //increment CCR0
		state=1;						      //flip the state
		break;
	case 1:
		P2OUT=S2+S3;					      //Step1: turn off S1&S4, turn on S2&S3
		CCR0+=width;
		state=2;
		break;
	case 2:
		P2OUT=S2+S4;					      //short both ends to gnd
		CCR0+=period-2*width;
		state=0;
		break;
	default:
		break;
	}
}

#pragma vector=USCIAB0RX_VECTOR           //says that the interrupt that follows will use the "USCIAB0RX_VECTOR" interrupt
__interrupt void USCI0RX_ISR(void) {

if (UCA0RXBUF =='A')                      // Change Amplitude Mode. Type and send character 'A' in realterm to get into the change amplitude mode.
{
	while(UCA0RXBUF=='A');                // Wait for the amplitude
	a=UCA0RXBUF;                          // let variable 'a' equals to the data been sent to MSP430
	TA1CCR2=1;                            // turn the LED dim to indicate data has been sent
	unsigned char TxData[]=               // Table of data to transmit
			{
			  0xF8,
			  a                           // input the data been sent to the table
			};

	IE2 |= UCB0TXIE;                      // Enable TX interrupt: transmit data to current source

	//I2C send data, OUT0 (first channel)
	PTxData = (unsigned char *)TxData;      // TX array start address
	TXByteCtr = sizeof TxData;              // Load TX byte counter
	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
	__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts

	//I2C send data, OUT1  (second channel)
	PTxData = (unsigned char *)TxData1;     // TX array start address
	TXByteCtr = sizeof TxData1;             // Load TX byte counter
	while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
	UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
	__bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
}
else if (UCA0RXBUF == 'W'){                 // Change Width mode. Type and send character 'W' in realterm to get into the change phase width mode.
	while(UCA0RXBUF=='W');                  // Wait for the width
	width=UCA0RXBUF;                        // let variable 'width' equals to the data been sent to MSP430
	TA1CCR2 = 100;                          // turn the LED bright to indicate data has been sent
}
else if (UCA0RXBUF == 'P'){                 // Change Period mode. Type and send character 'P' in realterm to get into the change pattern period mode.
	while(UCA0RXBUF=='P');                  // Wait for the period
	range=UCA0RXBUF;                        // let variable 'range' equals to the data been sent to MSP430
	TA1CCR2 = 0;                            // turn the LED off to indicate data has been sent
	period=range*100;                       // compute the period by using the 'range' variable. (A 7000-13000 period needs a 70-130 range)
}
else if (UCA0RXBUF=='O'){                   // Setup Over mode. Type and send character 'O' in realterm to get into setup over mode.
// Notice: This mode is for setup use. After the first time entering this mode, there's no need to enter this mode again in the future when changing parrameters.
	P2DIR |= BIT1 + BIT2 + BIT3 + BIT4;	    //set pins 2.1, 2.2, 2.3, and 2.4 as outputs
	P2OUT &= ~(BIT4 + BIT1 + BIT2 + BIT3);	//initializes outputs of pins 2.1, 2.2, 2.3, and 2.4 as 0 (all open switches)
	CCTL0 = CCIE;				            //Puts the timer control on CCR0
	CCR0 = 5000;				            //first interrupt period
	TACTL = TASSEL_2 + MC_2 + ID_0;	        //Use SMCLK to interrupt (because VLO wouldn't meet timing requirement of 60us)
	TA1CCR2 = 1;                            // indicating Setup over.
	__enable_interrupt();				    //global interrupt enable
    __bis_SR_register(LPM1+GIE);            // Enter LPM1 w/ interrupts
}
}
//------------------------------------------------------------------------------
// The USCIAB0TX_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with the byte count. Also, TXData
// points to the next byte to transmit.
//------------------------------------------------------------------------------
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  if (TXByteCtr)                            // Check TX byte counter
  {
    UCB0TXBUF = *PTxData++;                 // Load TX buffer
    TXByteCtr--;                            // Decrement TX byte counter
  }
  else
  {
    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
    IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
  }
}
