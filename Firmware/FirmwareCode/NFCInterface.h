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
 * This file contains the code which interacts with the RF430CL330H NFC module.
 *
 *
*/
#ifndef RATDBS_FIRMWARE_NFCINTERFACE_H_
#define RATDBS_FIRMWARE_NFCINTERFACE_H_

#include <msp430.h>
#include "stdint.h"
#include "stddef.h"
#include "Firmware.h"

#define RF430_I2C_ADDRESS 0x28


// RF430 register addresses
#define CONTROL_REG        0xFFFE
#define STATUS_REG         0xFFFC
#define VERSION_REG        0xFFEF // NOTE - this points to the version BYTE
#define INTR_ENABLE_REG    0xFFFA
#define INTR_FLAG_REG      0xFFF8

//define the different virtual register bits
//CONTROL_REG bits
#define SW_RESET        BIT0
#define RF_ENABLE       BIT1
#define INT_ENABLE      BIT2
#define INTO_HIGH       BIT3
#define INTO_DRIVE      BIT4
#define BIP8_ENABLE     BIT5
#define STANDBY_ENABLE  BIT6
#define TEST430_ENABLE  BIT7

//STATUS_REG bits
#define READY      BIT0
#define CRC_ACTIVE BIT1
#define RF_BUSY    BIT2

//INTERRUPT_*_REG bits
#define END_OF_WRITE BIT2

struct __attribute__((__packed__)) TLV {
    unsigned char Tag;
    unsigned char Len;
    unsigned char FileID[2];
    unsigned char MaxFileSize[2];
    unsigned char ReadAccess;
    unsigned char WriteAccess;
};

typedef struct __attribute__((__packed__)) CapabilityContainer {
    unsigned char FileID[2];
    unsigned char Len[2];
    unsigned char MappingVersion;
    unsigned char MLe[2];
    unsigned char MLc[2];
    struct __attribute__((__packed__)) TLV TLV;
} CapabilityContainer;

#define DOMAIN "rnel.rice.edu"
#define DATA_TYPE_NAME "rsm"
#define EXTERNAL_RECORD_TYPE_NAME DOMAIN ":" DATA_TYPE_NAME

typedef struct __attribute__((__packed__)) NDEFExternalRecordHeader_t {
    unsigned char FileID[2];
    unsigned char NLen[2]; // Payload length, MSB first
    unsigned char Flags; // MB=1,ME=1,CF=0,SR=1,IL=0,TNF=0x04 => 0xD4
    unsigned char RecordTypeLength; // length of type name
#ifdef LONG_RECORD
    unsigned char PayloadLength[4]; // should just be PayloadLength, if SR=1
#else
    unsigned char PayloadLength;
#endif
    unsigned char TypeName[sizeof(EXTERNAL_RECORD_TYPE_NAME)-1]; // rnel.rice.edu:rsm
} ExternalRecordHeader;

typedef struct __attribute__((__packed__)) NDEF_ExternalRecord_t {
    unsigned char TagApplicationName[7];
    CapabilityContainer CapContainer; // 17 bytes
    ExternalRecordHeader RecordHeader; // header depends on string sizes bytes
    DeviceData_t DeviceData;
} NDEF_ExternalRecord_t;

typedef struct __attribute__((__packed__)) I2C_NDEF_FullRecord_t {
    unsigned char MemoryAddress[2];
    NDEF_ExternalRecord_t ND;
} I2C_NDEF_FullRecord;

typedef struct __attribute__((__packed__)) I2C_DataRecord_t {
    unsigned char MemoryAddress[2];
    ExternalRecordHeader RecordHeader;
    DeviceData_t DevData;
} I2C_DataRecord;

typedef struct __attribute__((__packed__)) I2C_StatusRecord_t {
    unsigned char MemoryAddress[2];
    DeviceStatus_t Status;
} I2C_StatusRecord;

#define EXTERNAL_RECORD_HEADER_ADDR  offsetof(NDEF_ExternalRecord_t, RecordHeader)
#define EXTERNAL_RECORD_DATA_START  offsetof(NDEF_ExternalRecord_t, DeviceData)

#define STIMPARAMS_ADDR offsetof(NDEF_ExternalRecord_t, DeviceData) + offsetof(DeviceData_t, StimParams)
#define STATUS_ADDR offsetof(NDEF_ExternalRecord_t, DeviceData) + offsetof(DeviceData_t, Status)

#define RECORD_LEN_ADDR offsetof(NDEF_ExternalRecord_t, RecordHeader) + offsetof(ExternalRecordHeader, NLen)
#define RECORD_SIZE (sizeof(ExternalRecordHeader) - offsetof(ExternalRecordHeader,Flags)) + sizeof(DeviceData_t)
#define PAYLOAD_LEN_ADDR offsetof(NDEF_ExternalRecord_t, RecordHeader) + offsetof(ExternalRecordHeader, PayloadLength)
#define PAYLOAD_SIZE sizeof(DeviceData_t)


extern volatile int RF430InterruptTriggered;

extern const NDEF_ExternalRecord_t* HWRecordPointer;

void NFCInterfaceSetup(void);
int UpdateDeviceStatus(void);
int ReadDeviceParams(StimParams_t *NewStimParams);
void ClearNFCInterrupts(void);

#endif
