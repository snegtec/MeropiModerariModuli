/*****************************************************************************
*
* MEROPI MODERARI MODULI
* a simple microcontroller system for controlling light output of linear 
* fluorescent tube according to saved program
*
* version: 0.2 (December 2017)
* compiler: Atmel Studio 6
* by       : Erik Kos
*          snegtec.com
*          snegtec@outlook.com
*
* License  : Copyright (c) 2014-2017 Erik Kos
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


/*
* MeropeCentral.c
*
* Created: 2014-10-06 17:20:05
*  Author: jck
*/

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include "all_imports.h"
#include "utils_time.h"
#include "utils_com.h"
#include "utils_pwm.h"
#include "utils_twi.h"

/*
cpu frequency is defined in menu: Project/...properties/Toolchain/AVR/GNU C compiler/Symbols/F_CPU=1000000UL
cpu freqency is set by fuse bits not by definition by F_CPU; F_CPU is needed by delay.h; ckdiv8 is also set
*/

void initialize_app()
{
	// init i2c
	twi_init();
	
	// disable BT reset (it is connected to PC1 pin ATMega328P)
	// set pin as output
	DDRC |= (1 << DDC1);
	
	// set high state
	PORTC |= (1 << DDC1);
	
	
	// pin34 - for disabling AT mode for BT
	// it is connected to PC0
	// set pin as output
	DDRC |= (1 << DDC0);
	
	// set low state
	PORTC &=~ (1 << DDC0);
	
	// set params for usart
	// 9600N81 - 9600, no parity, 8 data bits, 1 stop bit
	USART_Init();
	
	// set as output
	DDRD |= (1 << DDD5);

	char String[] = "start ";
	send_string(String);
	send_string("\r\n");
	
	setup_pwm();
	
	// enable interrupts
	SREG |= (1 << 7);
}

/*
Resets device
*/
void reset()
{
	// set up watchdog timer
	wdt_enable(WDTO_250MS);
}

int main(void)
{
	// disable watchdog timer
	MCUSR = 0;
	wdt_disable();
	
	initialize_app();
	
	// check VL bit in PCF8563
	if (get_vl())
	send_string("Replace backup battery and clear VL bit!!!\r\n");
	
	load_events();
	
	while(1)
	{
		// additional timer event
		_delay_ms(5000);
		timer_event();
		
	}
	
}
