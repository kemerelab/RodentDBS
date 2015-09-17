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
#include "CurrentSource.h"
#include "string.h"


// GLOBALS
volatile uint32_t DeviceMasterClock;
volatile unsigned char KernelWakeupFlag = 0;
// This flag locks the main kernel timer from waking up a subprocess that has slept.

volatile DeviceData_t DeviceData = {\
        .ID.idStr = DEFAULT_DEVICE_IDSTR, .ID.firmwareVersion = PROTOCOL_VERSION, \
        .Status.BatteryVoltage = 0, .Status.Uptime = 0, .Status.LastUpdate = 0,\
        .StimParams.Enabled = 0, .StimParams.Period = 7500, \
        .StimParams.Amplitude = 100, .StimParams.PulseWidth = 600 };
volatile DeviceStatus_t DeviceStatus = {.BatteryVoltage = 0, .Uptime = 0, .LastUpdate = 0};
volatile int StimParamsChanged = 0;
volatile int PowerLEDIntensity = 50;

#define CHECK_BATTERY_PERIOD 2500
volatile int CheckBatteryCounter = CHECK_BATTERY_PERIOD-1;
#define READ_NFC_DATA_PERIOD 333
volatile int ReadNFCDataCounter = READ_NFC_DATA_PERIOD-1;
#define UPDATE_NFC_DATA_PERIOD 500
volatile int UpdateNFCDataCounter = UPDATE_NFC_DATA_PERIOD-1;


void MasterClockSetup(void){
    P2DIR |= BIT5;          //initialize P2.5
    P2SEL |= BIT5;          //select P2.5 as PWM outputs

    TA1CCR0 = 1000;            // 1 ms period
    TA1CTL = TASSEL_2 + MC_1;  // SMCLK (1 MHz), up mode, enable interrupt
    TA1CCTL0 = CCIE;           // CCR0 interrupt enabled

    TA1CCTL2 = OUTMOD_7;           //CCR2 reset/set
    TA1CCR2 = PowerLEDIntensity;   //pin 2.5 brightness (PWM duty cycle versus TA1CCR0)
}

inline void Blink(void) {

}

void main(void)
{
    StimParams_t NewStimParams = { .Enabled = 0, .Period = 7500, \
            .Amplitude = 100, .PulseWidth = 600 };

    int StimParamsChanged = 0;

    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

    // Set up system clocks
    BCSCTL1 = CALBC1_8MHZ;   // Set range
    DCOCTL = CALDCO_8MHZ;    // Set DCO step + modulation
    BCSCTL3 |= LFXT1S_2;     // LFXT1 = VLO (~12 kHz)

    // Default - initialize all ports to output
    P1DIR = 0xFF; P2DIR = 0xFF; P3DIR = 0xFF;
    P1OUT = 0; P2OUT = 0; P3OUT = 0;

    I2CSetup();

    BatteryStatusSetup();

    /*
     * It would be good to add error checking here somehow.
     * Maybe make sure that I2C peripherals are present (I suppose this
     * requires I2C timeouts!).
     *
     */

    //SetupSwitchMatrix();

    NFCInterfaceSetup();

    MasterClockSetup();

    //EnableStimulation();

    //__enable_interrupt();   //global interrupt enable


    while (1) {

        if (CheckBatteryCounter <= 0) {
            CheckBattery();
            CheckBatteryCounter = CHECK_BATTERY_PERIOD-1;
        }


        KernelWakeupFlag = 0;
        __bis_SR_register(LPM1_bits | GIE);        // Enter LPM1 w/ interrupts
        KernelWakeupFlag = 1;


        if (ReadNFCDataCounter <= 0 ) {
            if (ReadDeviceParams(&NewStimParams) != 0) {
                StimParamsChanged = memcmp((unsigned char*)&NewStimParams,
                        (unsigned char*)&(DeviceData.StimParams), sizeof(NewStimParams));
                if (StimParamsChanged != 0) {
                    DisableStimulation();
                    DeviceData.StimParams.Period = NewStimParams.Period;
                    DeviceData.StimParams.Amplitude = NewStimParams.Amplitude;
                    SetOutputCurrent()
                    DeviceData.StimParams.PulseWidth = NewStimParams.PulseWidth;
                    DeviceData.StimParams.Enabled = NewStimParams.Enabled;
                    if (NewStimParams.Enabled != 0) {
                        EnableStimulation();
                        Blink();
                    }
                    DeviceStatus.LastUpdate = DeviceStatus.Uptime;
                }
            }
            ReadNFCDataCounter = READ_NFC_DATA_PERIOD-1;
        }


        KernelWakeupFlag = 0;
        __bis_SR_register(LPM1_bits | GIE);        // Enter LPM1 w/ interrupts
        KernelWakeupFlag = 1;


        if (UpdateNFCDataCounter <= 0) {
            UpdateDeviceStatus();
            UpdateNFCDataCounter = UPDATE_NFC_DATA_PERIOD-1;
        }

        KernelWakeupFlag = 0;
        __bis_SR_register(LPM1_bits | GIE);        // Enter LPM1 w/ interrupts
        KernelWakeupFlag = 1;

        // Add heartbeat functionality.

    }
}


// Timer A1 interrupt service routine => Assume CCR0 set for 1 ms ticks
#pragma vector=TIMER1_A0_VECTOR
__interrupt void MasterClockISR (void)
{
    static unsigned int SecondCounter = 1000;
   //TA1CCR0 += 1000;        // 1 ms period

    if (--SecondCounter == 0) {
        DeviceStatus.Uptime += 1;
        SecondCounter = 1000;
    }

    // Timer variables for each subprocess that runs in main loop
    CheckBatteryCounter--;
    UpdateNFCDataCounter--;
    ReadNFCDataCounter--;

    if (KernelWakeupFlag == 0) { // If the master loop has gone to sleep then wake it up
        __bic_SR_register_on_exit(LPM1_bits);
    }

}
