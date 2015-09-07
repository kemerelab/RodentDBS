/*
 * Copyright (c) 2015, Caleb Kemere
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * --/COPYRIGHT--
 *
 *******************************************************************************
 *
 *                       Rodent Stimulation Module (RSM) Firmware
 *
 * This file contains the main loop for the RSM module. The main loop cycles
 * between 3 phases of stimulation (forward current, reverse current, and inter-
 * stimulation), and additionally performs other monitoring and communication
 * functionality.
 *
 * Resources:
 *  - Uses watchdog timer for general program state control
 *  - Uses Timer A0 for stimulation pulse timing
 *  - Uses PWM/Timer A1 module for display light (and Pin 2.5)
 *  - Uses Pins 1.0-1.3 for control of switch array
 *  - Uses Pins 1.6, 1.7 for I2C ()
 *
*/

#include <msp430.h>
#include "Firmware.h"
#include "BatteryStatus.h"
#include "SwitchMatrix.h"
#include "I2C.h"
#include "NFCInterface.h"

/* Program coordination variables */
unsigned char ProgramState=0;							// status variable
long int Uptime = 0;
long int LastUpdate = 0;
long int LastChange = 0;

/* Stimulation parameters *
 *  - These are accessed in the stimulation code, but set in the communcation code
 */
int StimulationPhase;
int PulseWidth;
int InterPulseInterval;
int Period;
int Amplitude;
int StimParameterMutex;

unsigned int x=0;

#define DS4432_ADDRESS 0x48
#define DS4432_CURRENT0_REG_ADDR 0xF8

void PWM_TA1_Setup(void){
	TA1CCR0 = PWM_cycle;				//pwm period
	TA1CCTL2 = OUTMOD_7;		//CCR2 reset/set
	TA1CCR2 = PWM_duty;			//pwm duty cycle p2.5 brightness

	TA1CTL = TASSEL_1 + MC_1;	//Use ACLK, up till CCR0;

	P2DIR |= BIT5;			//initialize P2.5
	P2SEL |= BIT5;			//select P2.5 as PWM outputs
}


inline void SetOutputCurrent (void) {
	// refer to global Amplitude variable!
	WriteRegister_ByteAddress(DS4432_ADDRESS,DS4432_CURRENT0_REG_ADDR, 0xF1);
}



void main(void)
{
     unsigned short temp;

 	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

    // Set up system clocks
    BCSCTL1 = CALBC1_8MHZ; 					// Set range
    DCOCTL = CALDCO_8MHZ;  					// Set DCO step + modulation
    BCSCTL2 = DIVS_3;  // Setup SMCLK to be 1 MHz rather than 8
    BCSCTL3 |= LFXT1S_2;                     // LFXT1 = VLO

    // Default - initialize all ports to output
    P1DIR = 0xFF; P2DIR = 0xFF;	P3DIR = 0xFF;
    P1OUT = 0; P2OUT = 0; P3OUT = 0;

    I2CSetup();

    //BatteryStatusSetup();

    //SetupSwitchMatrix();

    //PWM_Setup
    //PWM_TA1_Setup();

    NFCInterfaceSetup();

    //EnableStimulation();

    __enable_interrupt();	        //global interrupt enable

    __bis_SR_register(LPM1 | GIE);        // Enter LPM1 w/ interrupts
    while(1);
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR	//says that the interrupt that follows will use the "TIMER0_A0_VECTOR" interrupt
__interrupt void Timer_A(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
	switch(NextStimulationState) {
	case FORWARD:
		SetSwitchesForward();
		CCR0+=PulseWidth;						//increment CCR0
		break;
	case REVERSE:
		SetSwitchesReverse();
		CCR0+=PulseWidth;
		break;
	case GROUNDED:
		SetSwitchesGround();
		if (disableStimulationFlag) {
			NextStimulationState = OFF;
			TA0CCTL0 = ~CCIE;
		}
		else
			CCR0+=InterPulseInterval - (PulseWidth + PulseWidth);
		break;
	case OFF:
	default: // should never reach here
		SetSwitchesGround();
		TA0CCTL0 = ~CCIE;
		break;
	}
}
