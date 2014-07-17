/*
	1/14/09, Copyright Sparkfun Electronics
	nRF_USB2Serial.c
	Pete Dokter
	
	This code is designed to operate on the Nordic USB Serial converter and
	give the user a head start on the learning curve for the Nordic nRF2401A
	and nRF24L01 RF devices. Usind 2 such boards along with 2 like Nordic
	devices, the converter boards will first detect which devices have been
	attached, then allow the user to transmit characters bidirectionally.
	
	Notes:
	1) the Serial USB operates at 9600 baud
	2) the input buffer on converter board (in this code) is 512 bytes
	3) the transmit buffer is 4 bytes, meaning that you must send 4 bytes
		from your terminal before the device will transmit
	4) Upon power up and after USB initialization, the blue status LEDs
		will blink rapidly, then go out. After a second or so, STAT0 will
		blink twice to indicate nRF24L01 detection, OR STAT1 will blink
		twice to indicate that it has defaulted to work with an nRF2401A
		(meaning that an nRF24L01 was not detected). After that, the status
		LEDs will cycle 0-1-2-0-1-2 when a transmission has been recieved.
    
*/
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define FOSC 1000000
#define BAUD 9600
#define MYUBRR (((((FOSC * 10) / (16L * (BAUD/2))) + 5) / 10) - 1)

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)(~(1 << mask)))

#define STAT0	0 //PC0
#define STAT1	1 //PC1
#define STAT2	2 //PC2

//Define functions
//======================
void ioinit(void);

//static int uart_putchar(char c, FILE *stream);
uint8_t uart_getchar(void);
void put_char(char byte);

//static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void delay_ms(uint16_t x); //General purpose delay
void delay_us(uint8_t x);

//uint16_t check_beat(void);
//void blink_channel(char channel, uint16_t on_pulse, uint16_t off_pulse);

#include <nRF2401A_lib.c>
#include <nRF24L01_lib.c>
//======================
unsigned char RX_array[512];
volatile unsigned short RX_in = 0;
unsigned short RX_read = 0;
unsigned char data_send = 0;




ISR (SIG_USART_RECV)//USART Receive Interrupt
{
	cli();//Disable Interrupts
	RX_array[RX_in] = UDR0;
	
	RX_in++;
	
	if (RX_in >= 416) RX_in = 0;
	
	sei();//Enable Interrupts
	
}

ISR(TIMER2_OVF_vect)
{
	//This vector is only here to wake unit up from sleep mode
}

