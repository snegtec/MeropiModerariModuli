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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "all_imports.h"
#include "utils_time.h"
#include "utils_com.h"
#include "utils_twi.h"
#include "utils.h"


/*
There is need to add some delay between sending commands by i2c.
_delay_ms(50);

*/

#define VL_BIT 7
#define CENTURY_BIT 7

void timeToBcd(struct Time time, uint8_t *hour_bcd, uint8_t *minute_bcd, uint8_t *second_bcd)
{
	// seconds
	*second_bcd = ((time.second / 10) << 4) + (time.second % 10);
	
	// minutes
	uint8_t tbcd = ((time.minute / 10) << 4) + (time.minute % 10);
	*minute_bcd = tbcd;
	
	// hour
	tbcd = ((time.hour / 10) << 4) + (time.hour % 10);
	*hour_bcd = tbcd;
}

void set_time(struct Time *time)
{
	uint8_t hour_bcd;
	uint8_t minute_bcd;
	uint8_t second_bcd;
	timeToBcd(* time, &hour_bcd, &minute_bcd, &second_bcd);
	
	// send data
	twi_start();
	twi_address_write(0xA2);
	
	twi_send(0x02);
	twi_send(second_bcd);
	twi_send(minute_bcd);
	twi_send(hour_bcd);
	
	twi_stop();
	_delay_ms(50);
	
}

/*
Converts bcd time to Time struct
*/
void bcdToTime(uint8_t hour_bcd, uint8_t minute_bcd, uint8_t second_bcd, struct Time *time)
{
	
	// minutes 6:4 and 3:0 bits
	uint8_t second = ((0b01110000 & second_bcd) >> 4) * 10 + (0b00001111 & second_bcd);
	
	// minutes 6:4 and 3:0 bits
	uint8_t minute = ((0b01110000 & minute_bcd) >> 4) * 10 + (0b00001111 & minute_bcd);
	
	// hours 5:4 and 3:0 bits
	uint8_t hour = ((0b00110000 & hour_bcd) >> 4) * 10 + (0b00001111 & hour_bcd);
	
	time->second = second;
	time->minute = minute;
	time->hour = hour;
}

void get_time(struct Time *time)
{
	// PCF8563 - address for read: 0xA3, address for write: 0xA2
	
	twi_start();
	twi_address_write(0xA2);
	twi_send(0x02);
	
	twi_start();
	twi_address_read(0xA3);
	
	uint8_t received[3];
	
	for (uint8_t i = 0; i < 2; i++)
	received[i] = twi_receive();
	
	received[2] = twi_receive_last();
	
	twi_stop();
	_delay_ms(50);

	//struct Time time;
	bcdToTime(received[2], received[1], received[0], time);
}

void read_time()
{
	struct Time time;
	get_time(&time);
	
	char ts[30];
	send_string("time: ");
	sprintf(ts, "%d", time.hour);
	send_string(ts);
	send_string(":");
	sprintf(ts, "%02d", time.minute);
	send_string(ts);
	send_string(":");
	sprintf(ts, "%02d", time.second);
	send_string(ts);
	send_string("\r\n");
}

/*
Converts bcd date to Date struct
*/
void bcdToDate(uint8_t year_bcd, uint8_t month_bcd, uint8_t day_bcd, uint8_t week_day_bcd, struct Date *date)
{
	// day 5:4 and 3:0 bits
	uint8_t day = ((0b00110000 & day_bcd) >> 4) * 10 + (0b00001111 & day_bcd);
	
	// week_day 2:0 bits
	// Sunday is 0, Saturday is 6
	uint8_t week_day = 0b00000111 & week_day_bcd;
	
	// month 4:4 and 3:0 bits
	uint8_t month = ((0b00010000 & month_bcd) >> 4) * 10 + (0b00001111 & month_bcd);
	
	// century
	uint8_t century = month_bcd & (1 << 7);
	
	// year 7:4 and 3:0 bits
	uint16_t year = ((0b11110000 & year_bcd) >> 4) * 10 + (0b00001111 & year_bcd);
	
	if (century)
	year += 2000;
	else
	year += 1900;
	
	date->day = day;
	date->week_day = week_day;
	date->month = month;
	date->year = year;
}

