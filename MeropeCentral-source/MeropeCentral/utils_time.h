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


struct Time
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	
};

struct Date
{
	uint8_t day;
	uint8_t week_day;
	uint8_t month;
	uint16_t year;
};


// util_time.c
uint32_t current_time();
void timeToBcd(struct Time time, uint8_t *hour_bcd, uint8_t *minute_bcd, uint8_t *second_bcd);
void bcdToTime(uint8_t hour_bcd, uint8_t minute_bcd, uint8_t second_bcd, struct Time *time);

void disable_clkout();
void read_time();
void get_time(struct Time *time);
void set_time(struct Time *time);
void read_date();
void get_date(struct Date *date);
void set_date(struct Date *date);
int get_vl();
void reset_vl();

int weekdays(char *day);
char * days(int weekday);

int32_t parse_time(char *time);
void timeToString(int32_t time_value, char *string);