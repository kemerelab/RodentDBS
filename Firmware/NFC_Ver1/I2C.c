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
 * This file contains the I2C interface code and variables
 *
 *
*/

#include <msp430.h>
#include <stdint.h>
#include "I2C.h"

unsigned char* _i2c_tx_data;
unsigned int _i2c_byte_count;

inline void I2CSetup (void)
{
	P1SEL  |= BIT6 + BIT7;                    // Assign I2C pins to USCI_B0
	P1SEL2 |= BIT6 + BIT7;                    // Assign I2C pins to USCI_B0
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
	UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	UCB0BR0 = 10;                             // fSCL = SMCLK(1MHz)/10 = ~400kHz
	UCB0BR1 = 0;
}

// Write a byte to a register with a 2 byte address
void WriteRegister_WordAddress(char slave_addr, uint16_t reg_addr, uint16_t reg_value)
{
	unsigned char data[4];
	data[0] = reg_addr >> 8;
	data[1] = reg_addr & 0xFF;
	data[2] = reg_value & 0xFF;
	data[3] = reg_value >> 8;

	UCB0I2CSA = slave_addr;    // Slave Address
	UCB0CTL1  &= ~UCSWRST;
	IE2 |= UCB0TXIE;                          // Enable TX interrupt

	_i2c_tx_data = data;                    // TX array start address
    _i2c_byte_count = sizeof data;          // Load TX byte counter
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
                                            // Remain in LPM0 until all data
                                            // is TX'd

	IE2 &= ~UCB0TXIE;    // Disable TX interrupt
	//UCB0CTL1 |= UCSWRST; // Turn off i2c
}


//writes a byte string to I2C slave_addr of length data_length
void WriteContinuous_I2C(char slave_addr, uint16_t memory_addr, unsigned char* write_data, unsigned int data_length) {
	UCB0I2CSA = slave_addr;    // Slave Address
	UCB0CTL1  &= ~UCSWRST;
	IE2 |= UCB0TXIE;                          // Enable TX interrupt

    _i2c_tx_data = write_data;      // TX array start address
    _i2c_byte_count = data_length;              // Load TX byte counter
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
                                            // Remain in LPM0 until all data
                                            // is TX'd
	IE2 &= ~UCB0TXIE;    // Disable TX interrupt
	//UCB0CTL1 |= UCSWRST; // Turn off i2c
}

//------------------------------------------------------------------------------
// The USCIAB0TX_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with the byte count. Also, TXData
// points to the next byte to transmit.
//------------------------------------------------------------------------------
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  if (_i2c_byte_count)                            // Check TX byte counter
  {
    UCB0TXBUF = *_i2c_tx_data++;                 // Load TX buffer
    _i2c_byte_count--;                            // Decrement TX byte counter
  }
  else
  {
    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
    IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
    __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
  }
}

//------------------------------------------------------------------------------
// The USCIAB0TX_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with the byte count. Also, TXData
// points to the next byte to transmit.
//------------------------------------------------------------------------------

// Read a byte from a register with a 2 byte address
unsigned char ReadRegister_WordAddress(char slave_addr, uint16_t reg_addr)
{
	unsigned char reg_value;

	UCB0I2CSA = slave_addr;    // Slave Address
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation
	//while(!(UCB0CTL1 & UCTXSTT)); // Assert that start condition was started
	while((UC0IFG & UCB0TXIFG) == 0); // This signifies successful start condition

	UCB0TXBUF = reg_addr >> 8; // Load the first address byte to i2c data
	while((UC0IFG & UCB0TXIFG) == 0); // Wait until data is sent

	UCB0TXBUF = reg_addr & 0xFF; // Load the second address byte
	while((UC0IFG & UCB0TXIFG) == 0); // Wait until data is sent

	UCB0CTL1 &= ~UCTR; // Change to read mode and...
	UCB0CTL1 |= UCTXSTT;		// set repeated start
	while(!(UCB0CTL1 & UCTXSTT)); // Assert that start condition was started

	UCB0CTL1 |= UCTXSTP; // Set stop condition - necessary because single byte read

	while((UC0IFG & UCB0RXIFG) == 0); // Wait until data is received
	reg_value = UCB0RXBUF;
	UCB0CTL1 |= UCSWRST; // Turn off i2c

	return reg_value;
}


// Read a series of bytes from a memory starting at a 2 byte address
void ReadMemory_WordAddress(char slave_addr, uint16_t reg_addr,
		unsigned char* data, int byte_count)
{
	unsigned int i;

	UCB0I2CSA = slave_addr;    // Slave Address
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation

	UCB0TXBUF = reg_addr >> 8; // Load the first address byte
	while(!(UCB0CTL1 & UCTXSTT)); // Assert that start condition was started
    while((UC0IFG & UCB0TXIFG) == 0); // Wait until data is sent
	UCB0TXBUF = reg_addr & 0xFF; // Load the second address byte
	while((UC0IFG & UCB0TXIFG) == 0); // Wait until data is sent
	UCB0CTL1 &= ~UCTR; // Change to read mode and...
	UCB0CTL1 |= UCTXSTT;		// set repeated start
	for (i = 0; i < byte_count; i++) {
		while(!(UC0IFG & UCB0RXIFG)); // Wait until data is received
		data[i] = UCB0RXBUF;
	}
	UCB0CTL1 |= UCTXSTP; // Set stop condition
	UCB0CTL1 |= UCSWRST; // Turn off i2c
}



// Write a byte to a register with a 1 byte address
void WriteRegister_ByteAddress(char slave_addr,
		unsigned char reg_addr, unsigned char reg_value)
{
	//WriteContinuousBytes_I2C(slave_addr, TxData, 2);

	UCB0I2CSA = slave_addr;    // Slave Address
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation

	UCB0TXBUF = reg_addr; // Load the first byte of i2c data
	while(!(UCB0CTL1 & UCTXSTT)); // Assert that start condition was started
    while((UC0IFG & UCB0TXIFG) == 0); // Wait until data is sent
	UCB0TXBUF = reg_value; // Load the next byte of data
	while((UC0IFG & UCB0TXIFG) == 0); // Wait until data is sent
	UCB0CTL1 |= UCTXSTP; // Set stop condition
	UCB0CTL1 |= UCSWRST; // Turn off i2c
}

// Read a byte from a register with a 1 byte address
unsigned char ReadRegister_ByteAddress(char slave_addr,
		unsigned char reg_addr)
{
	unsigned char reg_value;

	UCB0I2CSA = slave_addr;    // Slave Address
	UCB0CTL1  &= ~UCSWRST;
	UCB0CTL1 |= UCTXSTT + UCTR;		// Start i2c write operation

	while(!(UCB0CTL1 & UCTXSTT)); // Assert that start condition was started
	while((UC0IFG & UCB0TXIFG) == 0); // Wait until data is sent
	UCB0TXBUF = reg_addr; // Load the first byte of i2c data
	while((UC0IFG & UCB0TXIFG) == 0); // Wait until data is sent
	UCB0CTL1 &= ~UCTR; // Change to read mode and...
	UCB0CTL1 |= UCTXSTT;		// set repeated start
	while(!(UCB0CTL1 & UCTXSTT)); // Assert that start condition was started
	UCB0CTL1 |= UCTXSTP; // Set stop condition - necessary because single byte read
	while((UC0IFG & UCB0RXIFG) == 0); // Wait until data is received
	reg_value = UCB0RXBUF;
	UCB0CTL1 |= UCSWRST; // Turn off i2c

	return reg_value;
}

