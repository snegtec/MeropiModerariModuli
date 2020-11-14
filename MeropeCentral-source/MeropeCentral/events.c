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

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>

#include "all_imports.h"
#include "utils_time.h"
#include "utils_com.h"
#include "utils_pwm.h"
#include "utils.h"

// event type
enum {EXP, PWM, CLOUDS};

/*
Important!
Clear all events (in EEPROM) after changing format of this struct!
*/
struct Event
{
	// pin number of port (for relay and pwm)
	// exact number of pin is declared below
	uint8_t pin;
	int32_t time;
	uint8_t type;
	
	// disable == 0, enable == 1; for pwm events it is start value
	int8_t pin_state; 
	
	// how much time it has to take dimming or lighting
	int32_t duration; 
	
	// how much increment or decrement value for specified time (in time field)
	int8_t inc; 
	
	int32_t temp_time;
	int32_t temp_duration;
	
	// final pin_state
	uint8_t final_state; 
	
};

#define EVENTS_SIZE 20

EEMEM struct Event events_ee[EVENTS_SIZE];

// how many events are saved
EEMEM uint8_t events_count_ee;

// ram versions
struct Event events[EVENTS_SIZE];
uint8_t events_count = -1;

// merope-central outputs/inputs
// pd 3, 4, 5, 6, 7, pb 0, 1, pc 2
// these are indices in actual_events[] array

// number of event for specified pin; which is currently active
enum {PIN_DISABLED = -2, NO_EVENT_YET = -1};

int8_t actual_events[8] = {PIN_DISABLED, PIN_DISABLED, PIN_DISABLED, PIN_DISABLED, PIN_DISABLED, PIN_DISABLED, PIN_DISABLED, PIN_DISABLED};

uint8_t event_mode = EVENT_MODE;

uint8_t get_event_mode() {
	return event_mode;
}

void set_manual_mode()
{
	event_mode = MANUAL_MODE;
}

void set_event_mode()
{
	event_mode = EVENT_MODE;
	load_events();
}

int what_pin(char *value)
{
	if (strcmp(value, "EXP0") == 0)
	return EXP0_PC2;
	else if (strcmp(value, "EXP1") == 0)
	return EXP1_PD4;
	else if (strcmp(value, "EXP2") == 0)
	return EXP2_PD7;
	else if (strcmp(value, "EXP3") == 0)
	return EXP3_PB0;
	else if (strcmp(value, "PWM0") == 0)
	return PWM0_PD6;
	else if (strcmp(value, "PWM1") == 0)
	return PWM1_PD5;
	else if (strcmp(value, "PWM2") == 0)
	return PWM2_PB1;
	else if (strcmp(value, "PWM3") == 0)
	return PWM3_PD3;
	
	return -1;
}

void add_event(int pin, int32_t time, int pin_state)
{
	// don't add if no space
	if (events_count >= EVENTS_SIZE - 1)
		return;
	
	events[events_count].pin = pin;
	events[events_count].time = time;
	events[events_count].pin_state = pin_state;
	
	switch(pin)
	{
		case EXP0_PC2:
		case EXP1_PD4:
		case EXP2_PD7:
		case EXP3_PB0:
		events[events_count].type = EXP;
		break;
		
		case PWM0_PD6:
		case PWM1_PD5:
		// not implemented yet case PWM2_PB1:
		// not implemented yet case PWM3_PD3:
		events[events_count].type = PWM;
		break;
		
		default:break;
	}

	events_count++;
	
	// save event to eeprom
	eeprom_busy_wait();
	eeprom_write_block(&events, &events_ee, sizeof(struct Event) * EVENTS_SIZE);
	
	// save count
	eeprom_busy_wait();
	eeprom_write_byte(&events_count_ee, events_count);
}

/*
Duration in minutes
*/
void add_pwm_event(int pin, int32_t time, int pin_state, int inc, int duration)
{
	// don't add if no place
	if (events_count >= EVENTS_SIZE - 1)
	return;
	
	events[events_count].pin = pin;
	events[events_count].time = time;
	events[events_count].pin_state = pin_state;
	events[events_count].inc = inc;
	events[events_count].duration = duration * 60;
	
	events[events_count].type = PWM;
	
	events_count++;
	
	// save event to eeprom
	eeprom_busy_wait();
	eeprom_write_block(&events, &events_ee, sizeof(struct Event) * EVENTS_SIZE);
	
	// save count
	eeprom_busy_wait();
	eeprom_write_byte(&events_count_ee, events_count);
}

