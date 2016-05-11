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

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <stdio.h>
#include <stdlib.h>
#include "all_imports.h"
#include "utils_com.h"

//************************
//** twi

void twi_init()
{
	// disable power reduction for TWI
	PRR &=~ (1 << PRTWI);
	
	//SCL frequency = CPU Clock frequency / (16 + 2(TWBR) * PrescalerValue)
	// 1MHz => SCL == 62,5kHz
	// TWBR = 0;
	// TWSR, TWPS1, TWPS0

	// 1MHz (prescaler == 1, twbr == 7) => 400kHz
	//TWBR = 7;
	// TWSR, TWPS1, TWPS0
	
	// prescaler 1/16
	TWBR = 108;
	TWSR |= (1 << TWPS1);
	
	// enable twi
	TWCR |= (1 << TWEN);
}

void error(char string[])
{
	send_string(string);
	
	char ts[15];
	sprintf(ts, "%#x", TWSR & 0xF8);
	send_string(ts);
}

void twi_start()
{
	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	
	// wait for TWINT flag
	while (!(TWCR & (1<<TWINT)));
	
	// if error
	if (((TWSR & 0xF8) != TW_START) & ((TWSR & 0xF8) != TW_REP_START))
	error("err_start ");
}

void twi_address_read(uint8_t address)
{
	// transmit address
	TWDR = address;
	
	// start transmission
	TWCR = (1<<TWINT) | (1<<TWEN);
	
	// wait for TWINT flag
	while (!(TWCR & (1<<TWINT)));
	
	// if error
	if ((TWSR & 0xF8) != TW_MR_SLA_ACK)
	error("err_address_read ");
	
}

void twi_address_write(uint8_t address)
{
	// transmit address
	TWDR = address;
	
	// start transmission
	TWCR = (1<<TWINT) | (1<<TWEN);
	
	// wait for TWINT flag
	while (!(TWCR & (1<<TWINT)));
	
	// if error
	if ((TWSR & 0xF8) != TW_MT_SLA_ACK)
	error("err_address_write ");
	
}

void twi_send(uint8_t data)
{
	// transmit data
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	
	// wait for TWINT flag
	while (!(TWCR & (1<<TWINT)));
	
	// if error
	if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
	error("err_send ");
}

uint8_t twi_receive()
{
	// receive data
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	
	// wait for TWINT flag
	while (!(TWCR & (1<<TWINT)));
	
	// if error
	if ((TWSR & 0xF8) != TW_MR_DATA_ACK)
	error("err_receive ");
	
	return TWDR;
}

uint8_t twi_receive_last()
{
	// receive data
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	// wait for TWINT flag
	while (!(TWCR & (1<<TWINT)));

	// if error
	if ((TWSR & 0xF8) != TW_MR_DATA_NACK)
	error("err_receive_last ");

	return TWDR;
}

void twi_stop()
{
	// send STOP
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}
