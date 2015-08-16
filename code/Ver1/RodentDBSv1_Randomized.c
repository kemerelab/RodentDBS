//Code to use with DBS PCB V1.2/V1.3
//Boying Meng
//November 2014
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

#include <msp430.h>

#define S1 BIT3
#define S2 BIT2
#define S3 BIT1
#define S4 BIT4

//LED
#define PWM_cycle 180
#define PWM_duty 1

unsigned char *PTxData;                     // Pointer to TX data
unsigned char TXByteCtr;
unsigned char state=0;							// status variable
//unsigned int p1,p2;

//stimulation parameters, timer interrupt f=1MHz	*********CHANGE PARAMETERS HERE**********
unsigned int PULSE_WIDTH=60;
unsigned int PULSE_PERIOD=6692;			// stimulating at 130Hz(6.7ms~8.7ms)
unsigned int PULSE_SLEEP;
unsigned int PULSE_OFFSET[32]={
		1137, 938, 23, 674, 324, 1588, 622, 1057,
		331, 1203, 525, 1308, 1378,	1496, 901, 167,
		457, 1826, 304, 1651, 1076, 1992, 156, 885,
		213, 1923, 9, 1549, 1634, 1737, 168, 799
};

unsigned int PULSE_index=0;

unsigned int V_Warning=586;					// may need to adjust this value...

unsigned int x=0;

const unsigned char TxData[] =              // Table of data to transmit
{
  0xF8,			//out0
  0xF1
};

const unsigned char TxData1[] =              // Table of data to transmit
{
  0xF9,			//out1
  0xF4
};

void ADC_Setup(void){	//ADC10 setup function
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE + REF2_5V;
	ADC10CTL1 = INCH_0 + ADC10DIV_0;// + ADC10SSEL0;

	ADC10AE0 = BIT0;

	P1DIR &=~BIT0;				//P1.0 as input
	P1SEL |= BIT0;				//P1.0 as input
}

void PWM_TA1_Setup(void){
	TA1CCR0 = PWM_cycle;				//pwm period
	TA1CCTL2 = OUTMOD_7;		//CCR2 reset/set
	TA1CCR2 = PWM_duty;			//pwm duty cycle p2.5 brightness

	TA1CTL = TASSEL_1 + MC_1;	//Use ACLK, up till CCR0;

	P2DIR |= BIT5;			//initialize P2.5
	P2SEL |= BIT5;			//select P2.5 as PWM outputs
}

int main(void)
{
	//timer setup
	BCSCTL1 = CALBC1_8MHZ; 					// Set range
	DCOCTL = CALDCO_8MHZ;  					// Set DCO step + modulation
	BCSCTL3 |= LFXT1S_2;                      // LFXT1 = VLO

	//I2C setup
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
	UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	UCB0BR0 = 20;                             	// fSCL = SMCLK/20 = ~400kHz
	UCB0BR1 = 0;
	UCB0I2CSA = 0x48;                         // Slave Address is 048h
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	IE2 |= UCB0TXIE;                          // Enable TX interrupt

	//I2C send data, OUT0
    PTxData = (unsigned char *)TxData;      // TX array start address
    TXByteCtr = sizeof TxData;              // Load TX byte counter
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
                                            // Remain in LPM0 until all data
                                            // is TX'd

	//I2C send data, OUT1
    PTxData = (unsigned char *)TxData1;      // TX array start address
    TXByteCtr = sizeof TxData1;              // Load TX byte counter
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
                                             //Remain in LPM0 until all data
                                             //is TX'd

    //calculate sleep period between pulses
    PULSE_SLEEP=PULSE_PERIOD-PULSE_WIDTH-PULSE_WIDTH;

    //switch pin setup
    P2DIR |= BIT1 + BIT2 + BIT3 + BIT4;	 //set pins 2.1, 2.2, 2.3, and 2.4 as outputs
    P2OUT &= ~(BIT4 + BIT1 + BIT2 + BIT3);	//initializes outputs of pins 2.1, 2.2, 2.3, and 2.4 as 0 (all open switches)

    //switch pin setup
    P1DIR |= BIT1 + BIT2 + BIT3 + BIT4;	 //set pins 1.1, 1.2, 1.3, and 1.4 as outputs
    P1OUT &= ~(BIT4 + BIT1 + BIT2 + BIT3);	//initializes outputs of pins 1.1, 1.2, 1.3, and 1.4 as 0 (all open switches)

    //ADC Setup
    ADC_Setup();

    //Vcc Measurement Aid
    P1DIR |= BIT5;		//set pin1.5 as output
    P1OUT &= ~BIT5;		//initially pin1.5 is low(gnd)

    //PWM_Setup
    PWM_TA1_Setup();

    // main timer interrupt setup
  	CCTL0 = CCIE;				//Puts the timer control on CCR0
  	CCR0 = 5000;				//first interrupt period
  	TACTL = TASSEL_2 + MC_2 + ID_3;	//Use SMCLK to interrupt (because VLO wouldn't meet timing requirement of 60us)

	__enable_interrupt();				//global interrupt enable
    __bis_SR_register(LPM1+GIE);        // Enter LPM1 w/ interrupts

}

#pragma vector = TIMER0_A0_VECTOR	//says that the interrupt that follows will use the "TIMER0_A0_VECTOR" interrupt
__interrupt void Timer_A(void){		//
	switch(state) {
	case 0:
		P2OUT=S1+S4;					//Step0: turn on S1 and S4
		P1OUT=S1+S4;
		CCR0+=PULSE_WIDTH;						//increment CCR0
		state=1;						//flip the state
		break;
	case 1:
		P2OUT=S2+S3;					//Step1: turn off S1&S4, turn on S2&S3
		P1OUT=S2+S3;
		CCR0+=PULSE_WIDTH;
		state=2;
		break;
	case 2:
		P2OUT=S2+S4;					//short both ends to gnd
		P1OUT=S2+S4;
		CCR0+=PULSE_SLEEP+PULSE_OFFSET[PULSE_index&0x001F];	//randomize
		PULSE_index++;
		state=0;

		//sample Vcc voltage
		x++;
		if (x==10000) {					//sampling rate ~ 100s?
			x=0;							//reset x;
			P1OUT|=BIT5;					//raise pin1.5 to Vcc
			ADC10CTL0 |= ENC + ADC10SC;		//enable ADC and take a sample and go into ADC interrupt
		}
		break;
	default:
		break;
	}
	//p2=P2OUT;						//see status of P2OUT...
}

#pragma vector = ADC10_VECTOR		//says that the interrupt that follows will use the "ADC10_VECTOR" interrupt
__interrupt void ADC10_ISR(void){	//
	unsigned int DutyCycle=0;		//a variable for fast calculations
	DutyCycle=ADC10MEM;
	if (DutyCycle<V_Warning){
		TA1CCR0 = 24000;
		TA1CCR2 = 2;
	} else {	// in case
		TA1CCR0 = PWM_cycle;
		TA1CCR2 = PWM_duty;
	}
	P1OUT&=~BIT5;					//set pin1.5 back to gnd
	ADC10CTL0 &= ~ENC;		//turn ADC temporarily off
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
    __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
  }
}
