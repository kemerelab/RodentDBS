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
 * Battery Status Code
 *
 * This file contains the code required to use the internal voltage references
 * to measure the battery voltage
 *
*/

#include <msp430.h>
#include "BatteryStatus.h"
#include "Firmware.h"

int BatteryVoltage;
long int BatteryStatusTickCounter;

batteryStatusStateEnum BatteryStatusState;

/*
 * Initialize the ADC10 to prepare to measure the battery voltage.
 */
void BatteryStatusSetup(void){  //ADC10 setup function
    BatteryVoltage = 0;
    BatteryStatusState = ASLEEP;
    BatteryStatusTickCounter = 0;

    ADC10CTL0 = SREF_1 + REF2_5V; // User Vref+ as positive rail and Vss as negative; Use 2.5 V Vref
    ADC10CTL0 = ADC10CTL0 + ADC10SR; // Set low speed sampling to minimize reference buffer power
    ADC10CTL0 = ADC10CTL0 + ADC10SHT_3; // Sample for longest period (64 clks)
    ADC10CTL1 = INCH_11; // For data source, use internal voltage, (Vcc-Vss)/2
    ADC10CTL1 = ADC10CTL1 + ADC10SSEL_3; // Use the master clock for the ADC (set as 8 MHz)
    ADC10AE0 = BIT0;

}

void ExecuteBatteryStatus(void) {
    switch (BatteryStatusState) {
    case ASLEEP:
        BatteryStatusState = STARTING_REF;
        ADC10CTL0 = ADC10CTL0 | (REFON + ADC10ON + ADC10IE);
        BatteryStatusTickCounter = 30; // Need to wait 30 us before next call to function
        break;
    case STARTING_REF:
        BatteryStatusState = SAMPLING;
        ADC10CTL0 |= ENC; // ADC10 enable set; this triggers sampling and will result in the ISR when data ready
        BatteryStatusTickCounter = 200; // wait for this to have occured and then go abck asleep
        break;
    case SAMPLING:
        BatteryStatusState = ASLEEP;
        BatteryStatusTickCounter = 1000000; // wait for a second to sample next
        break;
    }
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{   //
    ADC10CTL0 &= ~ENC;      //disable ADC
    ADC10CTL0 &= ~(REFON + ADC10ON + ADC10IE); // and then shutdown completely
    BatteryVoltage=ADC10MEM;
}

