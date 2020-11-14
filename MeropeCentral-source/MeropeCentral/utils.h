/*****************************************************************************
*
* MEROPI MODERARI MODULI
* a simple microcontroller system for controlling light output of linear 
* fluorescent tube according to saved program
*
* version: 0.1 (December 2014)
* compiler: Atmel Studio 6
* by       : Erik Kos
*          snegtec.com
*          snegtec@outlook.com
*
* License  : Copyright (c) 2014-2020 Erik Kos
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

// utils.c
int util_rand2(uint8_t min, uint8_t max);
int parse_int(const char *vs, int16_t *i);

