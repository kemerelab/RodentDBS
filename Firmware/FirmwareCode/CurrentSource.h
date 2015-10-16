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
 * Current Source Control Code
 *
 * This file contains the code required to control the DS4432 constant current
 * source. The OUT0 line is used in the most recent version of the module. The
 * amount of current is determined by the value in register at address 0xF8,
 * according to the formula:
 *
 *     I_out = DAC_VALUE * I_FS / 127
 *
 * where I_FS = V_RFS / (16 * R_FS) * 127 = 0.997 / (16 * 47 K) * 127
 *            ~= 168.4 uA
 *
 * Note that value is *NOT* two's complement signed. Rather, the MSB corresponds
 * to the direction of current, with 0 === SINK and 1 === SOURCE. With brain
 * connected between CURRENT and GROUND, we will almost always want the SOURCE
 * option.
 *
 * Resources:
 *   - Uses I2C to write to the appropriate registers
 *
*/

#ifndef CURRENT_SOURCE_H
#define CURRENT_SOURCE_H_

#include "stdint.h"

#define DS4432_ADDRESS 0x48
#define CURRENT0_REG_ADDR 0xF8
#define DIRECTION_MASK 0x80

inline void SetOutputCurrent (uint8_t value);

#endif

