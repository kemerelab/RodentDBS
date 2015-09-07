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
 * Resources:
 *  - Uses watchdog timer for general program state control
 *  - Uses Timer A0 for stimulation pulse timing
 *  - Uses PWM/Timer A1 module for display light (and Pin 2.5)
 *  - Uses Pins 1.0-1.3 for control of switch array
 *  - Uses Pins 1.6, 1.7 / USCI_A0 for I2C ()
 *
*/

#ifndef BATTERYSTATUS_H_
#define BATTERYSTATUS_H_

#include <msp430.h>
#include "Firmware.h"

extern int BatteryVoltage;
extern long int BatteryStatusTickCounter;

typedef enum {ASLEEP, STARTING_REF, SAMPLING} batteryStatusStateEnum;
extern batteryStatusStateEnum BatteryStatusState;

void BatteryStatusSetup(void);	// Setup ADC10 and initialize values

void ExecuteBatteryStatus(void);

#endif

