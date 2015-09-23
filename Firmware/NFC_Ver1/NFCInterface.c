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

volatile int RF430InterruptTriggered = 0;

void NFCInterfaceSetup(void) {
    // Reset the RF430 using its reset pin. Normally on power up this wouldn't be necessary,
    // but it's good in case something has happened...
    RF430_RESET_PSEL &= ~RF430_RESET;
    RF430_RESET_PSEL2 &= ~RF430_RESET;
    RF430_RESET_POUT &= ~RF430_RESET; // Reset pin is active low. Reset device
    RF430_RESET_PDIR |= RF430_RESET;
    __delay_cycles(1000); // Wait a bit
    RF430_RESET_POUT |= RF430_RESET; // Release the RF430 to on

    __delay_cycles(100000);

    RF430_INTR_PDIR &= ~RF430_INTR;     // Set INTR pin to input
    RF430_INTR_PREN |= RF430_INTR;     // Enable pullup/down resistor
    RF430_INTR_POUT &= ~RF430_INTR;     // Set pull DOWN resistor
    RF430_INTR_PIES &= ~RF430_INTR;     // Low-to-high edge
    RF430_INTR_PIFG &= ~RF430_INTR;     // IFG cleared
    RF430_INTR_PIE |= RF430_INTR;       // Interrupt enabled
    RF430_INTR_PIFG &= ~RF430_INTR;     // IFG cleared

    InitializeI2CSlave(RF430_ADDRESS);

    while(!(ReadRegister_WordAddress(STATUS_REG) & READY)); //wait until READY bit has been set
    /****************************************************************************/
    /* Errata Fix : Unresponsive RF - recommended firmware                      */
    /****************************************************************************/
    {
        //Please implement this fix as given in this block.  It is important that
        //no line be removed or changed.
        unsigned char version;
        version = ReadRegister_WordAddress(VERSION_REG);  // read the version register.  The fix changes based on what version of the
                                               // RF430 is being used.  Version C and D have the issue.  Next versions are
                                               // expected to have this issue corrected Ver C = 0x01, Ver D = 0x02
        if (version == 0x01 || version == 0x02)
        {   // the issue exists in these two versions
            WriteRegister_WordAddress(0xFFE0, 0x004E);
            WriteRegister_WordAddress(0xFFFE, 0x0080);
            if (version == 0x01)
            {  // Ver C
                WriteRegister_WordAddress(0x2a98, 0x0650);
            }
            else
            {   // Ver D
                WriteRegister_WordAddress(0x2a6e, 0x0650);
            }
            WriteRegister_WordAddress(0x2814, 0);
            WriteRegister_WordAddress(0x2815, 0); // Is this necessary?
            WriteRegister_WordAddress(0xFFE0, 0);
        }
        //Upon exit of this block, the control register is set to 0x0
    }

    //write NDEF memory with full Capability Container + NDEF message
    memcpy(&(NDEF_Data.ND.BinaryMessage), (unsigned char *)&DeviceData, sizeof(DeviceData));
    WriteContinuous_I2C((unsigned char *)(&NDEF_Data), sizeof(NDEF_Data));

    WriteRegister_WordAddress(INTR_ENABLE_REG, END_OF_WRITE);
    WriteRegister_WordAddress(CONTROL_REG, INTO_HIGH + INT_ENABLE + RF_ENABLE);
}

void UpdateDeviceStatus(void) {
    unsigned char statusString_I2C[2 + sizeof(DeviceStatus_t)] = {0x00, STATUS_ADDR};

    InitializeI2CSlave(RF430_ADDRESS);

    if ((ReadRegister_WordAddress(STATUS_REG) & (READY | RF_BUSY | CRC_ACTIVE)) != READY) {
        return;
    }
    else {
        WriteRegister_WordAddress(CONTROL_REG, 0x00); // Disable RF and interrupts
        memcpy(statusString_I2C+2, (unsigned char *)&(DeviceStatus), sizeof(DeviceStatus_t));
        WriteContinuous_I2C(statusString_I2C, sizeof(statusString_I2C));
        WriteRegister_WordAddress(CONTROL_REG, INTO_HIGH + INT_ENABLE + RF_ENABLE);
    }
}

int ReadDeviceParams(StimParams_t *NewStimParams) {
    InitializeI2CSlave(RF430_ADDRESS);

    if ((ReadRegister_WordAddress(STATUS_REG) & (READY | RF_BUSY | CRC_ACTIVE)) != READY) {
        return 0;
    }
    else {
        WriteRegister_WordAddress(CONTROL_REG, 0x00); // Disable RF
        //__delay_cycles(20);
        ReadMemory_WordAddress(STIMPARAMS_ADDR, (unsigned char*)NewStimParams, sizeof(NewStimParams));
        WriteRegister_WordAddress(CONTROL_REG, INTO_HIGH + INT_ENABLE + RF_ENABLE);
        return 1;
    }

}

void ClearNFCInterrupts(void) {
    WriteRegister_WordAddress(INTR_FLAG_REG, END_OF_WRITE); // Clear end of write interrupt
}


// Port 1 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
  P2IFG &= ~RF430_INTR;   // P2.0 IFG cleared
  if (RF430InterruptTriggered == 0)
      RF430InterruptTriggered = 1;
}
