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

#include "NFCInterface.h"
#include "Firmware.h"
#include "I2C.h"
#include "stdint.h"

struct I2C_NDEF_FullRecord NDEF_Data = { \
        .MemoryAddress = {0x00, 0x00}, \
        .ND.TagApplicationName={0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01}, \
        .ND.CapContainer.FileID = {0xE1, 0x03}, \
        .ND.CapContainer.Len = {0x00, 0x0F}, \
        .ND.CapContainer.MappingVersion = 0x20, \
        .ND.CapContainer.MLe = {0x00, 0xF9}, \
        .ND.CapContainer.MLc = {0x00, 0xF6}, \
        .ND.CapContainer.TLV.Tag = 0x04, \
        .ND.CapContainer.TLV.Len = 0x06, \
        .ND.CapContainer.TLV.FileID = {0xE1, 0x04}, \
        .ND.CapContainer.TLV.MaxFileSize = {0x0B, 0xDF}, \
        .ND.CapContainer.TLV.ReadAccess = 0x00, \
        .ND.CapContainer.TLV.WriteAccess = 0x00, \
        .ND.RecordHeader.FileID = {0xE1, 0x04}, \
        .ND.RecordHeader.NLen = {0x00, 3 + \
                sizeof(EXTERNAL_RECORD_TYPE_NAME) - 1 + \
                sizeof(DeviceData_t)}, \
        .ND.RecordHeader.Flags = 0xD4,\
        .ND.RecordHeader.RecordTypeLength = sizeof(EXTERNAL_RECORD_TYPE_NAME) - 1, \
        .ND.RecordHeader.PayloadLength = sizeof(DeviceData_t), \
        .ND.RecordHeader.TypeName = EXTERNAL_RECORD_TYPE_NAME, \
};


#define STATUS_START EXTERNAL_RECORD_DATA_START + sizeof(DeviceID_t) + sizeof(StimParams_t)
unsigned char statusString_I2C[sizeof(DeviceStatus_t)] = {0x00, STATUS_START};

void NFCInterfaceSetup(void) {
    // Reset the RF430 using its reset pin. Normally on power up this wouldn't be necessary,
    // but it's good in case something has happened...
    RF430_PORT_SEL &= ~RF430_RST;
    RF430_PORT_SEL2 &= ~RF430_RST;
    RF430_PORT_OUT &= ~RF430_RST; // Reset pin is active low. Reset device
    RF430_PORT_DIR |= RF430_RST;
    __delay_cycles(1000); // Wait a bit
    RF430_PORT_OUT |= RF430_RST; // Release the RF430 to on

    __delay_cycles(100000);
    // Should set up interrupt here?

    while(!(ReadRegister_WordAddress(RF430_ADDRESS, STATUS_REG) & READY)); //wait until READY bit has been set
    /****************************************************************************/
    /* Errata Fix : Unresponsive RF - recommended firmware                      */
    /****************************************************************************/
    {
        //Please implement this fix as given in this block.  It is important that
        //no line be removed or changed.
        unsigned char version;
        version = ReadRegister_WordAddress(RF430_ADDRESS, VERSION_REG);  // read the version register.  The fix changes based on what version of the
                                               // RF430 is being used.  Version C and D have the issue.  Next versions are
                                               // expected to have this issue corrected Ver C = 0x01, Ver D = 0x02



        if (version == 0x01 || version == 0x02)
        {   // the issue exists in these two versions
            WriteRegister_WordAddress(RF430_ADDRESS, 0xFFE0, 0x004E);
            WriteRegister_WordAddress(RF430_ADDRESS, 0xFFFE, 0x0080);
            if (version == 0x01)
            {  // Ver C
                WriteRegister_WordAddress(RF430_ADDRESS, 0x2a98, 0x0650);
            }
            else
            {   // Ver D
                WriteRegister_WordAddress(RF430_ADDRESS, 0x2a6e, 0x0650);
            }
            WriteRegister_WordAddress(RF430_ADDRESS, 0x2814, 0);
            WriteRegister_WordAddress(RF430_ADDRESS, 0x2815, 0); // Is this necessary?
            WriteRegister_WordAddress(RF430_ADDRESS, 0xFFE0, 0);
        }
        //Upon exit of this block, the control register is set to 0x0
    }

    //write NDEF memory with full Capability Container + NDEF message
    memcpy(&(NDEF_Data.ND.BinaryMessage), &DeviceData, sizeof(DeviceData));
    WriteContinuous_I2C(RF430_ADDRESS, (unsigned char *)(&NDEF_Data), sizeof(NDEF_Data));
    WriteRegister_WordAddress(RF430_ADDRESS, CONTROL_REG, RF_ENABLE);
}

const char byteHexTable[] =
    {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

inline void ByteToHexString(unsigned char x, unsigned char *x_e) {
    *x_e-- = byteHexTable[x && 0xF];
    *x_e = byteHexTable[x >> 4];
}

inline void WordToHexString(uint16_t x, unsigned char *x_e) {
    *x_e-- = *(byteHexTable + (x && 0xF));
    *x_e-- = *(byteHexTable + ((x >> 4) && 0xF));
    *x_e-- = *(byteHexTable + ((x >> 8) && 0xF));
    *x_e = *(byteHexTable + (x >> 12));
}

inline void DoubleWordToHexString(uint32_t x, unsigned char *x_e) {
    uint16_t x1, x2;

    x1 = x && 0xFFFF;
    x2 = x >> 16;
    WordToHexString(x2,x_e - 4);
    WordToHexString(x1,x_e);
}

void UpdateNFC(void) {
    static unsigned char *uptimeString = statusString_I2C + 2;

    if ((ReadRegister_WordAddress(RF430_ADDRESS, STATUS_REG) & (READY | RF_BUSY | CRC_ACTIVE)) != READY) {
        return;
    }
    else {
        WriteRegister_WordAddress(RF430_ADDRESS, CONTROL_REG, 0x00); // Disable RF
        //WordToHexString((uint16_t) Uptime, uptimeString + 8);
        memcpy(uptimeString, &(DeviceData.Status), sizeof(DeviceStatus_t));
        WriteContinuous_I2C(RF430_ADDRESS, statusString_I2C, sizeof(statusString_I2C));
        WriteRegister_WordAddress(RF430_ADDRESS, CONTROL_REG, RF_ENABLE);
    }
}