void dateToBcd(struct Date date, uint8_t *year_bcd, uint8_t *month_bcd, uint8_t *day_bcd, uint8_t *week_day_bcd)
{
	// year
	if (date.year >= 2000)
	*year_bcd = (((date.year - 2000) / 10) << 4) + (date.year % 10);
	else
	*year_bcd = (((date.year - 1900) / 10) << 4) + (date.year % 10);
	
	// month
	uint8_t tbcd = ((date.month / 10) << 4) + (date.month % 10);
	*month_bcd = tbcd;
	
	// set century flag
	if (date.year >= 2000)
	*month_bcd |= (1 << CENTURY_BIT);
	
	// day
	tbcd = ((date.day / 10) << 4) + (date.day % 10);
	*day_bcd = tbcd;
	
	// week_day
	tbcd = ((date.week_day / 10) << 4) + (date.week_day % 10);
	*week_day_bcd = tbcd;
	
}

void get_date(struct Date *date)
{
	// PCF8563 - address for read: 0xA3, address for write: 0xA2
	
	twi_start();
	twi_address_write(0xA2);
	twi_send(0x05);
	
	twi_start();
	twi_address_read(0xA3);
	
	uint8_t received[4];
	
	for (uint8_t i = 0; i < 3; i++)
	received[i] = twi_receive();
	
	received[3] = twi_receive_last();
	
	twi_stop();

	// year, month, day, week_day
	bcdToDate(received[3], received[2], received[0], received[1], date);
	
}

void set_date(struct Date *date)
{
	uint8_t year_bcd;
	uint8_t month_bcd;
	uint8_t day_bcd;
	uint8_t week_day_bcd;
	
	dateToBcd(*date, &year_bcd, &month_bcd, &day_bcd, &week_day_bcd);
	
	// send data
	twi_start();
	twi_address_write(0xA2);
	
	twi_send(0x05);
	twi_send(day_bcd);
	twi_send(week_day_bcd);
	twi_send(month_bcd);
	twi_send(year_bcd);
	
	twi_stop();
	_delay_ms(50);
}


/*
Converts string date (for example: "1530") to time and date (number of seconds)
*/
int32_t parse_time(char *time)
{
	char tca[6];
	
	strncpy(tca, time, 2);
	tca[2] = 0;
	int16_t hour = 0;
	int result = parse_int(tca, &hour);
	
	if (result == 0)
	return -1;
	
	strncpy(tca, time + 2, 2);
	tca[2] = 0;
	int16_t minute = 0;
	result = parse_int(tca, &minute);
	
	if (result == 0)
	return -1;
	
	int32_t ti = ((int32_t) hour * 60 * 60) + (minute * 60);
	
	return ti;
}

/*
Converts "WED1530" to time and date (number of seconds)
*/
/*
int32_t parse_time(char *time)
{
	char tca[6];

	strncpy(tca, time, 3);
	tca[3] = 0;
	int days = weekdays(tca);

	strncpy(tca, time + 3, 2);
	tca[2] = 0;
	int16_t hour = 0;
	int result = parse_int(tca, &hour);

	if (result == 0)
	return -1;

	strncpy(tca, time + 5, 2);
	tca[2] = 0;
	int16_t minute = 0;
	result = parse_int(tca, &minute);

	if (result == 0)
	return -1;

	int32_t ti = ((int32_t) days * 24 * 60 * 60) + ((int32_t) hour * 60 * 60) + (minute * 60);

	return ti;
}
*/

/*
Converts time from an integer to format like 1530
*/
void timeToString(int32_t time_value, char *string)
{
	time_value = time_value % ((int32_t) 24 *60 * 60);
	int32_t hour = time_value / (60 * 60);
	
	time_value = time_value % (60 * 60);
	int16_t minute = time_value / (60);
	
	char ts[30];
	
	sprintf(ts, "%02d", (int8_t) hour);
	strncpy(string, ts, 2);
	
	sprintf(ts, "%02d", (int8_t) minute);
	strncpy(string + 2, ts, 2);
	
	string[4] = 0x00;
}

