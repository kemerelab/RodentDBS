/*
 * {main.c}
 *
 * {main application}
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/
//******************************************************************************
//  RF430 Tester (Host/Master)
//
//  RF430 User Address Map
//  -----------------------------------------
//  Address		| Size	| Description		|
//  -----------------------------------------
//  0xFFFE		| 2B	| Control Register	|
//  0xFFFC		| 2B	| Status Register	|
//  0xFFFA		| 2B	| Interrupt Enable	|
//  0xFFF8		| 2B	| Interrupt Flags	|
//  0xFFF6		| 2B	| CRC Result		|
//  0xFFF4		| 2B	| CRC Length		|
//  0xFFF2		| 2B	| CRC Start Address	|
//  0xFFF0		| 2B	| Comm WD Ctrl Reg	|
//  -----------------------------------------
//  0x0000 - 	| 2kB	| NDEF App Memory	|
//    0x07FF	|		|					|
//
//  Description: This demo demonstrates RF430 read/write accesses via the I2C bus.
//	This is the code for the tester/host processor, which is the MASTER. This master
//  can send read and write commands over I2C to access the data structure in the
//	RF430 device. The following formats are used:
//
//	Write Access:
//	The master transmits a 2 byte address, then transmits N 16 bit values MSB first
//	to be stored by RF430 in locations starting at this address and incrementing.
//
//	Read Access:
//	The master transmits a 2 byte address, then receives back from RF430
//  the two bytes RF430 has stored at this location, MSB first.
//
//	Continuous Read Access:
//	The master receives the N bytes stored in RF430 starting at the next address
//	(next address = the last accessed address + 2) and incrementing, MSB first.
//
//
//                                /|\  /|\	  (Host/Tester)
//                   RF430        10k  10k     MSP430FR5739
//                  (Slave)        |    |        Master
//             _________________   |    |   _________________
//            |              SDA|<-|----+->|P1.6/UCB0SDA  XIN|-+
//            |                 |  | I2C   |                 |
//            |              SCL|<-+------>|P1.7/UCB0SCL XOUT|-
//            |                 |          |                 |
//      GND<--|E(2-0)       /RST|<---------|P3.7         P4.0|<----S1
//            |             INTO|--------->|P2.0         P4.1|<----S2
//            |                 |          |                 |
//            |                 |          |           P3.4-6|---->LED5-7
//            |                 |          |                 |
//            |_________________|          |_________________|
//
// Built with CCSv5.2
//******************************************************************************
#include "msp430.h"
#include "RF430_example.h"

unsigned char NDEF_Application_Data[] = RF430_DEFAULT_DATA;

unsigned char test_data[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};

unsigned char CRC_Data[] = {1,2,3,4,5,6,7,8,9};

unsigned char Cmd = 0;	//Command byte for SPI mode
unsigned char read_complete = 0;
unsigned char rx_byte_count = 0;
unsigned char tx_byte_count = 0;
unsigned int Results[11] = {0,0,0,0,0,0,0,0,0,0,0};

/****************************************************************************/
/* Code-binary that opens on ETW and retrims LF oscillator to below 280kHz  */
/****************************************************************************/
unsigned char func_retrim_osc[] = {
        0xB2, 0x40, 0x11, 0x96, 0x10, 0x01,
        0xB2, 0x40, 0x60, 0x03, 0x18, 0x01,
        0x30, 0x41
};

unsigned char button_pressed = 0;
unsigned char into_fired = 0;

