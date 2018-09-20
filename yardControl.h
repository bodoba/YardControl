/* *********************************************************************************** */
/*                                                                                     */
/*  Copyright (c) 2018 by Bodo Bauer <bb@bb-zone.com>                                  */
/*                                                                                     */
/*  This program is free software: you can redistribute it and/or modify               */
/*  it under the terms of the GNU General Public License as published by               */
/*  the Free Software Foundation, either version 3 of the License, or                  */
/*  (at your option) any later version.                                                */
/*                                                                                     */
/*  This program is distributed in the hope that it will be useful,                    */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of                     */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                      */
/*  GNU General Public License for more details.                                       */
/*                                                                                     */
/*  You should have received a copy of the GNU General Public License                  */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/>.              */
/* *********************************************************************************** */

#ifndef yardControl_h
#define yardControl_h

#include <stdio.h>
#include <stdbool.h>
#include <wiringPi.h>
#include <pcf8574.h>
#include <time.h>

/* ----------------------------------------------------------------------------------- *
 * PCF8574 io extender
 * ----------------------------------------------------------------------------------- */
#define ADDR_IOEXT_0   0x38
#define PINBASE_0        64

#define ADDR_IOEXT_1   0x39
#define PINBASE_1        72

/* ----------------------------------------------------------------------------------- *
 * output pins for valve control
 * ----------------------------------------------------------------------------------- */
#define VALVE_A  PINBASE_0+0
#define VALVE_B  PINBASE_0+1
#define VALVE_C  PINBASE_0+2
#define VALVE_D  PINBASE_0+3

/* ----------------------------------------------------------------------------------- *
 * Output pins for control LEDs
 * ----------------------------------------------------------------------------------- */
#define NC_1_0   PINBASE_1+0
#define LED_P1   PINBASE_1+1
#define LED_P2   PINBASE_1+2
#define LED_AUTO PINBASE_1+3

/* ----------------------------------------------------------------------------------- *
 * input pins for push buttons
 * ----------------------------------------------------------------------------------- */
#define BUTTON_A    PINBASE_0+4
#define BUTTON_B    PINBASE_0+5
#define BUTTON_C    PINBASE_0+6
#define BUTTON_D    PINBASE_0+7

#define NC_1_4      PINBASE_1+4
#define BUTTON_P1   PINBASE_1+5
#define BUTTON_P2   PINBASE_1+6
#define BUTTON_AUTO PINBASE_1+7


/* ----------------------------------------------------------------------------------- *
 * Definition of a push button
 * ----------------------------------------------------------------------------------- */
typedef struct {
    int     btnPin;       // Input pin
    int     ledPin;       // LED indicating button state
    bool    state;        // button state
    int     lastReading;  // last button reading
    bool    locked;       // when locked, button value can't be changed manually
    int     radioGroup;   // >  0 defines radio button group
} pushbutton_t;

#endif /* yardControl_h */
