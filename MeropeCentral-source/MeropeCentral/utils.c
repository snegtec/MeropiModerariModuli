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

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "all_imports.h"

int parse_int(const char *vs, int16_t *i)
{
	// here should be %d; %i doesn't work because strings like "08" are treated like octals
	// so "08" is converted to 0.
	if (sscanf (vs, "%d", i) == 1)
		return 1;
	
	return 0;
}

/*
Random number in range: <min, max>
<0..255>
*/
int util_rand2(uint8_t min, uint8_t max)
{
	// max - min => 0..range
	int range = max - min;
	
	uint8_t ri = rand();
	
	while (ri > range)
	ri = ri >> 2;
	
	return min + ri;
}
