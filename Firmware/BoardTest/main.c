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
 * This code is used to test the functionality of an RSM module.
 *
 * In a functioning module, the LED should blink following programming.
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
#include "stdint.h"


// GLOBALS
volatile uint32_t DeviceMasterClock;
volatile unsigned char KernelWakeupFlag = 0;
// This flag locks the main kernel timer from waking up a subprocess that has slept.

void MasterClockSetup(void){
    P2DIR |= BIT5;          //initialize P2.5
    P2SEL |= BIT5;          //select P2.5 as PWM outputs

    TA1CCR0 = 1000;            // 1 ms period
    TA1CTL = TASSEL_2 + MC_1;  // SMCLK (1 MHz), up mode, enable interrupt
    TA1CCTL0 = CCIE;           // CCR0 interrupt enabled

    TA1CCTL2 = OUTMOD_7;           //CCR2 reset/set
    TA1CCR2 = 900;   //pin 2.5 brightness (PWM duty cycle versus TA1CCR0)
}

inline void Blink(void) {

}

void main(void)
{

    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

    // Set up system clocks
    BCSCTL1 = CALBC1_8MHZ;   // Set range
    DCOCTL = CALDCO_8MHZ;    // Set DCO step + modulation
    BCSCTL3 |= LFXT1S_2;     // LFXT1 = VLO (~12 kHz)

    // Default - initialize all ports to output
    P1DIR = 0xFF; P2DIR = 0xFF; P3DIR = 0xFF;
    P1OUT = 0; P2OUT = 0; P3OUT = 0;

    MasterClockSetup();

    while(1) {}

}


// Timer A1 interrupt service routine => Assume CCR0 set for 1 ms ticks
#pragma vector=TIMER1_A0_VECTOR
__interrupt void MasterClockISR (void)
{
    static unsigned int SecondCounter = 1000;
   //TA1CCR0 += 1000;        // 1 ms period

    if (--SecondCounter == 0) {
        SecondCounter = 1000;
    }


    if (KernelWakeupFlag == 0) { // If the master loop has gone to sleep then wake it up
        __bic_SR_register_on_exit(LPM1_bits);
    }

}
