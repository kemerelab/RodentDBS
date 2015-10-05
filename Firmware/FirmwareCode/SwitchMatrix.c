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
 * This file contains the code which controls the switch matrix that creates the
 * biphasic current pulse.
 *
 *   Input Current --------  S_IN1 \----------/ S_OUT1 ----|
 *                     |                 |                 |
 *                     |                 V                 |
 *                     |               BRAIN               |
 *                     |                 ^                 |
 *                     |                 |                 |
 *                     |---  S_IN2 \-----|----/ S_OUT2 ----|------ GND
 *
*/

#include <msp430.h>
#include "SwitchMatrix.h"
#include "Board.h"

stimulationStateEnum NextStimulationState = OFF;
int disableStimulationFlag = 0;

inline void SetupSwitchMatrix(void) {
    //switch pin setup
    P1DIR |= S_OUT1 + S_OUT2 + S_IN1 + S_IN2;     //set pins 1.1, 1.2, 1.3, and 1.4 as outputs
    SetSwitchesOff();
    disableStimulationFlag = 0;
    TA0CTL = TASSEL_2 + MC_2; // Use SMCLK for source (1 MHz)
}

inline void EnableStimulation(void) {
    TA0CCR0 = DeviceData.StimParams.Period;   // First interrupt period
    NextStimulationState = FORWARD;
    TA0CCTL0 = CCIE;    // Enable interrupt
}

inline void DisableStimulation (void) {
    disableStimulationFlag = 1;
}

inline void SetSwitchesOff(void) {
    P1OUT &= ~(S_OUT1 + S_OUT2 + S_IN1 + S_IN2);
    NextStimulationState = OFF;
}

inline void SetSwitchesForward(void) {
    P1OUT &= ~(S_OUT1 + S_IN2);
    P1OUT |= (S_IN1 + S_OUT2);
    NextStimulationState = REVERSE;
}

inline void SetSwitchesReverse(void) {
    P1OUT &= ~(S_IN1 + S_OUT2);
    P1OUT |= ~(S_OUT1 + S_IN2);
    NextStimulationState = GROUNDED;
}

inline void SetSwitchesGround(void) {
    P1OUT &= ~(S_OUT1 + S_OUT2 + S_IN1 + S_IN2);
    P1OUT |= (S_OUT1 + S_OUT2);
    NextStimulationState = FORWARD;
}


#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void)
{
    switch(NextStimulationState) {
    case FORWARD:
        SetSwitchesForward();
        CCR0+=DeviceData.StimParams.PulseWidth; //increment CCR0
        break;
    case REVERSE:
        SetSwitchesReverse();
        CCR0+=DeviceData.StimParams.PulseWidth; //increment CCR0
        break;
    case GROUNDED:
        SetSwitchesGround();
        if (disableStimulationFlag) {
            NextStimulationState = OFF;
            TA0CCTL0 = ~CCIE;
        }
        else {
            CCR0 += DeviceData.StimParams.Period - \
               (DeviceData.StimParams.PulseWidth + DeviceData.StimParams.PulseWidth); //increment CCR0
            if (DeviceData.StimParams.JitterLevel > 0) {
                CCR0 -= (1999 >> ( 3 - DeviceData.StimParams.JitterLevel)); // subtract off half of jitter level!
                CCR0 += (jitterValueTable[jitterTableCounter++] >> ( 3 - DeviceData.StimParams.JitterLevel));
                if (jitterTableCounter >= jitterTableLength)
                    jitterTableCounter = 0;
            }
        }
        break;
    case OFF:
    default: // should never reach here
        SetSwitchesGround();
        TA0CCTL0 = ~CCIE;
        break;
    }
}