/*
Duration in minutes
*/
void add_clouds_event(int pin, int32_t time, int pin_state, int duration, int final_state)
{
	// don't add if no place
	if (events_count >= EVENTS_SIZE - 1)
	return;
	
	events[events_count].pin = pin;
	events[events_count].time = time;
	events[events_count].pin_state = pin_state;
	events[events_count].duration = duration * 60; // convert to seconds
	events[events_count].final_state = final_state;
	
	events[events_count].type = CLOUDS;
	
	events_count++;
	
	// save event to eeprom
	eeprom_busy_wait();
	eeprom_write_block(&events, &events_ee, sizeof(struct Event) * EVENTS_SIZE);
	
	// save count
	eeprom_busy_wait();
	eeprom_write_byte(&events_count_ee, events_count);
}

/*
Search through events table and check are there any events for specified pin
if yes, change PIN_DISABLED => NO_EVENT_YET
*/
void prepare_actual_events()
{
	for (uint8_t a = 0; a < sizeof(actual_events); a++)
	{
		for (uint8_t i = 0; i < events_count; i++)
		{
			if (a == events[i].pin)
			{
				//change: PIN_DISABLED => NO_EVENT_YET
				actual_events[a] = NO_EVENT_YET;
				break;
			}
		}
	}
}

void add_default_events()
{
	struct Event tea[20];
	
	//***
	// clouds events
	
	// 0 event
	tea[0].pin = PWM1_PD5;
	tea[0].time = 0;
	tea[0].pin_state = 0;
	tea[0].type = PWM;
	
	// 1 event
	tea[1].pin = PWM1_PD5;
	tea[1].time = 5;
	tea[1].pin_state = 20;
	tea[1].duration = 50;
	tea[1].final_state = 20;
	tea[1].type = CLOUDS;
	
	// 2 event
	tea[2].pin = PWM1_PD5;
	tea[2].time = 55;
	tea[2].pin_state = 0;
	tea[2].type = PWM;
	
	// save event
	eeprom_busy_wait();
	eeprom_write_block(&tea, &events_ee, sizeof(struct Event) * EVENTS_SIZE);
	
	// save count
	events_count = 3;
	eeprom_busy_wait();
	eeprom_write_byte(&events_count_ee, events_count);
}

void print_event(uint8_t event_number)
{
	struct Event event = events[event_number];
	
	// print pin
	switch (event.pin)
	{
		case PWM0_PD6:
		send_string(" PWM0_PD6 ");
		break;
		
		case PWM1_PD5:
		send_string(" PWM1_PD5 ");
		break;
		
		case PWM2_PB1:
		send_string(" PWM2_PB1 ");
		break;
		
		case PWM3_PD3:
		send_string(" PWM3_PD3 ");
		break;
		
		case EXP0_PC2:
		send_string(" EXP0_PC2 ");
		break;
		
		case EXP1_PD4:
		send_string(" EXP1_PD4 ");
		break;
		
		case EXP2_PD7:
		send_string(" EXP2_PD7 ");
		break;
		
		case EXP3_PB0:
		send_string(" EXP3_PB0 ");
		break;
		
		default:
		send_string(" unknown pin ");
		break;
	}
	
	if (event.type == EXP)
	send_string("EXP ");
	else if (event.type == PWM)
	send_string("PWM ");
	else if (event.type == CLOUDS)
	send_string("CLOUDS ");
	
	// event_time
	char formatted_date[30];
	timeToString(event.time, formatted_date);
	send_string(formatted_date);
	send_string(" ");
	
	if (event.type == EXP)
	{
		if (event.pin_state == 1)
		{
			// simple event
			send_string("HIGH ");
			
		} else if (event.pin_state == 0)
		{
			send_string("LOW ");
		}
		
	}
	else if (event.type == PWM)
	{
		// Note:
		// the current value is displayed in utils_pwm => set_pwm0() which is run in run_pwm().
		
		//TODO - it always shows 0%
		// it is because event=>pin_state is just the initial value not the current pin state!
		send_int(event.pin_state);
		send_string("% ");
		
		if ((event.inc != 0) & (event.duration != 0))
		{
			send_int(event.inc);
			send_string("%/");
			send_int(event.duration / 60);
			send_string("[m] ");
		}
	}
	else if (event.type == CLOUDS)
	{
		send_int(event.pin_state);
		send_string("% ");
		
		send_int(event.inc);
		send_string("%/");
		send_int(event.temp_duration);
		send_string("[s] ");
	}
	
	send_enter();
}


void print_events()
{
	char ts[30];
	
	send_string("Number of events: ");
	sprintf(ts, "%d", events_count);
	send_string(ts);
	send_enter();
	
	for (uint8_t i = 0; i < events_count; i++)
	{
		print_event(i);
	}
	
}