void main (void)
{
	volatile unsigned int test = 0;
	volatile unsigned int flags = 0;
	//unsigned char read_data[200];

    WDTCTL = WDTPW + WDTHOLD;				// Turn off Watch Dog

    CSCTL0_H = 0xA5;
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;          // Set max. DCO setting = 8MHz
    CSCTL2 = SELA_3 + SELS_3 + SELM_3;      // set ACLK = MCLK = DCO
    CSCTL3 = DIVA_3 + DIVS_0 + DIVM_0;      // set MCLK = SMCLK = 8MHz

    // Configure pins for I2C
	PORT_I2C_SEL0 &= ~(SCL + SDA);
    PORT_I2C_SEL1 |= SCL + SDA;

    //configure eUSCI for I2C
	UCB0CTL1 |= UCSWRST;	            			//Software reset enabled
	UCB0CTLW0 |= UCMODE_3  + UCMST + UCSYNC + UCTR;	//I2C mode, Master mode, sync, transmitter
	UCB0CTLW0 |= UCSSEL_2;                    		// SMCLK = 8MHz

	UCB0BRW = 40; 						// Baudrate = SMLK/40 = 200kHz

	UCB0I2CSA  = 0x0028;				//slave address - determined by pins E0, E1, and E2 on the RF430CL330H
	UCB0CTL1  &= ~UCSWRST;

	//RST RF430 (in case board is still powered but the MSP430 reset for some reason - MSP430 RST button pushed for example)
	PORT_RST_SEL0 &= ~RST;
	PORT_RST_SEL1 &= ~RST;
	PORT_RST_OUT &= ~RST; 				//RF430CL330H device in reset
	PORT_RST_DIR |= RST;
	__delay_cycles(1000);
	PORT_RST_OUT |= RST; 				//release the RF430CL330H

	//configure LEDs
	PORT_LED_SEL0 &= ~(LED5 + LED6 + LED7); 	//GPIO
	PORT_LED_SEL1 &= ~(LED5 + LED6 + LED7); 	//GPIO
	PORT_LED_DIR |= LED5 + LED6 + LED7; 		//output
	PORT_LED_OUT &= ~(LED5 + LED6 + LED7); 		//start out off

	//configure button S1 for interrupts
	PORT_BUTTON_SEL0 &= ~S1; 		//GPIO
	PORT_BUTTON_SEL1 &= ~S1; 		//GPIO
	PORT_BUTTON_DIR &= ~S1; 		//input
	PORT_BUTTON_OUT |= S1; 			//output high for pullup
	PORT_BUTTON_REN |= S1; 			//internal pullup resistor
	PORT_BUTTON_IES |= S1; 			//fire interrupt on high-to-low transition
	PORT_BUTTON_IFG &= ~S1; 		//clear any pending flags
	PORT_BUTTON_IE |= S1; 			//enable interrupt

	//configure pin for INTO interrupts
	PORT_INTO_SEL0 &= ~INTO; 		//GPIO
	PORT_INTO_SEL1 &= ~INTO; 		//GPIO
	PORT_INTO_DIR &= ~INTO; 		//input
	PORT_INTO_OUT |= INTO; 			//output high for pullup
	PORT_INTO_REN |= INTO; 			//internal pullup resistor
	PORT_INTO_IFG &= ~INTO; 		//clear interrupt flag
	PORT_INTO_IES |= INTO; 			//fire interrupt on high-to-low transition since INTO is setup active low

	__delay_cycles(1000000); 		//leave time for the RF430CL33H to get itself initialized

	while(!(Read_Register(STATUS_REG) & READY)); //wait until READY bit has been set

	/****************************************************************************/
    /* Errata Fix : Unresponsive RF - recommended firmware                      */
    /****************************************************************************/
	{
		//Please implement this fix as given in this block.  It is important that
		//no line be removed or changed.
		unsigned int version;
		version = Read_Register(VERSION_REG);  // read the version register.  The fix changes based on what version of the
											   // RF430 is being used.  Version C and D have the issue.  Next versions are
											   // expected to have this issue corrected
											   // Ver C = 0x0101, Ver D = 0x0201
		if (version == 0x0101 || version == 0x0201)
		{	// the issue exists in these two versions
			Write_Register(0xFFE0, 0x004E);
			Write_Register(0xFFFE, 0x0080);
			if (version == 0x0101)
			{  // Ver C
				Write_Register(0x2a98, 0x0650);
			}
			else
			{	// Ver D
				Write_Register(0x2a6e, 0x0650);
			}
			Write_Register(0x2814, 0);
			Write_Register(0xFFE0, 0);
		}
		//Upon exit of this block, the control register is set to 0x0
	}


	/****************************************************************************/
    /* Configure RF430CL330H for Typical Usage Scenario                         */
    /****************************************************************************/

    //write NDEF memory with Capability Container + NDEF message
    Write_Continuous(0, NDEF_Application_Data, 48);

    //Enable interrupts for End of Read and End of Write
    Write_Register(INT_ENABLE_REG, EOW_INT_ENABLE + EOR_INT_ENABLE);

    //Configure INTO pin for active low and enable RF
    Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE + RF_ENABLE);

    __delay_cycles(1000);

	PORT_INTO_IFG &= ~INTO; //clear any pending flags
	PORT_INTO_IE |= INTO;	//enable interrupt

    while (1)
    {

    	__bis_SR_register(LPM3_bits + GIE); //go to low power mode and enable interrupts. Here we are waiting for an RF read or write
    	__no_operation();

    	//device has woken up, check status
    	if(into_fired)
    	{

    		//before we read/write to RF430CL330H we should disable RF
    		Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE); //clear control reg to disable RF
    		__delay_cycles(750000);
    		flags = Read_Register(INT_FLAG_REG); //read the flag register to check if a read or write occurred
    		Write_Register(INT_FLAG_REG, EOW_INT_FLAG + EOR_INT_FLAG); //ACK the flags to clear

			PORT_LED_OUT &= ~(LED6 + LED7);		//clear LEDs

    		if(flags & EOW_INT_FLAG) 			//check if the tag was written
    		{
    			//tag was updated, so we should read out the new data
    			//read out the data
    			//Read_Continuous(0, read_data, 200);
   			    __no_operation(); //breakpoint here to examine the data

    			//show that tag was written with LEDs
    			PORT_LED_OUT |= LED6;
    			__delay_cycles(9000000);
    			PORT_LED_OUT &= ~(LED6 + LED7);//clear LEDs
    		}
    		else if(flags & EOR_INT_FLAG) //check if the tag was read
    		{
    			__no_operation();

    			//show that tag was read with LEDs
    			PORT_LED_OUT |= LED7;
    			__delay_cycles(9000000);
    			PORT_LED_OUT &= ~(LED6 + LED7);//clear LEDs
    		}

    		flags = 0;
    		into_fired = 0; //we have serviced INTO

    		//Enable interrupts for End of Read and End of Write
    		 Write_Register(INT_ENABLE_REG, EOW_INT_ENABLE + EOR_INT_ENABLE);

			//Configure INTO pin for active low and re-enable RF
			Write_Register(CONTROL_REG, INT_ENABLE + INTO_DRIVE + RF_ENABLE);

    		//re-enable INTO
    		PORT_INTO_IFG &= ~INTO;
    		PORT_INTO_IE |= INTO;

			__no_operation();
    	}
    	if(button_pressed)
    	{

    		button_pressed = 0; //we have serviced the button press
    	}

    }
}


#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
	//INTO interrupt fired
	if(PORT_INTO_IFG & INTO)
	{

		into_fired = 1;

		PORT_INTO_IE &= ~INTO; //disable INTO
		PORT_INTO_IFG &= ~INTO; //clear interrupt flag

		__bic_SR_register_on_exit(LPM3_bits); //wake up to handle INTO
	}
}

#pragma vector=PORT4_VECTOR
__interrupt void PORT4_ISR(void)
{
	//BUTTON interrupt fired
	if(PORT_BUTTON_IFG & S1)
	{
		button_pressed = 1;
		PORT_BUTTON_IFG &= ~S1; //clear interrupt flag
		__bic_SR_register_on_exit(LPM3_bits + GIE); //wake up to handle button press
	}
}

//trap ISRs for unused interrupts
#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void)
{
	while(1); //trap ISR
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	while(1); //trap ISR
}




