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

enum {HIGH_STATE = 1, LOW_STATE = 0};
enum {PWM0_PD6 = 0, PWM1_PD5 = 1, PWM2_PB1 = 2, PWM3_PD3 = 3, EXP0_PC2 = 4, EXP1_PD4 = 5, EXP2_PD7 = 6, EXP3_PB0 = 7};

// MeropeCentral
void print_events();
void reset();

// events.c
#define EVENT_MODE 0
#define MANUAL_MODE 1
void load_events();
void timer_event();
void clear_events();
void set_manual_mode();
void set_event_mode();
uint8_t get_event_mode();
void set_pin(uint8_t pin, uint8_t pin_state);
int what_pin(char *value);
void add_event(int pin, int32_t time, int pin_state);
void add_pwm_event(int pin, int32_t time, int pin_state, int inc, int duration);
void add_clouds_event(int pin, int32_t time, int pin_state, int duration, int final_state);

