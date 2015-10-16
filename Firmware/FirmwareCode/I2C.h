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

#ifndef RATDBS_FIRMWARE_I2C_H_
#define RATDBS_FIRMWARE_I2C_H_

#include <msp430.h>
#include "I2C.h"
#include <stdint.h>

extern int i2c_debug_flag;

inline void I2CSetup (void);

// initialize I2C system for communicating with a different slave
inline void InitializeI2CSlave(unsigned char slave_addr);

//writes a byte string to I2C slave_addr of length data_length
void WriteContinuous_I2C(const unsigned char* write_data, unsigned int data_length);

// Write a byte to a register with a 2 byte address
void WriteRegister_WordAddress(uint16_t reg_addr, uint16_t reg_value);

// Write a byte to a register with a 1 byte address
void WriteRegister_ByteAddress(unsigned char reg_addr, unsigned char reg_value);

// Read a series of bytes from a memory starting at a 2 byte address
void ReadMemory_WordAddress(uint16_t reg_addr, unsigned char* data, int byte_count);

// Read a byte from a register with a 2 byte address
unsigned char ReadRegister_WordAddress(uint16_t reg_addr);

// Read a byte from a register with a 1 byte address
unsigned char ReadRegister_ByteAddress(unsigned char reg_addr);


#endif

