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
 * Board.h
 *
 *  Defines which map actual hardware pins to functionality
 *
 */
#ifndef RATDBS_FIRMWARE_BOARD_H_
#define RATDBS_FIRMWARE_BOARD_H_

// Status LED
#define STATUS_LED_PDIR P2DIR
#define STATUS_LED_PSEL P2SEL
#define STATUS_LED_POUT P2OUT
#define STATUS_LED_PIN BIT5

/* Pins which control the switch matrix. Assumed on Port 1.
 *   Input Current --------  S_IN1 \----------/ S_OUT1 ----|
 *                     |                 V                 |
 *                     |               BRAIN               |
 *                     |                 ^                 |
 *                     |---  S_IN2 \-----|----/ S_OUT2 ----|------ GND */
#define S_OUT1 BIT4
#define S_OUT2 BIT3
#define S_IN1 BIT1
#define S_IN2 BIT4


// Reset and interrupt interface for RF430CCL330H (NFC IC)
#define RF430_RESET_POUT  P2OUT
#define RF430_RESET_PSEL  P2SEL
#define RF430_RESET_PSEL2 P2SEL2
#define RF430_RESET_PDIR  P2DIR
#define RF430_RESET_PIN   BIT2

#define RF430_INTR_POUT  P2OUT
#define RF430_INTR_PSEL  P2SEL
#define RF430_INTR_PSEL2 P2SEL2
#define RF430_INTR_PDIR  P2DIR
#define RF430_INTR_PREN  P2REN
#define RF430_INTR_PIE   P2IE
#define RF430_INTR_PIES  P2IES
#define RF430_INTR_PIFG  P2IFG
#define RF430_INTR_PIN   BIT0




#endif /* RATDBS_FIRMWARE_BOARD_H_ */
