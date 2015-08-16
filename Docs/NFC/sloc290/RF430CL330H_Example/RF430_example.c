/*
 * {RF430_example.c}
 *
 * {Functions}
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
/*
 * RF430_example.c
 *
 *  Created on: Feb 6, 2013
 *      Author: a0273119
 */
#include "msp430.h"
#include "RF430_example.h"

unsigned char RxData[2] = {0,0};
unsigned char TxData[2] = {0,0};
unsigned char TxAddr[2] = {0,0};

// Reads the register at reg_addr, returns the result
unsigned int Read_Register(unsigned int reg_addr)
{
	TxAddr[0] = reg_addr >> 8; 		// MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	// LSB of address

	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = 0x0002;
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[1];
	while(!(UCB0IFG & UCBCNTIFG));
	UCB0CTL1 &= ~UCTR; 				// I2C read operation
	UCB0CTL1 |= UCTXSTT; 			// Repeated start
	while(!(UCB0IFG & UCRXIFG0));
	RxData[0] = UCB0RXBUF;
	UCB0CTLW0 |= UCTXSTP; 			// Send stop after next RX
	while(!(UCB0IFG & UCRXIFG0));
	RxData[1] = UCB0RXBUF;
	while (!(UCB0IFG & UCSTPIFG));  // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

	return RxData[1] << 8 | RxData[0];
}

//reads the register at reg_addr, returns the result
unsigned int Read_Register_BIP8(unsigned int reg_addr)
{
	unsigned char BIP8 = 0;
	TxAddr[0] = reg_addr >> 8; 		// MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	// LSB of address

	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = 0x0002;
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation

	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[0];
	BIP8 ^= TxAddr[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[1];
	BIP8 ^= TxAddr[1];

	while(!(UCB0IFG & UCBCNTIFG));
	UCB0CTL1 &= ~UCTR; 				// I2C read operation
	UCB0CTL1 |= UCTXSTT; 			// Repeated start

	while(!(UCB0IFG & UCRXIFG0));
	RxData[0] = UCB0RXBUF;
	BIP8 ^= RxData[0];
	while(!(UCB0IFG & UCRXIFG0));
	RxData[1] = UCB0RXBUF;
	BIP8 ^= RxData[1];

	UCB0CTLW0 |= UCTXSTP; 			// Send stop after next RX
	while(!(UCB0IFG & UCRXIFG0));	// Receive BIP8
	if(BIP8 != UCB0RXBUF)			// Compare to known value

		__no_operation();			// Breakpoint encase BIP8 doesn't match

	while (!(UCB0IFG & UCSTPIFG));  // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

	return RxData[0] << 8 | RxData[1];
}

//Continuous read data_length bytes and store in the area "read_data"
void Read_Continuous(unsigned int reg_addr, unsigned char* read_data, unsigned int data_length)
{
	unsigned int i;

	TxAddr[0] = reg_addr >> 8; 		// MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	// LSB of address


	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = 0x0002;
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation

		while(!(UCB0IFG & UCTXIFG0));
		UCB0TXBUF = TxAddr[0];
		while(!(UCB0IFG & UCTXIFG0));
		UCB0TXBUF = TxAddr[1];
		while(!(UCB0IFG & UCBCNTIFG));
		UCB0CTL1 &= ~UCTR; 			//i2c read operation
		UCB0CTL1 |= UCTXSTT; 		//repeated start
		while(!(UCB0IFG & UCRXIFG0));

	for(i = 0; i < data_length-1; i++)
	{
		while(!(UCB0IFG & UCRXIFG0));
		read_data[i] = UCB0RXBUF;
		if(i == data_length-1)
			UCB0CTL1 |= UCTXSTP; 	//send stop after next RX
	}

	UCB0CTLW0 |= UCTXSTP; 			//send stop after next RX
	while(!(UCB0IFG & UCRXIFG0));
	read_data[i] = UCB0RXBUF;
	while (!(UCB0IFG & UCSTPIFG)); 	// Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;
}

//writes the register at reg_addr with value
void Write_Register(unsigned int reg_addr, unsigned int value)
{
	TxAddr[0] = reg_addr >> 8; 		// MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	// LSB of address
	TxData[0] = value >> 8;
	TxData[1] = value & 0xFF;

	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = 0x0004;
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation
	//write the address
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[1];
	//write the data
	while(!(UCB0IFG & UCTXIFG0));
	//UCB0TXBUF = TxData[0];
	UCB0TXBUF = TxData[1];
	while(!(UCB0IFG & UCTXIFG0));
	//UCB0TXBUF = TxData[1];
	UCB0TXBUF = TxData[0];
	while (!(UCB0IFG & UCBCNTIFG));
	UCB0CTL1 |= UCTXSTP;
	while (!(UCB0IFG & UCSTPIFG)); 	// Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

}

//writes the register at reg_addr with value
void Write_Register_BIP8(unsigned int reg_addr, unsigned int value)
{
	unsigned char BIP8 = 0;

	TxAddr[0] = reg_addr >> 8; 		//MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	//LSB of address
	TxData[0] = value >> 8;
	TxData[1] = value & 0xFF;

	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = 0x0005;
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		//start i2c write operation

	//write the address
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[0];
	BIP8 ^= TxAddr[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[1];
	BIP8 ^= TxAddr[1];

	//write the data
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxData[0];
	BIP8 ^= TxData[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxData[1];
	BIP8 ^= TxData[1];

	//send BIP8 byte
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = BIP8;

	while (!(UCB0IFG & UCBCNTIFG));
	UCB0CTL1 |= UCTXSTP;
	while (!(UCB0IFG & UCSTPIFG));     // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

}

//writes the register at reg_addr and incrementing addresses with the data at "write_data" of length data_length
void Write_Continuous(unsigned int reg_addr, unsigned char* write_data, unsigned int data_length)
{
	unsigned int i;

	TxAddr[0] = reg_addr >> 8; 		//MSB of address
	TxAddr[1] = reg_addr & 0xFF; 	//LSB of address

	UCB0CTLW1 = UCASTP_1;
	UCB0TBCNT = data_length; 		//data_length is in words, TBCNT is in bytes, so multiply by 2
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		//start i2c write operation
	//write the address
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[0];
	while(!(UCB0IFG & UCTXIFG0));
	UCB0TXBUF = TxAddr[1];
	//write the data
	for(i = 0; i < data_length; i++)
	{
		while(!(UCB0IFG & UCTXIFG0));
		UCB0TXBUF = write_data[i];
	}

	while(!(UCB0IFG & UCTXIFG0));
	while (!(UCB0IFG & UCBCNTIFG));
	UCB0CTL1 |= UCTXSTP;
	while (!(UCB0IFG & UCSTPIFG));     // Ensure stop condition got sent
	UCB0CTL1  |= UCSWRST;

}
