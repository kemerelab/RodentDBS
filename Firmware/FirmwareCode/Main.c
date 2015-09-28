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
#include "Board.h"
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

// Initialize the device data. Descriptive - ID and firmware version; Status -
//   uptime, battery voltage, and last update.
volatile DeviceData_t DeviceData = {\
        .ID.idStr = DEFAULT_DEVICE_IDSTR, .ID.firmwareVersion = PROTOCOL_VERSION, \
        .Status.BatteryVoltage = 0, .Status.Uptime = 0, .Status.LastUpdate = 0,\
        .StimParams.Enabled = 0, .StimParams.Period = 7500, \
        .StimParams.Amplitude = 100, .StimParams.PulseWidth = 60, \
        .Status.BatteryVoltage = 0, .Status.Uptime = 0, .Status.LastUpdate = 0};
//volatile DeviceStatus_t DeviceStatus = {.BatteryVoltage = 0, .Uptime = 0, .LastUpdate = 0};
volatile int StimParamsChanged = 0;

/*
 * State variables for main loop control.
 */
volatile unsigned int CheckBatteryCounter = 50;
volatile unsigned int ReadNFCDataCounter = READ_NFC_DATA_PERIOD-1;
volatile unsigned int UpdateNFCDataCounter = UPDATE_NFC_DATA_PERIOD-1;


/*
 * We use an LED to monitor device status. In addition to controlling intensity
 * with PWM, we have the LED blink with a very short ON time. There are
 * currently two blinking states, a "stimulation is off" blink at 2 Hz,
 * a "stimulation is on" blink at 1 Hz.
 *
 */
volatile int PowerLEDIntensity = 200; // Out of 1000
volatile unsigned int LEDState = 0;
#define HEARTBEAT_ON 25
unsigned int HeartBeatPattern_StimOff[] = {HEARTBEAT_ON, 1500-HEARTBEAT_ON, HEARTBEAT_ON, 1500-HEARTBEAT_ON}; // (on, off, on, off)
unsigned int HeartBeatPattern_StimOn[] = {HEARTBEAT_ON, 250, HEARTBEAT_ON, 1500 - 2*HEARTBEAT_ON - 250};
unsigned int *HeartBeatPattern;
volatile unsigned int LEDCounter = HEARTBEAT_ON;


void MasterClockSetup(void){
    TA1CTL = TASSEL_2 + MC_1;  // SMCLK (1 MHz), up mode, enable interrupt
    TA1CCR0 = 1000;            // 1 ms period
    TA1CCTL0 = CCIE;           // CCR0 interrupt enabled

    // Initialize GPIO for status LED
    STATUS_LED_PDIR |= STATUS_LED_PIN;    //initialize Status LED pin as output
    STATUS_LED_PSEL |= STATUS_LED_PIN;    //select Status LED pin as PWM
    TA1CCTL2 = OUTMOD_7;           //CCR2 reset/set
    TA1CCR2 = PowerLEDIntensity;   //pin 2.5 brightness (PWM duty cycle versus TA1CCR0)

    HeartBeatPattern = HeartBeatPattern_StimOff;
}

inline void KernelSleep(void) {
    KernelWakeupFlag = 0;
    __bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ interrupts
    KernelWakeupFlag = 1;
}

void main(void)
{
    StimParams_t NewStimParams = { .Enabled = 0, .Period = 7500, \
            .Amplitude = 100, .PulseWidth = 600 };

    int StimParamsChanged = 0;

    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

    // Set up system clocks
    BCSCTL1 = CALBC1_1MHZ;   // Set range
    DCOCTL = CALDCO_1MHZ;    // Set DCO step + modulation
    BCSCTL3 |= LFXT1S_2;     // LFXT1 = VLO (~12 kHz)
    BCSCTL2 = DIVS_0;  // Setup SMCLK to be 1 MHz (divide DCO/1)

    // Default - initialize all ports to output
    P1DIR = 0xFF; P2DIR = 0xFF; P3DIR = 0xFF;
    P1OUT = 0; P2OUT = 0; P3OUT = 0;

    I2CSetup();

    BatteryStatusSetup();

    SetupSwitchMatrix();

    NFCInterfaceSetup();

    MasterClockSetup();

    while (1) {

        if ((RF430InterruptTriggered != 0) || (ReadNFCDataCounter <= 0 )) {
            if (RF430InterruptTriggered != 0) {
                ClearNFCInterrupts();
                RF430InterruptTriggered = 0;
            }
            if (ReadDeviceParams(&NewStimParams) != 0) {
                StimParamsChanged = memcmp((unsigned char*)&NewStimParams,
                        (unsigned char*)&(DeviceData.StimParams), sizeof(NewStimParams));
                if (StimParamsChanged != 0) {
                    DisableStimulation();
                    // NOTE - WE'RE NOT DOING ANY ERROR CHECKING! MAKE SURE PARAMETERS
                    //   ARE ERROR CHECKED BEFORE TRANSMITTING!
                    DeviceData.StimParams.Period = NewStimParams.Period;
                    DeviceData.StimParams.Amplitude = NewStimParams.Amplitude;
                    SetOutputCurrent(DeviceData.StimParams.Amplitude);
                    DeviceData.StimParams.PulseWidth = NewStimParams.PulseWidth;
                    DeviceData.StimParams.JitterLevel = NewStimParams.JitterLevel;
                    DeviceData.StimParams.Enabled = NewStimParams.Enabled;
                    if (NewStimParams.Enabled != 0) {
                        EnableStimulation();
                        HeartBeatPattern = HeartBeatPattern_StimOn;
                    }
                    else {
                        HeartBeatPattern = HeartBeatPattern_StimOff;
                    }
                    DeviceData.Status.LastUpdate = DeviceData.Status.Uptime;
                }
            }
            ReadNFCDataCounter = READ_NFC_DATA_PERIOD-1;
            KernelSleep();
        }
        else if (UpdateNFCDataCounter <= 0) {
            UpdateDeviceStatus();
            UpdateNFCDataCounter = UPDATE_NFC_DATA_PERIOD-1;
        }
        else if (CheckBatteryCounter <= 0) {
            CheckBattery();
            CheckBatteryCounter = CHECK_BATTERY_PERIOD-1;
        }

        KernelSleep();

    }
}


// Timer A1 interrupt service routine => Assume CCR0 set for 1 ms ticks
#pragma vector=TIMER1_A0_VECTOR
__interrupt void MasterClockISR (void)
{
    static unsigned int SecondCounter = 1000;

    if (--SecondCounter == 0) {
        DeviceData.Status.Uptime += 1;
        SecondCounter = 1000;
    }

    // Timer variables for each subprocess that runs in main loop
    CheckBatteryCounter--;
    UpdateNFCDataCounter--;
    ReadNFCDataCounter--;

    // Time variable for blinking LED
    LEDCounter--;
    if (LEDCounter == 0) {
        if ((++LEDState & 0x01) == 0)
            TA1CCR2 = PowerLEDIntensity;   //pin 2.5 brightness (PWM duty cycle versus TA1CCR0)
        else
            TA1CCR2 = 0;   //pin 2.5 brightness (PWM duty cycle versus TA1CCR0)

        LEDState &= 0x03; // faster than checking length
        LEDCounter = HeartBeatPattern[LEDState];
    }

    if (KernelWakeupFlag == 0) { // If the master loop has gone to sleep then wake it up
        __bic_SR_register_on_exit(CPUOFF);
    }

}
