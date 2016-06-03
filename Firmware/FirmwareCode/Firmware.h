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
 * Firmware.h
 *
 *  Constants and constant strings for firmware
 *
 */

#ifndef RATDBS_FIRMWARE_FIRMWARE_H_
#define RATDBS_FIRMWARE_FIRMWARE_H_

#include "stdint.h"

/* Program coordination variables */
extern volatile unsigned char KernelWakeupFlag;                         // status variable

#define PROTOCOL_VERSION 5
#define DEFAULT_DEVICE_IDSTR "0005"

/* Device ID variables */
typedef struct __attribute__((__packed__)) DeviceID_t {
    uint8_t firmwareVersion;
    char idStr[4];
} DeviceID_t;

/* Stimulation parameters *
 *  - These are accessed in the stimulation code, but set in the communcation code
 */
typedef struct __attribute__((__packed__)) StimParams_t {
    uint8_t Enabled;
    uint16_t Period;
    uint8_t Amplitude;
    uint16_t PulseWidth;
    uint8_t JitterLevel;
} StimParams_t;

/* Device status variables */
typedef struct __attribute__((__packed__)) DeviceStatus_t {
    uint16_t BatteryVoltage;
    uint32_t Uptime;
    uint32_t LastUpdate;
} DeviceStatus_t;


typedef struct __attribute__((__packed__)) DeviceData_t {
    DeviceID_t ID;
    StimParams_t StimParams;
    DeviceStatus_t Status;
} DeviceData_t;

extern volatile DeviceData_t DeviceData;

/*
 * Our main loop currently runs three different processes, (1) checking
 * the battery power level, (2) checking for and responding to parameter
 * changes via NFC, and (3) writing status information to the NFC chip.
 * the PERIOD definition determines how regularly these processes take
 * place (in ms).
 *
 */
#define READ_NFC_DATA_PERIOD 30033 // 30 s (and when triggered by interrupt!)
#define CHECK_BATTERY_PERIOD 20000 // 20 s
#define UPDATE_NFC_DATA_PERIOD 10000 // every 10 s

#endif /* RATDBS_FIRMWARE_FIRMWARE_H_ */
