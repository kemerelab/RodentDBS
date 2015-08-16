/*
 * {RF430_example.h}
 *
 * {RF430 header}
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
 * RF430_example.h
 *
 *	This is for the FR5739 FRAM Launch Pad
 *
 *  Created on: Feb 6, 2013
 *      Author: JD Crutchfield
 */

#ifndef RF430_EXAMPLE_H_
#define RF430_EXAMPLE_H_

unsigned int Read_Register(unsigned int reg_addr);
unsigned int Read_Register_BIP8(unsigned int reg_addr);
void Read_Continuous(unsigned int reg_addr, unsigned char* read_data, unsigned int data_length);

void Write_Register(unsigned int reg_addr, unsigned int value);
void Write_Continuous(unsigned int reg_addr, unsigned char* write_data, unsigned int data_length);
void Write_Register_BIP8(unsigned int reg_addr, unsigned int value);

//MSP-EXP430FR5739 port definitions
//I2C
#define PORT_I2C_OUT	P1OUT
#define PORT_I2C_DIR	P1DIR
#define PORT_I2C_SEL0	P1SEL0
#define PORT_I2C_SEL1	P1SEL1
#define SDA	BIT6
#define SCL BIT7

//INTO
#define PORT_INTO_IN	P2IN
#define PORT_INTO_OUT	P2OUT
#define PORT_INTO_DIR	P2DIR
#define PORT_INTO_SEL0	P2SEL0
#define PORT_INTO_SEL1	P2SEL1
#define PORT_INTO_REN	P2REN
#define PORT_INTO_IE	P2IE
#define PORT_INTO_IES	P2IES
#define PORT_INTO_IFG	P2IFG
#define INTO	BIT0

//RST
#define PORT_RST_OUT	P3OUT
#define PORT_RST_DIR	P3DIR
#define PORT_RST_SEL0	P3SEL0
#define PORT_RST_SEL1	P3SEL1
#define RST	BIT7

//Buttons
#define PORT_BUTTON_IN		P4IN
#define PORT_BUTTON_OUT		P4OUT
#define PORT_BUTTON_DIR 	P4DIR
#define PORT_BUTTON_SEL0	P4SEL0
#define PORT_BUTTON_SEL1	P4SEL1
#define PORT_BUTTON_REN		P4REN
#define PORT_BUTTON_IE		P4IE
#define PORT_BUTTON_IES		P4IES
#define PORT_BUTTON_IFG		P4IFG
#define S1	BIT0
#define S2	BIT1

//LEDs
#define PORT_LED_OUT	P3OUT
#define PORT_LED_DIR	P3DIR
#define PORT_LED_SEL0	P3SEL0
#define PORT_LED_SEL1	P3SEL1
#define LED5	BIT4
#define LED6	BIT5
#define LED7	BIT6
//#define LED8	BIT7 //LED8 is on same pin as RF430CL330H RST line, so can't use

//define the values for Granite's registers we want to access
#define CONTROL_REG 		0xFFFE
#define STATUS_REG			0xFFFC
#define INT_ENABLE_REG		0xFFFA
#define INT_FLAG_REG		0xFFF8
#define CRC_RESULT_REG		0xFFF6
#define CRC_LENGTH_REG		0xFFF4
#define CRC_START_ADDR_REG	0xFFF2
#define COMM_WD_CTRL_REG	0xFFF0
#define VERSION_REG			0xFFEE //contains the software version of the ROM
#define TEST_FUNCTION_REG   0xFFE2
#define TEST_MODE_REG		0xFFE0

//define the different virtual register bits
//CONTROL_REG bits
#define SW_RESET		BIT0
#define RF_ENABLE		BIT1
#define INT_ENABLE		BIT2
#define INTO_HIGH		BIT3
#define INTO_DRIVE		BIT4
#define BIP8_ENABLE		BIT5
#define STANDBY_ENABLE	BIT6
#define TEST430_ENABLE	BIT7

//STATUS_REG bits
#define READY			BIT0
#define CRC_ACTIVE		BIT1
#define RF_BUSY			BIT2

//INT_ENABLE_REG bits
#define EOR_INT_ENABLE		BIT1
#define EOW_INT_ENABLE		BIT2
#define CRC_INT_ENABLE		BIT3
#define BIP8_ERROR_INT_ENABLE		BIT4
#define NDEF_ERROR_INT_ENABLE		BIT5
#define GENERIC_ERROR_INT_ENABLE	BIT7

//INT_FLAG_REG bits
#define EOR_INT_FLAG	BIT1
#define EOW_INT_FLAG	BIT2
#define CRC_INT_FLAG	BIT3
#define BIP8_ERROR_INT_FLAG	BIT4
#define NDEF_ERROR_INT_FLAG	BIT5
#define GENERIC_ERROR_INT_FLAG	BIT7

//COMM_WD_CTRL_REG bits
#define WD_ENABLE	BIT0
#define TIMEOUT_PERIOD_2_SEC	0
#define TIMEOUT_PERIOD_32_SEC	BIT1
#define TIMEOUT_PERIOD_8_5_MIN	BIT2
#define TIMEOUT_PERIOD_MASK		BIT1 + BIT2 + BIT3

#define TEST_MODE_KEY 0x004E

#define RF430_DEFAULT_DATA		{  														\
/*NDEF Tag Application Name*/ 															\
0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 												\
																						\
/*Capability Container ID*/ 															\
0xE1, 0x03, 																			\
0x00, 0x0F,	/* CCLEN */																	\
0x20,		/* Mapping version 2.0 */													\
0x00, 0xF9,	/* MLe (49 bytes); Maximum R-APDU data size */								\
0x00, 0xF6, /* MLc (52 bytes); Maximum C-APDU data size */								\
0x04, 		/* Tag, File Control TLV (4 = NDEF file) */									\
0x06, 		/* Length, File Control TLV (6 = 6 bytes of data for this tag) */			\
0xE1, 0x04,	/* File Identifier */														\
0x0B, 0xDF, /* Max NDEF size (3037 bytes of useable memory) */							\
0x00, 		/* NDEF file read access condition, read access without any security */		\
0x00, 		/* NDEF file write access condition; write access without any security */	\
																						\
/* NDEF File ID */ 																		\
0xE1, 0x04, 																			\
																						\
/* NDEF File for Hello World  (48 bytes total length) */								\
0x00, 0x14, /* NLEN; NDEF length (3 byte long message) */ 								\
0xD1, 0x01, 0x10, 																		\
0x54, /* T = text */																	\
0x02, 																					\
0x65, 0x6E, /* 'e', 'n', */																\
																						\
/* 'Hello, world!' NDEF data; Empty NDEF message, length should match NLEN*/			\
0x48, 0x65, 0x6C, 0x6C, 0x6f, 0x2c, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21  			\
}

#endif /* RF430_EXAMPLE_H_ */
