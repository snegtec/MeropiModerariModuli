/*****************************************************************************
*
* MEROPI MODERARI MODULI
* a simple microcontroller system for controlling light output of linear
* fluorescent tube according to saved program
*
* version: 0.2 (December 2017)
* compiler: Atmel Studio 6
* by       : Jacek Szymoniak
*          snegtec.com
*          snegtec@outlook.com
*
* License  : Copyright (c) 2014-2017 Jacek Szymoniak
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

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>

#include "all_imports.h"
#include "utils_com.h"

//************************
//**

void setup_pwm()
{
	// set pins as outputs
	// set pin as output
	DDRD |= (1 << DDD6); // PWM0, OC0A
	DDRD |= (1 << DDD5); // PWM1, OC0B
	
	// DDR - set driver for OC0A and OC0B pins
	// set pins as output
	//DDRD |= (1 << DDD6);
	
	// enable interrupts for timer0
	//TIMSK0 |= (1 << TOIE0);
	//TIFR0 |= (1 << TOV0);
	
	// mode 3, Fast PWM
	// def TCCR0B = TCCR0B & ~(1 << WGM02);
	TCCR0A = TCCR0A | (1 << WGM01);
	TCCR0A = TCCR0A | (1 << WGM00);
	
	// clock select
	// TCCR0B: CS02 CS01 CS00
	
	// clkio/1024
	//TCCR0B |= (0b101 << CS00);
	
	// clkio/64
	//TCCR0B |= (0b011 << CS00);
	
	// clkio/256
	//TCCR0B |= (0b100 << CS00);
	
	// clkio/1
	//TCCR0B |= (0b001 << CS00);
	
	// clkio/8
	TCCR0B |= (0b010 << CS00);
	
	// toggle OC pin
	//TCCR0A |= (0b10 << COM0A0);
	
	// toggle
	//TCCR0A |= (1 << COM0A0);
	//TCCR0A &=~ (1 << COM0A1);
	
	// clear/set
	TCCR0A &=~ (1 << COM0A0);
	TCCR0A |= (1 << COM0A1);
}

/*
value: 0..255
*/
void set_pwm0(uint8_t value)
{
	// write timer register
	OCR0A = ~ value;
}

/*
value: 0..255
*/
void set_pwm1(uint8_t value)
{
	// write timer register
	OCR0B = value;
}

void set_pin(uint8_t pin, uint8_t pin_state)
{
	
	switch(pin)
	{
		case EXP1_PD4:
		// set pin as output
		DDRD |= (1 << DDD4);
		
		if (pin_state == HIGH_STATE)
		// set high state
		PORTD |= (1 << DDD4);
		else
		// set low state
		PORTD &=~ (1 << DDD4);
		break;
		
		case EXP2_PD7:
		// set pin as output
		DDRD |= (1 << DDD7);
		
		if (pin_state == HIGH_STATE)
		// set high state
		PORTD |= (1 << DDD7);
		else
		// set low state
		PORTD &=~ (1 << DDD7);
		break;
		
		case EXP0_PC2:
		// set pin as output
		DDRC |= (1 << DDC2);
		
		if (pin_state == HIGH_STATE)
		// set high state
		PORTC |=(1 << DDC2);
		else
		// set low state
		PORTC &=~ (1 << DDC2);
		break;
		
		case EXP3_PB0:
		// set pin as output
		DDRB |= (1 << DDB0);
		
		if (pin_state == HIGH_STATE)
		// set high state
		PORTB |= (1 << DDB0);
		else
		// set low state
		PORTB &=~ (1 << DDB0);
		break;
		default:break;
	}
}
