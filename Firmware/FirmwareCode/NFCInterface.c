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
#include "Board.h"
#include "I2C.h"
#include "stdint.h"

I2C_NDEF_FullRecord NDEF_Data = {
        .MemoryAddress = {0x00, 0x00},
        .ND.TagApplicationName={0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01},
        .ND.CapContainer.FileID = {0xE1, 0x03},
        .ND.CapContainer.Len = {0x00, 0x0F},
        .ND.CapContainer.MappingVersion = 0x20,
        .ND.CapContainer.MLe = {0x00, 0xF9},
        .ND.CapContainer.MLc = {0x00, 0xF6},
        .ND.CapContainer.TLV.Tag = 0x04,
        .ND.CapContainer.TLV.Len = 0x06,
        .ND.CapContainer.TLV.FileID = {0xE1, 0x04},
        .ND.CapContainer.TLV.MaxFileSize = {0x0B, 0xDF},
        .ND.CapContainer.TLV.ReadAccess = 0x00,
        .ND.CapContainer.TLV.WriteAccess = 0x00,
        .ND.RecordHeader.FileID = {0xE1, 0x04},
        .ND.RecordHeader.NLen = {RECORD_SIZE >> 8, RECORD_SIZE & 0xFF},
        .ND.RecordHeader.RecordTypeLength = sizeof(EXTERNAL_RECORD_TYPE_NAME) - 1,
#ifdef LONG_RECORD
        .ND.RecordHeader.Flags = 0xC4, // SR = 0
        .ND.RecordHeader.PayloadLength = {0,0,PAYLOAD_SIZE >> 8,PAYLOAD_SIZE & 0xFF},
#else
        .ND.RecordHeader.Flags = 0xD4, // SR = 1
        .ND.RecordHeader.PayloadLength = PAYLOAD_SIZE,
#endif
        .ND.RecordHeader.TypeName = EXTERNAL_RECORD_TYPE_NAME
};



volatile int RF430InterruptTriggered = 0;

void NFCInterfaceSetup(void) {

    // Reset the RF430 using its reset pin. Normally on power up this wouldn't be necessary,
    // but it's good in case something has happened...
    RF430_RESET_PSEL &= ~RF430_RESET_PIN;
    RF430_RESET_PSEL2 &= ~RF430_RESET_PIN;
    RF430_RESET_POUT &= ~RF430_RESET_PIN; // Reset pin is active low. Reset device
    RF430_RESET_PDIR |= RF430_RESET_PIN;
    __delay_cycles(1000); // Wait a bit
    RF430_RESET_POUT |= RF430_RESET_PIN; // Release the RF430 to on

    __delay_cycles(100000); // Wait for RF430 to boot

    RF430_INTR_PDIR &= ~RF430_INTR_PIN;     // Set INTR pin to input
    RF430_INTR_PREN |= RF430_INTR_PIN;     // Enable pullup/down resistor
    RF430_INTR_POUT &= ~RF430_INTR_PIN;     // Set pull DOWN resistor
    RF430_INTR_PIES &= ~RF430_INTR_PIN;     // Low-to-high edge
    RF430_INTR_PIFG &= ~RF430_INTR_PIN;     // IFG cleared
    RF430_INTR_PIE |= RF430_INTR_PIN;       // Interrupt enabled
    RF430_INTR_PIFG &= ~RF430_INTR_PIN;     // IFG cleared

    InitializeI2CSlave(RF430_I2C_ADDRESS);

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
    NDEF_Data.ND.DeviceData = DeviceData;
//    memcpy(&(NDEF_Data.ND.BinaryMessage), (unsigned char *)&DeviceData, sizeof(DeviceData));
    WriteContinuous_I2C((unsigned char *)(&NDEF_Data), sizeof(I2C_NDEF_FullRecord));

    WriteRegister_WordAddress(INTR_ENABLE_REG, END_OF_WRITE);
    WriteRegister_WordAddress(CONTROL_REG, INTO_HIGH + INT_ENABLE + RF_ENABLE);
}


void ClearNFCInterrupts(void) {
    WriteRegister_WordAddress(INTR_FLAG_REG, END_OF_WRITE); // Clear end of write interrupt
}

int ReadDeviceParams(StimParams_t *NewStimParams) {
    InitializeI2CSlave(RF430_I2C_ADDRESS);

    if ((ReadRegister_WordAddress(STATUS_REG) & (READY | RF_BUSY | CRC_ACTIVE)) != READY) {
        return -1;
    }
    else {
        WriteRegister_WordAddress(CONTROL_REG, 0x00); // Disable RF
        //__delay_cycles(20);
        i2c_debug_flag = 1;
        ReadMemory_WordAddress(STIMPARAMS_ADDR, (unsigned char*)NewStimParams, sizeof(StimParams_t));
        WriteRegister_WordAddress(CONTROL_REG, INTO_HIGH + INT_ENABLE + RF_ENABLE);
        return 0;
    }

}

I2C_StatusRecord Status_I2C = {.MemoryAddress = {0x00, STATUS_ADDR}};

int UpdateDeviceStatus(void) {

    Status_I2C.Status = DeviceData.Status;
    InitializeI2CSlave(RF430_I2C_ADDRESS);
//    memcpy(&(DataRecord_I2C.DevData), (unsigned char *)&(DeviceData), sizeof(DeviceData_t));
//    memcpy(statusString_I2C+2, (unsigned char *)&(DeviceData.Status), sizeof(DeviceStatus_t));

    if ((ReadRegister_WordAddress(STATUS_REG) & (READY | RF_BUSY | CRC_ACTIVE)) != READY) {
        return -1;
    }
    else {
        WriteRegister_WordAddress(CONTROL_REG, 0x00); // Disable RF and interrupts
        WriteContinuous_I2C((unsigned char *)&Status_I2C, sizeof(I2C_StatusRecord));
        WriteRegister_WordAddress(CONTROL_REG, INTO_HIGH + INT_ENABLE + RF_ENABLE);
        return 0;
    }
}

// Port 1 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
  P2IFG &= ~RF430_INTR_PIN;   // P2.0 IFG cleared
  if (RF430InterruptTriggered == 0)
      RF430InterruptTriggered = 1;
}
