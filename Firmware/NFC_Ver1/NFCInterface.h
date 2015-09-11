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
#ifndef NFCINTERFACE_H_
#define NFCINTERFACE_H_

#include <msp430.h>

#define RF430_ADDRESS 0x28


#define RF430_PORT_OUT P2OUT
#define RF430_PORT_SEL P2SEL
#define RF430_PORT_SEL2 P2SEL2
#define RF430_PORT_DIR P2DIR
#define RF430_RST BIT0

// RF430 register addresses
#define CONTROL_REG        0xFFFE
#define STATUS_REG         0xFFFC
#define VERSION_REG        0xFFEF // NOTE - this points to the version BYTE

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

#define RF430_DEFAULT_DATA      {                                                       \
/*NDEF Tag Application Name*/                                                           \
0x00, 0x00, \
0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01,                                               \
                                                                                        \
/*Capability Container ID*/                                                             \
0xE1, 0x03,                                                                             \
0x00, 0x0F, /* CCLEN */                                                                 \
0x20,       /* Mapping version 2.0 */                                                   \
0x00, 0xF9, /* MLe (49 bytes); Maximum R-APDU data size */                              \
0x00, 0xF6, /* MLc (52 bytes); Maximum C-APDU data size */                              \
0x04,       /* Tag, File Control TLV (4 = NDEF file) */                                 \
0x06,       /* Length, File Control TLV (6 = 6 bytes of data for this tag) */           \
0xE1, 0x04, /* File Identifier */                                                       \
0x0B, 0xDF, /* Max NDEF size (3037 bytes of useable memory) */                          \
0x00,       /* NDEF file read access condition, read access without any security */     \
0x00,       /* NDEF file write access condition; write access without any security */   \
                                                                                        \
/* NDEF File ID */                                                                      \
0xE1, 0x04,                                                                             \
                                                                                        \
/* NDEF File for Hello World  (48 bytes total length) */                                \
0x00, 0x14, /* NLEN; NDEF length (3 byte long message) */                               \
0xD1, /* MB=1, ME=1, CF=0 (not chunked), SR=1 (length is encoded in one byte), No ID length, TNF = 0x01 (NFC Forum Well Known Type) */    \
0x01, /* "Length of the record type */                                               \
0x10,   /* Length of the text record + 'T', 0x02, 'e', 'n' (4) */                    \
0x54, /* TYPE 'T'= text */                                                                  \
0x02, /* Status byte - UTF-8, 2 byte language code */                                   \
0x65, 0x6E, /* 'e', 'n', */                                                             \
                                                                                        \
/* 'Hello, world!' NDEF data; Empty NDEF message, length should match NLEN*/            \
'I','n','i','t','i','a','l','i','z','e','d','.'             \
}

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
    unsigned char NLen[2];
    unsigned char Flags; // MB=1,ME=1,CF=0,SR=1,IL=0,TNF=0x04 => 0xD4
    unsigned char RecordTypeLength; // length of type name
    unsigned char PayloadLength;
    unsigned char TypeName[sizeof(EXTERNAL_RECORD_TYPE_NAME)-1]; // rnel.rice.edu:rsm
} ExternalRecordHeader;

typedef struct __attribute__((__packed__)) NDEF_ExternalRecord_t {
    unsigned char TagApplicationName[7];
    CapabilityContainer CapContainer; // 17 bytes
    ExternalRecordHeader RecordHeader; // header depends on string sizes bytes
    unsigned char BinaryMessage[255];
} NDEF_ExternalRecord_t;

struct __attribute__((__packed__)) I2C_NDEF_FullRecord {
    unsigned char MemoryAddress[2];
    NDEF_ExternalRecord_t ND;
};

#define EXTERNAL_RECORD_DATA_START  7 + sizeof(CapabilityContainer)  + \
    sizeof(ExternalRecordHeader) // - for UnknownRecord

void NFCInterfaceSetup(void);
void UpdateNFC(void);

#endif