/*
Converts time from an integer to format like WED1530
*/
/*
void timeToString(int32_t time_value, char *string)
{
	// what day
	int32_t week_day = time_value / ((int32_t) 24 *60 * 60);

	time_value = time_value % ((int32_t) 24 *60 * 60);
	int32_t hour = time_value / (60 * 60);

	time_value = time_value % (60 * 60);
	int16_t minute = time_value / (60);

	strncpy(string, days(week_day), 3);

	char ts[30];

	sprintf(ts, "%02d", (int8_t) hour);
	strncpy(string + 3, ts, 2);

	sprintf(ts, "%02d", (int8_t) minute);
	strncpy(string + 5, ts, 2);

	string[7] = 0x00;
}
*/


/*
Converts name of the day to a number
for example: MON => 1
*/
int weekdays(char *day)
{
	if (strcmp(day, "MON") == 0)
	return 1;
	else if (strcmp(day, "TUE") == 0)
	return 2;
	else if (strcmp(day, "WED") == 0)
	return 3;
	else if (strcmp(day, "THU") == 0)
	return 4;
	else if (strcmp(day, "FRI") == 0)
	return 5;
	else if (strcmp(day, "SAT") == 0)
	return 6;
	else if (strcmp(day, "SUN") == 0)
	return 0;
	
	return 0;
}

/*
Converts number of day to its name
For example: 5 => FRI
*/
char * days(int weekday)
{
	if (weekday == 0)
	return "SUN";
	else if (weekday == 1)
	return "MON";
	else if (weekday == 2)
	return "TUE";
	else if (weekday == 3)
	return "WED";
	else if (weekday == 4)
	return "THU";
	else if (weekday == 5)
	return "FRI";
	else if (weekday == 6)
	return "SAT";
	
	return "unknown day";
}

/*
It converts time to format similar to milliseconds format.
But in this case it is "number of minutes from beginning of the week".
Resolution of the event is 1 minute.
*/
uint32_t current_time()
{
	struct Time time;
	get_time(&time);
	
	struct Date date;
	get_date(&date);
	
	// whole cycle is 7 days, resolution is 1 minute
	//return (date.week_day * 24 * 60) + (time.hour * 60 + time.minute);
	
	// whole cycle is 7 days, resolution is 1 second
	//return ((int32_t) date.week_day * 24 * 60 * 60) + ((int32_t)  time.hour * 60 * 60) + ((int32_t) time.minute * 60) + time.second;
	
	// whole cycle is 24 hours, resolution is 1 second
	return ((int32_t)  time.hour * 60 * 60) + ((int32_t) time.minute * 60) + time.second;
	
	// for tests: whole cycle is 60 minutes, resolution 1 second
	//return (time.minute * 60) + (time.second);
	
	// for tests: whole cycle is 60 seconds, resolution 1 second
	//return time.second;
}


void disable_clkout()
{
	// send data
	twi_start();
	twi_address_write(0xA2);
	
	// unset (clear) FE bit in register CLKOUT_control
	twi_send(0x0D);
	twi_send(0b00000000);
	
	twi_stop();
	_delay_ms(50);
}


/*
if VL bit is set, it means the back up power is discharged and is empty; it needs to replace backup battery.
*/
int get_vl()
{
	// PCF8563 - address for read: 0xA3, address for write: 0xA2
	
	twi_start();
	twi_address_write(0xA2);
	twi_send(0x02);
	
	twi_start();
	twi_address_read(0xA3);
	
	uint8_t received = twi_receive_last();
	twi_stop();
	
	if (received & (1 << VL_BIT))
	return 1;
	else
	return 0;
	
}

/*
reset VL bit
*/
void reset_vl()
{
	// PCF8563 - address for read: 0xA3, address for write: 0xA2
	
	twi_start();
	twi_address_write(0xA2);
	twi_send(0x02);
	twi_send(0x00);
	
	twi_stop();
}



void read_date()
{
	struct Date date;
	get_date(&date);
	
	char ts[30];
	send_string("date: ");
	sprintf(ts, "%d", date.year);
	send_string(ts);
	
	send_string("/");
	sprintf(ts, "%02d", date.month);
	send_string(ts);
	
	send_string("/");
	sprintf(ts, "%02d", date.day);
	send_string(ts);
	
	send_string(" ");
	char *td = days(date.week_day);
	send_string(td);
	
	send_string("\r\n");

}


