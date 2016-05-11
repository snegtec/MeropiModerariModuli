/*****************************************************************************
*
* MEROPI MODERARI MODULI
* a simple microcontroller system for controlling light output of linear 
* fluorescent tube according to saved program
* 
* version: 0.1 (December 2014)
* author: Jacek Szymoniak
*   snegtec.com
*   snegtec@outlook.com
* 
* Copyright (c) 2014-2016 Jacek Szymoniak
* License:
*   application, source code - GNU General Public License
*   hardware (schematics, pcb design) - Creative Commons Attribution 
*   Share-Alike license (CC BY-SA 4.0)
* 
****************************************************************************/

Used tools
* compiler: Atmel Studio 6
* application for pcb design: Eagle 6.5.0.

Notes
* this is a beta version!
* pcb for power and relay modules are one-sided; you can create it cheaper than 2 sided versions.

Important note!
* The power module use 230V. There could requirements in your country about design and electrical rules for design of such devices. If you are not confident when working with such voltages don't use the power module. Use ready power supply. 

TODO
* it is better to use external oscilator
* it would be good idea to add socket for serial (txd, rxd); for example during reset Bluetooth connection is lost; it is not a problem in final, production environment but during debug it bothers;
* holes for mounting the boards could have bigger diameter
* cloud events hasn't been tested too carefully; only PWM1 pin has support for cloud events
* no support for daylight saving time 
* add more meaningful captions for elements on pcb boards; like 5V or 3,3V instead of X5 or X3;

Bugs
* sometimes there is a problem with transmission when AT+TIME? is sent to the module; probably twi and serial communication interfere in some way with each other; it also can stop the events from working; so currently - don't use AT+TIME? command in production environment

Bootloader information
* My version of module uses ATMEL AVR UART Bootloader for AVR-GCC/avr-libc. It is not needed for modules to work. But it can be very helpful to update software when a final module is mounted in a place where there is difficult access.

* MMM software has support for butterfly compatible programmer. It allows for touchless programming of the module. No need to press any button before programming the device. Programmer needs just to use Bluetooth serial port. You start programming, the module resets and bootloader loads into memory and loads new program to memory. More info in utils_com.c file on line 536.

More information about bootloader:
* ATMEL AVR UART Bootloader for AVR-GCC/avr-libc
* by Martin Thomas, Kaiserslautern, Germany
* mthomas@rhrk.uni-kl.de
* eversmith@heizung-thomas.de
* http://www.siwawi.arubi.uni-kl.de/avr_projects


/****************************************************************************/



