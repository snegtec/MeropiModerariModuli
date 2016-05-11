/*****************************************************************************
*
* MEROPI MODERARI MODULI
* a simple microcontroller system for controlling light output of linear
* fluorescent tube according to saved program
*
* version: 0.1 (December 2014)
* compiler: Atmel Studio 6
* by       : Jacek Szymoniak
*          snegtec.com
*          snegtec@outlook.com
*
* License  : Copyright (c) 2014-2016 Jacek Szymoniak
*
****************************************************************************
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
****************************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "all_imports.h"
#include "utils_time.h"
#include "utils_pwm.h"
#include "utils.h"


char buffer[150];
uint8_t pos = 0;

enum {STANDARD, PASS};
int mode = STANDARD;


void clear_buffer()
{
	// clear buffer
	for (uint8_t i = 0; i < sizeof(buffer); i++)
	buffer[i] = 0x00;
	pos = 0;
}

void send_char(unsigned char letter)
{
	while (!( UCSR0A & (1<<UDRE0)));

	// write to buffer to send	
	UDR0 = letter;
}

void send_string(char *StringPtr)
{
	while(*StringPtr != 0x00)
	{
		send_char(*StringPtr);
		StringPtr++;
	}
}

void send_enter()
{
	send_string("\r\n");
}

void send_line(char *StringPtr)
{
	send_string(StringPtr);
	send_string("\r\n");
}

void send_int(int32_t value)
{
	char ts[30];
	sprintf(ts, "%ld", value);
	send_string(ts);
}

//************************
//* interpreter

void parse_command(char *value)
{
	// AT+SOMETHINGr => SOMETHING
	
	uint16_t size = strlen(value);
	
	// copy string
	char tca[size + 1];
	strncpy(tca, value, size);
	tca[size] = 0x00;
	
	// param (for example: MODE=LAST)
	char *found = strchr(tca, '=');
	if (found != NULL)
	{
		uint16_t where = found - tca;
		
		// copy command
		char command[where + 1];
		strncpy(command, tca, where);
		command[sizeof(command) - 1] = 0x00;
		
		// copy param
		char param[size - where + 1];
		strncpy(param, tca + where + 1, size - where - 1);
		param[size - where - 1] = 0x00;
		
		// ***
		
		// sets time: AT+TIME=17:33:45r
		// always 2 digits for every number: AT+TIME=03:05:07
		if (strcmp(command, "TIME") == 0)
		{
			char value[3];
			int16_t tia[3];
			long int result = 0;
			
			for (unsigned int i = 0; i < 3; i++)
			{
				strncpy(value, param + i * 3, 2);
				value[2] = 0x00;
				result = parse_int(value, &tia[i]);
			}
			
			if (result == 0)
			{
				send_string("WRONG TIME PARSE\r\n");
			} else
			{
				struct Time time;
				time.hour = tia[0];
				time.minute = tia[1];
				time.second = tia[2];
				
				set_time(&time);
				
				read_time();
			}
			
		}
		// AT+DATE=2014/10/29 WED ; 3 on the end is week_day
		// year - 4 digits, month and day are 2 digits, week_day is 1 digit
		// AT+DATE=2014/02/07/0
		else if (strcmp(command, "DATE") == 0)
		{
			char value[5];
			int16_t tia[4];
			long int result = 0;
			
			for (unsigned int i = 0; i < 4; i++)
			{
				// parse string
				switch (i)
				{
					case 0:
					strncpy(value, param, 4);
					value[4] = 0x00;
					break;
					
					case 1:
					strncpy(value, param + 5, 2);
					value[2] = 0x00;
					break;
					
					case 2:
					strncpy(value, param + 8, 2);
					value[2] = 0x00;
					break;
					
					case 3:
					strncpy(value, param + 11, 3);
					value[4] = 0x00;
					break;
				}
				
				if (i != 3)
				result = parse_int(value, &tia[i]);
				else
				tia[i] = weekdays(value);
			}
			
			if (result == 0)
			{
				send_string("WRONG DATE PARSE\r\n");
			} else
			{
				struct Date date;
				date.year = tia[0];
				date.month = tia[1];
				date.day = tia[2];
				date.week_day = tia[3];
				
				set_date(&date);
				
				read_date();
			}
			
		}
		// EXP1 WED 15h30 disable
		// AT+ADD=EXP1;WED1530;DISABLE
		// AT+ADD=PWM0;SAT2015;35
		// AT+ADD=PWM0;SAT2015;35;50;20
		else if (strcmp(command, "ADD") == 0)
		{
			char *pch;
			char *saveptr;
			
			// what pin
			pch = strtok_r(param, ";", &saveptr);
			int pin = what_pin(pch);
			
			// time
			pch = strtok_r(NULL, ";", &saveptr);
			int32_t time = parse_time(pch);
			
			// state
			pch = strtok_r(NULL, ";", &saveptr);
			int pin_state = 0;
			if (strcmp(pch, "LOW") == 0)
			pin_state = 0;
			else if (strcmp(pch, "HIGH") == 0)
			pin_state = 1;
			else
			{
				// percent value for pwm?
				int result = parse_int(pch, &pin_state);
				if (result == 0)
				send_string("WRONG PIN STATE PARSE\r\n");
			}
			
			// if PWM duration event
			pch = strtok_r(NULL, ";", &saveptr);
			
			int inc = -1;
			int16_t duration = -1;
			
			//AT+ADD=PWM0;THU2000;70;-50;30
			if (pch != NULL)
			{
				uint16_t result = parse_int(pch, &inc);
				if (result == 0)
				send_string("WRONG INC PARSE\r\n");
				
				// duration of change
				pch = strtok_r(NULL, ";", &saveptr);
				
				result = parse_int(pch, &duration);
				if (result == 0)
				send_string("WRONG DURATION PARSE\r\n");
			}
			
			// add event
			if (duration == -1)
			add_event(pin, time, pin_state);
			else
			add_pwm_event(pin, time, pin_state, inc, duration);
			
			send_string("OK\r\n");
		}
		// AT+CLOUD=PWM1;WED1530;30;50;30
		// AT+COOUD=pin;time;pin_state;duration;final_state
		else if (strcmp(command, "CLOUD") == 0)
		{
			char *pch;
			char *saveptr;
			
			// what pin
			pch = strtok_r(param, ";", &saveptr);
			int pin = what_pin(pch);
			
			// time;
			pch = strtok_r(NULL, ";", &saveptr);
			int32_t time = parse_time(pch);
			
			// state
			pch = strtok_r(NULL, ";", &saveptr);
			int16_t pin_state = 0;
			
			// percent value for pwm?
			int result = parse_int(pch, &pin_state);
			if (result == 0)
			send_string("WRONG PIN STATE PARSE\r\n");
			
			// duration
			pch = strtok_r(NULL, ";", &saveptr);
			int16_t duration = -1;
			
			result = parse_int(pch, &duration);
			if (result == 0)
			send_string("WRONG DURATION PARSE\r\n");
			
			// final_state
			pch = strtok_r(NULL, ";", &saveptr);
			int final_state = -1;
			
			result = parse_int(pch, &final_state);
			if (result == 0)
			send_string("WRONG FINAL STATE PARSE\r\n");
			
			add_clouds_event(pin, time, pin_state, duration, final_state);
			
			send_string("OK\r\n");
		}
		// AT+PWM0=30 => PWM0 at 30%
		else if (strcmp(command, "PWM0") == 0)
		{
			int16_t ti;
			long int result = 0;
			
			result = parse_int(param, &ti);
			
			if (result == 0)
			{
				send_string("WRONG PWM0 PARSE\r\n");
			} else
			{
				set_pwm0(ti);
				send_string("OK\r\n");
			}
			
		}
		// AT+PWM1=45 => PWM4 at 45%
		else if (strcmp(command, "PWM1") == 0)
		{
			int16_t ti;
			long int result = 0;
			
			result = parse_int(param, &ti);
			
			if (result == 0)
			{
				send_string("WRONG PWM1 PARSE\r\n");
			} else
			{
				set_pwm1(ti);
				send_string("OK\r\n");
			}
			
		}
		// AT+EXP0=0 => low state EXP0
		else if (strcmp(command, "EXP0") == 0)
		{
			if (param[0] == '1')
			set_pin(EXP0_PC2, 1);
			else
			set_pin(EXP0_PC2, 0);
			
			send_string("OK\r\n");
		}
		else if (strcmp(command, "EXP1") == 0)
		{
			if (param[0] == '1')
			set_pin(EXP1_PD4, 1);
			else
			set_pin(EXP1_PD4, 0);
			
			send_string("OK\r\n");
		}
		else if (strcmp(command, "EXP2") == 0)
		{
			if (param[0] == '1')
			set_pin(EXP2_PD7, 1);
			else
			set_pin(EXP2_PD7, 0);
			
			send_string("OK\r\n");
		}
		else if (strcmp(command, "EXP3") == 0)
		{
			if (param[0] == '1')
			set_pin(EXP3_PB0, 1);
			else
			set_pin(EXP3_PB0, 0);
			
			send_string("OK\r\n");
		}
		else if (strcmp(command, "PASS") == 0)
		{
			clear_buffer();
			
			mode = PASS;
			
			// pin34 - enable AT mode for BT
			// set high state
			PORTC |= (1 << DDC0);
			
			_delay_ms(200);
			
			send_string(param);
			send_string("\r\n");
			
			return;
			
		}
		else
		send_string("?\r\n");
		
	} else
	{
		// copy command
		char command[size + 1];
		strncpy(command, tca, size);
		command[sizeof(command) - 1] = 0x00;
		
		if (strcmp(command, "MEROPE") == 0)
		send_string("MEROPE OK\r\n");
		else if (strcmp(command, "TIME?") == 0)
		read_time();
		else if (strcmp(command, "DATE?") == 0)
		read_date();
		else if (strcmp(command, "PASS") == 0)
		{
			mode = PASS;
			clear_buffer();
			
			// pin34 - enable AT mode for BT
			// set high state
			PORTC |= (1 << DDC0);
			
			_delay_ms(100);
			
			send_string("AT+VERSION?\r\n");
			return;
		}
		else if (strcmp(command, "DISABLE_CLKOUT") == 0)
		{
			disable_clkout();
			send_string("OK\r\n");
		}
		else if (strcmp(command, "CLEAR_EVENTS") == 0)
		{
			clear_events();
			send_string("OK\r\n");
		}
		else if (strcmp(command, "PRINT_EVENTS") == 0)
		{
			print_events();
		}
		else if (strcmp(command, "MANUAL_MODE") == 0)
		{
			set_manual_mode();
			send_string("OK\r\n");
		}
		else if (strcmp(command, "EVENT_MODE") == 0)
		{
			set_event_mode();
			send_string("OK\r\n");
		}
		else if (strcmp(command, "GET_VL") == 0)
		{
			// check VL bit in PCF8563
			if (get_vl())
			send_string("VL bit is set. Replace backup battery\r\n");
			else
			send_string("VL bit isn't set. Backup battery is ok\r\n");
		}
		else if (strcmp(command, "RESET_VL") == 0)
		{
			// resets vl_bit in PCF8563
			reset_vl();
			send_string("OK\r\n");
		}
		else if (strcmp(command, "RESET") == 0)
		{
			// resets device
			send_string("OK\r\n");
			reset();
		}
		else
		send_string("?\r\n");
	}
}


void check()
{
	if ((mode == PASS) & ((buffer[pos - 1] == 0x0D) | (buffer[pos - 1] == 0x0A)))
	{
		// set low state
		PORTC &=~ (1 << DDC0);
		
		_delay_ms(100);
		
		mode = STANDARD;
		
		char tbuf[80];
		
		// clear buffer
		for (uint8_t i = 0; i < sizeof(tbuf); i++)
		tbuf[i] = 0x00;
		
		strncpy(tbuf, buffer, pos - 1);
		
		send_string(tbuf);
		send_string("\r\n");
		
		clear_buffer();
		return;
	} else if (mode == PASS)
	return;
	
	// always start with "AT"
	if ((buffer[0] != 'A') & (buffer[0] != 0x1B) /* & (buffer[0] != '#')*/)
	clear_buffer();
	
	// automatic reset before programming by BT and AVRDUDE
	else if ((buffer[0] == 0x1B) & (pos > 1) & (buffer[1] == 'S'))
	{
		// send '?' to force AVRDUDE repeat start sequence
		send_char('?');
		
		// reset
		reset();
		
		/*
		Explanation
		Current MeropeCentral uses butterfly compatible loader. 
		But programming needs to be done without any need to touch module. 
		So no-buttons-for-programming.
		
		AVRDUDE sends sequence ESC, S to programmer (programmed device with bootloader).
		But the problem was, AVRDUDE sends it only once. Unless it receives '?' char.
		So the idea is to use first sent '?', and after that ESC, S to reset device (to load bootloader).
		When AVRDUDE receives '?' it sends again ESC, S.
		And all works!
		
		For more info:
		if (pgm->flag & IS_BUTTERFLY_MK) {...} else {important part}
		https://github.com/kcuzner/avrdude/blob/master/avrdude/butterfly.c
		
		Simply: '?' char is needed after receiving ESC, S. But it cannot be send after reset, to do it is need to send '?' before reset.
		*/
	}
	// wrong reset command
	else if ((buffer[0] == 0x1B) & (pos > 1) & (buffer[1] != 'S'))
	{
		clear_buffer();
	}
	// if not AT command (but A-something)
	else if ((buffer[0] == 'A') & (buffer[1] != 'T') & (buffer[pos - 1] == 0x0D))
	{
		send_string("?\r\n");
		clear_buffer();
	}
	// AT command
	else if ((buffer[0] == 'A') & (buffer[1] == 'T') & (buffer[pos - 1] == 0x0D))
	{
		// AT + \r => size == 3
		if (buffer[2] == 0x0D)
		{
			send_string("OK\r\n");
		}
		// AT+SOMETHING or AT+MODE=LAST
		else if (buffer[2] == '+')
		{
			char *full_command;
			char *saveptr;
			
			// PASS mode?
			if ((buffer[3] == 'P') & (buffer[4] == 'A') & (buffer[5] == 'S') & (buffer[6] == 'S'))
			{
				send_line(buffer + 3);
				parse_command(buffer + 3);
				return;
			}
			
			// standard mode
			// buffer + 3 => omit "AT+"
			full_command = strtok_r(buffer + 3,"+&\r\n", &saveptr);
			while (full_command != NULL)
			{
				send_line(full_command);
				
				parse_command(full_command);
				
				// next command
				full_command = strtok_r(NULL, "+&\r\n", &saveptr);
			}
		}
		clear_buffer();
	}
}


//************************
//* usart

ISR(USART_RX_vect)
{
	buffer[pos] = UDR0;
	pos++;
	
	check();
}

void USART_Init()
{
	// disable power reduction for USART
	PRR &=~ (1 << PRUSART0);

	// enable interrupts for usart
	UCSR0B |= (1 << RXCIE0);
	
	// usart configuration
	//UBRR0H = (unsigned char)(MYUBRR>>8);
	//UBRR0L = (unsigned char) MYUBRR;
	// 9600 for 1MHz: 12 => taken from a table from datasheet for ATmega328P
	UBRR0H = (unsigned char)(12>>8);
	UBRR0L = (unsigned char) 12;
	
	// 9600 and double speed for 8MHz: 103
	//UBRR0H = (unsigned char)(103>>8);
	//UBRR0L = (unsigned char) 103;
	
	// set double speed
	UCSR0A |= (1 << U2X0);
	
	// set frame format: 8 data, 1 stop bit (default)
	UCSR0C |= (3<<UCSZ00);
	
	// enable receiver
	UCSR0B |= (1<<RXEN0);
	
	// enable transmitter
	UCSR0B |= (1<<TXEN0);
	
}

//************************