int main(void)
{
	int x, y, z;
	
	unsigned char nRF24L01;
	
	nRF24L01 = 0;
	
    ioinit();
	
	delay_ms(1500);
	
	for (x = 0; x < 5; x++)
	{
		cbi(PORTC, STAT1);//stat1 on
		delay_ms(50);
		sbi(PORTC, STAT1);//stat1 off
		
		cbi(PORTC, STAT0);//stat0 on
		cbi(PORTC, STAT2);//stat2 on
		delay_ms(50);
		sbi(PORTC, STAT0);//stat0 off
		sbi(PORTC, STAT2);//stat2 off
	}
	
	
	
	delay_ms(1500);
	
	
	//check for 24L01=================================================
	init_24L01_pins();
	
	for (x = 0; x < 20; x++)
	{
		cbi(L01_PORT, L01_CSN); //Select chip
		y = tx_spi_byte(0xFF);
		sbi(L01_PORT, L01_CSN); //Deselect chip
		delay_ms(50);
		
	}
	
	if (y == 14) nRF24L01 = 1;
	
	
	y = 0, z = 0;
	
	//indicate 24L01 detected
	if (nRF24L01 == 1)
	{
		for (x = 0; x < 2; x++)
		{
			cbi(PORTC, STAT0);//stat1 on
			delay_ms(250);
			sbi(PORTC, STAT0);//stat1 off
			delay_ms(250);
		}
	}
	
	//indicate 2401A
	else
	{
		for (x = 0; x < 2; x++)
		{
			cbi(PORTC, STAT1);//stat1 on
			delay_ms(250);
			sbi(PORTC, STAT1);//stat1 off
			delay_ms(250);
		}
	}
	
	
	cbi(PORTC, STAT0);//stat0 on
		
	if (nRF24L01 == 1) config_rx_nRF24L01();
	else config_rx_nRF2401A();
	
	for (x = 0; x < 4; x++)
	{
		rf_rx_array[x] = 0;
	}
		
	while(1)
	{
		if(RX_in != RX_read)
		{
			rf_tx_array[y] = RX_array[RX_read];
			RX_read++, y++;
			if(RX_read >= 512) RX_read = 0;
			if (y == 4)
			{
				if (nRF24L01 == 1)
				{
					config_tx_nRF24L01();
					data_array[0] = rf_tx_array[0];
					data_array[1] = rf_tx_array[1];
					data_array[2] = rf_tx_array[2];
					data_array[3] = rf_tx_array[3];
					tx_data_nRF24L01();
					config_rx_nRF24L01();
				}
				
				else
				{
					config_tx_nRF2401A();
					tx_data_nRF2401A();
					config_rx_nRF2401A();
				}
				
				y = 0;
			}
			
		}	
			
				
		if (nRF24L01 == 1)
		{
			if (!(L01_IRQ_PORT & (1<<RX_IRQ)))
			{
				rx_data_nRF24L01();
				data_send = 1;
			}
		}
		
		else if (_01A_PORT_PIN & (1<<_01A_DR))
		{
			rx_data_nRF2401A();
			data_send = 1;
		}
			
		if (data_send)
		{
			for (x = 0; x < 4; x++)
			{
				if (rf_rx_array[x] != 0) put_char(rf_rx_array[x]);

			}
			
			data_send = 0;

			if (z == 0)
			{
				sbi(PORTC, STAT0);//stat0 off
				cbi(PORTC, STAT1);//stat1 on
			}
			
			else if (z == 1)
			{
				sbi(PORTC, STAT1);//stat1 off
				cbi(PORTC, STAT2);//stat2 on
			}
			
			else if (z == 2)
			{
				sbi(PORTC, STAT2);//stat2 off
				cbi(PORTC, STAT0);//stat0 on
			}
			
			z++;
			if (z == 3) z = 0;		
		
		}
	}
	
}




void ioinit(void)
{
	//1 = output, 0 = input
	DDRC = 0b00000111;//Status LEDs on PC0-2
	DDRB = 0b00100110;
	
	PORTC = 0b00000111;

	//USART Baud rate: 9600
    UBRR0H = MYUBRR >> 8;
    UBRR0L = MYUBRR;
	
    //UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0A = (1<<U2X0);
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);	//Enable Interrupts on receive character
	
	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);

    //Init Timer0 for delay_us
    //TCCR0B = (1<<CS00); //Set Prescaler to No Prescaling (assume we are running at internal 1MHz). CS00=1 
	TCCR2B = (1<<CS20); //Set Prescaler to 1. CS20=1
	
	sei();
	
    //Setup nRF2401A
	//configure_receiver();

}


uint8_t uart_getchar(void)
{
    while( !(UCSR0A & (1<<RXC0)) );
    return(UDR0);
}

//General short delays
void delay_ms(uint16_t x)
{
	
	for (; x > 0 ; x--)
	{
		delay_us(250);
		delay_us(250);
		delay_us(250);
		delay_us(250);
	}
}

//General short delays
void delay_us(uint8_t x)
{
	TIFR2 = 0x01; //Clear any interrupt flags on Timer2
	
    TCNT2 = 256 - x; //256 - 125 = 131 : Preload timer 2 for x clicks. Should be 1us per click

	while( (TIFR2 & (1<<TOV2)) == 0);
}


void put_char(char byte)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = byte;
}


