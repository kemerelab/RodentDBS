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
const unsigned char NDEF_Application_Data[] = RF430_DEFAULT_DATA;

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
		{	// the issue exists in these two versions
			WriteRegister_WordAddress(RF430_ADDRESS, 0xFFE0, 0x004E);
			WriteRegister_WordAddress(RF430_ADDRESS, 0xFFFE, 0x0080);
			if (version == 0x01)
			{  // Ver C
				WriteRegister_WordAddress(RF430_ADDRESS, 0x2a98, 0x0650);
			}
			else
			{	// Ver D
				WriteRegister_WordAddress(RF430_ADDRESS, 0x2a6e, 0x0650);
			}
			WriteRegister_WordAddress(RF430_ADDRESS, 0x2814, 0);
			WriteRegister_WordAddress(RF430_ADDRESS, 0x2815, 0); // Is this necessary?
			WriteRegister_WordAddress(RF430_ADDRESS, 0xFFE0, 0);
		}
		//Upon exit of this block, the control register is set to 0x0
	}

    //write NDEF memory with Capability Container + NDEF message
    WriteContinuous_I2C(RF430_ADDRESS, NDEF_Application_Data, sizeof(NDEF_Application_Data));
    WriteRegister_WordAddress(RF430_ADDRESS, CONTROL_REG, RF_ENABLE);

}