void clear_events()
{
	events_count = 0;
	
	// clear all data
	for (int i = 0; i < EVENTS_SIZE; i++)
	{
		events[i].pin = -1;
		events[i].time = 0;
		events[i].pin_state = 0;
		events[i].inc = 0;
		events[i].duration = 0;
		events[i].type = 0;
		
	}
	
	// save event to eeprom
	eeprom_busy_wait();
	eeprom_write_block(&events, &events_ee, sizeof(struct Event) * EVENTS_SIZE);
	
	// save count
	eeprom_busy_wait();
	eeprom_write_byte(&events_count_ee, events_count);
	
	event_mode = MANUAL_MODE;
}

void run_pwm(uint8_t pin, struct Event *event)
{
	int32_t time = current_time();
	
	if (pin == PWM0_PD6)
	{
		if (event->type == CLOUDS)
		{
			//***
			// CLOUDS mode
			
			int32_t subevent_passed_time = time - event->temp_time;
			
			// new cloud: if no cloud before (new event) or passed time of last cloud
			if ((event->temp_duration == 0) | (subevent_passed_time > event->temp_duration))
			{
				
				if (subevent_passed_time > event->temp_duration)
				{
					if (event->temp_duration == 0)
					send_string("new cloud - beginning ");
					else
					send_string("new cloud ");
					
					// start value for new subevent
					event->pin_state = event->pin_state + event->inc;
				}
				
				// start time for subevent
				event->temp_time = time;
				
				// if last cloud
				int32_t margin = 200; // 200 seconds before end of the duration of CLOUDS event
				if (((event->time + event->duration) - time) <= margin)
				{
					// last cloud
					send_string("last cloud ");
					
					event->temp_duration = (event->time + event->duration) - time;
					
					// final pin_state
					event->inc = event->final_state - event->pin_state;
				}
				else
				{
					event->temp_duration = util_rand2(10, margin);
					
					// range of changes: 70..100%
					int new_state = util_rand2(70, 100);
					
					event->inc = new_state - event->pin_state;
				}
			}
			
			// start value
			// convert % => 0..255
			uint8_t ti = (event->pin_state * 255) / 100;
			
			// increment/decrement value
			// convert % => 0..255
			int16_t tinc = (event->inc * 255) / 100;
			
			if (subevent_passed_time <= event->temp_duration)
			{
				ti = ti + ((int32_t) tinc * subevent_passed_time) / event->temp_duration;
			}
			else
			{
				// correction
				// 50%/15s = 3.3%/s (3%/s) => 15s * 3% = 45% => 50% - 45% = 5% of error
				ti = ti + tinc;
			}
			
			set_pwm0(ti);
		}
		else if (event->type == PWM)
		{
			// start value
			// convert % => 0..255
			uint8_t ti = (event->pin_state * 255) / 100;
			
			if (event->duration != 0)
			{
				// increment/decrement value
				// convert % => 0..255
				int16_t tinc = (event->inc * 255) / 100;
				
				int32_t passed_time = time - event->time;
				
				if (passed_time <= event->duration)
				{
					ti = ti + ((int32_t) tinc * passed_time) / event->duration;
				}
				else
				{
					// correction
					// 50%/15s = 3.3%/s (3%/s) => 15s * 3% = 45% => 50% - 45% = 5% of error
					ti = ti + tinc;
				}
			}
			
			set_pwm0(ti);
		}
	}
	else if (pin == PWM1_PD5)
	{
		if (event->type == CLOUDS)
		{
			//***
			// CLOUDS mode
			int32_t subevent_passed_time = time - event->temp_time;
			
			// new cloud: if no cloud before (new event) or passed time of last cloud
			//if ((event->temp_time == 0) | (event->temp_time < time))
			if ((event->temp_duration == 0) | (subevent_passed_time > event->temp_duration))
			{
				
				if (subevent_passed_time > event->temp_duration)
				{
					if (event->temp_duration == 0)
					send_string("new cloud - beginning ");
					else
					send_string("new cloud ");
					
					// start value for new subevent
					event->pin_state = event->pin_state + event->inc;
				}
				
				// start time for subevent
				event->temp_time = time;
				
				// if last cloud
				int32_t margin = 200; // 200 seconds before end of the duration of CLOUDS event
				if (((event->time + event->duration) - time) <= margin)
				{
					// last cloud
					send_string("last cloud ");
					
					event->temp_duration = (event->time + event->duration) - time;
					
					// final pin_state
					event->inc = event->final_state - event->pin_state;
				}
				else
				{
					event->temp_duration = util_rand2(10, margin);
					
					// range of changes: 70..100%
					int new_state = util_rand2(70, 100);
					
					event->inc = new_state - event->pin_state;
				}
				
			}
			
			// start value
			// convert % => 0..255
			uint8_t ti = (event->pin_state * 255) / 100;
			
			// increment/decrement value
			// convert % => 0..255
			int16_t tinc = (event->inc * 255) / 100;
			
			if (subevent_passed_time <= event->temp_duration)
			{
				ti = ti + ((int32_t) tinc * subevent_passed_time) / event->temp_duration;
			}
			else
			{
				// correction
				// 50%/15s = 3.3%/s (3%/s) => 15s * 3% = 45% => 50% - 45% = 5% of error
				ti = ti + tinc;
			}
			
			set_pwm1(ti);
		}
		else if (event->type == PWM)
		{
			// start value
			// convert % => 0..255
			uint8_t ti = (event->pin_state * 255) / 100;
			
			if (event->duration != 0)
			{
				// increment/decrement value
				// convert % => 0..255
				int16_t tinc = (event->inc * 255) / 100;
				
				int32_t passed_time = time - event->time;
				
				if (passed_time <= event->duration)
				{
					ti = ti + ((int32_t) tinc * passed_time) / event->duration;
				}
				else
				{
					// correction
					// 50%/15s = 3.3%/s (3%/s) => 15s * 3% = 45% => 50% - 45% = 5% of error
					ti = ti + tinc;
				}
			}
			
			set_pwm1(ti);
		}
	}
}

