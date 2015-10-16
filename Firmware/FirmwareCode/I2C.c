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
unsigned char* _i2c_rx_data;
unsigned int _i2c_tx_byte_count;
unsigned int _i2c_rx_byte_count;
unsigned char _i2c_repeated_start_rx;

int i2c_debug_flag = 0;

inline void I2CSetup (void)
{

    // Assume SMCLK is 1 MHz
    P1SEL  |= BIT6 + BIT7;                    // Assign I2C pins to USCI_B0
    P1SEL2 |= BIT6 + BIT7;                    // Assign I2C pins to USCI_B0
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 10;                             // fSCL = SMCLK(1MHz)/1 = 100 kHz
    UCB0BR1 = 0;
}

inline void InitializeI2CSlave(unsigned char slave_addr) {
    unsigned char temp;

	while (UCB0CTL1 & UCTXSTP) {
		temp = UCB0RXBUF;
	}
    UCB0CTL1 |= UCSWRST;           // Turn off i2c (this was buggy?)
    UCB0I2CSA = slave_addr;          // Slave Address
    UCB0CTL1  &= ~UCSWRST;           // Exit SW reset state of I2C
}

// Write two bytes to a register with a 2 byte address
inline void WriteRegister_WordAddress(uint16_t reg_addr, uint16_t reg_value)
{
    unsigned char data[4];
    data[0] = reg_addr >> 8;
    data[1] = reg_addr & 0xFF;
    data[2] = reg_value & 0xFF;
    data[3] = reg_value >> 8;

    WriteContinuous_I2C(data, 4);
}

// Write a byte to a register with a 1 byte address
void WriteRegister_ByteAddress(unsigned char reg_addr, unsigned char reg_value)
{
    unsigned char data[2];
    data[0] = reg_addr;
    data[1] = reg_value;

    WriteContinuous_I2C(data, 2);
}

//writes a byte string to I2C slave_addr of length data_length
void WriteContinuous_I2C(const unsigned char* write_data, unsigned int data_length)
{
    _i2c_repeated_start_rx = 0;
    IE2 |= UCB0TXIE;                 // Enable TX interrupt
    _i2c_tx_data = write_data;       // TX array start address
    _i2c_tx_byte_count = data_length;   // Load TX byte counter
    while (UCB0CTL1 & UCTXSTP);      // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;      // I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ interrupts (will stall here
                                            // until all data is TX'd).
    IE2 &= ~UCB0TXIE;                // Disable TX interrupt
}

// Read a series of bytes from a memory starting at a 2 byte address
void ReadMemory_WordAddress(uint16_t reg_addr,
        unsigned char* data, int byte_count)
{
    unsigned char addr_data[2];
    addr_data[0] = reg_addr >> 8;
    addr_data[1] = reg_addr & 0xFF;

    _i2c_repeated_start_rx = 1;
    _i2c_rx_data = data;              // RX array start address
    _i2c_rx_byte_count = byte_count;  // Load RX byte counter

    IE2 |= UCB0TXIE;                 // Enable TX interrupt
    _i2c_tx_data = addr_data;        // TX array start address
    _i2c_tx_byte_count = 2;          // Load TX byte counter
    while (UCB0CTL1 & UCTXSTP);      // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;      // I2C TX, start condition

    __bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ interrupts (will stall here
                                     //     until all data is TX'd and RX'd).
    IE2 &= ~UCB0RXIE;                // Disable RX interrupt.
    if (i2c_debug_flag != 0)
    	i2c_debug_flag = 0;
}


// Read a byte from a register with a 2 byte address
unsigned char ReadRegister_WordAddress(uint16_t reg_addr)
{
    unsigned char reg_value;
    ReadMemory_WordAddress(reg_addr, &reg_value, 2);
    return reg_value;
}

/* ------------------------------------------------------------------------------
 *
 * The USCIAB0TX_ISR handles both TX and RX interrupts.
 *   - For TX, a pointer to the data start buffer is loaded into
 *     _i2c_tx_data, and the number of bytes to be transmitted into
 *     _i2c_tx_byte_count. The interrupt will return from CPUOFF state
 *     when transmission is complete.
 *   - Similarly, for RX, a pointer to the data buffer to be filled
 *     is loaded into _i2c_rx_buffer, with the count in _i2c_rx_byte_count.
 *   - The most complex scenario is when a transmitted and repeated start to
 *     receive is required, for example to read from a specific memory address.
 *     In this case, both _i2c_tx* and _i2c_rx* must be filled, and the
 *     _i2c_repeated_start_rx flag set. Note that in this case, the process
 *     will be started by enabling the TX interrupt and setting the transmit/start
 *     flags, but that at the end, the RX interrupt needs to be disabled.
 *
 * ------------------------------------------------------------------------------ */
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
    if (UC0IFG & UCB0TXIFG) {
      if (_i2c_tx_byte_count) {         // Check TX byte counter
        UCB0TXBUF = *_i2c_tx_data++;    // Load TX buffer
        _i2c_tx_byte_count--;           // Decrement TX byte counter
      }
      else {
          if (_i2c_repeated_start_rx == 0) { // Regular TX
              UCB0CTL1 |= UCTXSTP;           // Got all data, set I2C stop condition
              IFG2 &= ~UCB0TXIFG;            // Clear USCI_B0 TX int flag
              __bic_SR_register_on_exit(CPUOFF);    // Exit LPM0
          }
          else {                        // Switch to RX with repeated start
              IE2 &= UCB0TXIE;          // Disable TX interrupt,...
              IE2 |= UCB0RXIE;          //    enable RX interrupt, ...
              IFG2 &= ~UCB0TXIFG;       //    clear USCI_B0 TX int flag, ...
              UCB0CTL1 &= ~UCTR;        //    change to read mode and, ...
              UCB0CTL1 |= UCTXSTT;      //    set repeated start.
          }
      }
    }
    else {
        _i2c_rx_byte_count--;                // Decrement RX byte counter
        if (_i2c_rx_byte_count) {            // Check RX byte counter
          *_i2c_rx_data++ = UCB0RXBUF;       // Fill RX buffer
          __delay_cycles(2);
          if (_i2c_rx_byte_count == 1)       // Second to last bye,
              UCB0CTL1 |= UCTXSTP;           //    set I2C stop condition.
        }
        else {
          *_i2c_rx_data++ = UCB0RXBUF;       // Load last RX byte
          __bic_SR_register_on_exit(CPUOFF); // Exit LPM0
        }
    }
}





// Read a byte from a register with a 1 byte address
unsigned char ReadRegister_ByteAddress(unsigned char reg_addr)
{
    unsigned char reg_value;

    UCB0CTL1  &= ~UCSWRST;
    UCB0CTL1 |= UCTXSTT + UCTR;     // Start i2c write operation

    while(!(UCB0CTL1 & UCTXSTT)); // Assert that start condition was started
    while((UC0IFG & UCB0TXIFG) == 0); // Wait until data is sent
    UCB0TXBUF = reg_addr; // Load the first byte of i2c data
    while((UC0IFG & UCB0TXIFG) == 0); // Wait until data is sent
    UCB0CTL1 &= ~UCTR; // Change to read mode and...
    UCB0CTL1 |= UCTXSTT;        // set repeated start
    while(!(UCB0CTL1 & UCTXSTT)); // Assert that start condition was started
    UCB0CTL1 |= UCTXSTP; // Set stop condition - necessary because single byte read
    while((UC0IFG & UCB0RXIFG) == 0); // Wait until data is received
    reg_value = UCB0RXBUF;

    return reg_value;
}