void set_event(uint8_t event_number)
{
	uint8_t which_pin = events[event_number].pin;
	
	actual_events[which_pin] = event_number;
	
	switch(which_pin)
	{
		case EXP0_PC2:
		case EXP1_PD4:
		case EXP2_PD7:
		case EXP3_PB0:
		set_pin(which_pin, events[event_number].pin_state);
		break;
		
		case PWM0_PD6:
		case PWM1_PD5:
		// not implemented yet case PWM2_PB1:
		// not implemented yet case PWM3_PD3:
		run_pwm(which_pin, &events[event_number]);
		break;
		
		default:break;
	}
	
	int32_t time = current_time();
	
	// event_time
	char formatted_date[30];
	timeToString(time, formatted_date);
	send_string(formatted_date);
	send_string(" ");
	
	print_event(event_number);
}

void get_event_indices(uint8_t indices[], uint8_t *indices_count, uint8_t pin)
{
	*indices_count = 0;
	
	for (uint8_t i = 0; i < events_count; i++)
	{
		if (events[i].pin == pin)
		{
			indices[*indices_count] = i;
			(*indices_count)++;
		}
	}
}

void current_state()
{
	if (event_mode == MANUAL_MODE)
	return;
	
	// get current time
	int32_t time = current_time();
	
	// indices for every pin
	uint8_t indices_count = 0;
	uint8_t event_indices[EVENTS_SIZE];
	
	for (uint8_t a = 0; a < sizeof(actual_events); a++)
	{
		if (actual_events[a] != PIN_DISABLED)
		{
			get_event_indices(event_indices, &indices_count, a);
			
			// if no events or just one event
			if (indices_count == 0)
			continue;
			
			// if earlier than the first event => current state is the last event
			if (time < events[event_indices[0]].time)
			{
				if ((events[event_indices[indices_count - 1]].duration > 0) | (actual_events[a] != event_indices[indices_count - 1]))
				set_event(event_indices[indices_count - 1]);
			}
			// if last event or later
			else if (time >= events[event_indices[indices_count - 1]].time)
			{
				if ((events[event_indices[indices_count - 1]].duration > 0) | (actual_events[a] != event_indices[indices_count - 1]))
				set_event(event_indices[indices_count - 1]);
			}
			else
			{
				// find events in middle
				for (uint8_t i = 1; i < indices_count; i++)
				{
					if ((time >= events[event_indices[i - 1]].time) & (time < events[event_indices[i]].time))
					{
						if ((events[event_indices[i - 1]].duration > 0) | (actual_events[a] != event_indices[i - 1]))
						set_event(event_indices[i - 1]);
					}
				}
			}
		}
	}
}

void load_events()
{
	// initialize random number generator; it has to be done only once (after reset)
	srand(current_time());
	
	events_count = eeprom_read_byte(&events_count_ee);
	
	// initialize - for testing purposes
	/*
	if (events_count == 0)
	{
	send_string("add_default\r\n");
	add_default_events();
	}
	*/
	
	if (events_count == 0)
	{
		send_line("No events => MANUAL_MODE");
		event_mode = MANUAL_MODE;
		return;
	}
	
	// load events into ram
	eeprom_read_block(&events, &events_ee, sizeof(struct Event) * EVENTS_SIZE);
	
	// prepare
	prepare_actual_events();
	
	// print events
	print_events();
	
	// check current state of pins
	// if the device is power off, and later power on - it doesn't know what should be current state of pins
	
	int32_t time = current_time();
	send_string(" start time: ");
	
	char formatted_date[30];
	timeToString(time, formatted_date);
	send_string(formatted_date);
	send_enter();
	
	current_state();
}

/*
Additional timer event
*/
void timer_event()
{
	if (event_mode == MANUAL_MODE) {
		return;
	}

	// for pwm time dependent events - always keep up to date
	current_state();
}


