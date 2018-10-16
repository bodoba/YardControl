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
#include "pushButton.h"

#ifndef yardControl_h
#define yardControl_h
/* ----------------------------------------------------------------------------------- *
 * MPC23016 IO extender
 * ----------------------------------------------------------------------------------- */
#define ADDR_IOEXT_0   0x20
#define PINBASE_0        64

/* ----------------------------------------------------------------------------------- *
 * output pins for valve control
 * ----------------------------------------------------------------------------------- */
#define VALVE_A       PINBASE_0+8  // control valve A
#define VALVE_B       PINBASE_0+9  // control valve B
#define VALVE_C       PINBASE_0+10  // control valve C
#define VALVE_D       PINBASE_0+11 // control valve D

/* ----------------------------------------------------------------------------------- *
 * Output pins for control LEDs
 * ----------------------------------------------------------------------------------- */
#define LED_S2        PINBASE_0+12  // sequence 2 selected
#define LED_AUTO      PINBASE_0+13 // automatic mode enabled
#define LED_S1        PINBASE_0+14 // sequence 1 selected
#define LED_RUN       PINBASE_0+15 // selected sequence is running

/* ----------------------------------------------------------------------------------- *
 * input pins for push buttons
 * ----------------------------------------------------------------------------------- */
#define BUTTON_A      PINBASE_0+0  // switch valve A
#define BUTTON_B      PINBASE_0+1   // switch valve B
#define BUTTON_C      PINBASE_0+2  // switch valve C
#define BUTTON_D      PINBASE_0+3  // switch valve D

#define NC_1_4        PINBASE_0+4  // not connected
#define BUTTON_AUTO   PINBASE_0+5  // toggle automatic mode
#define BUTTON_SELECT PINBASE_0+6  // select between sequence 1 and 2
#define BUTTON_RUN    PINBASE_0+7  // run selected sequence

/* ----------------------------------------------------------------------------------- *
 * radio groups for bush buttons
 * ----------------------------------------------------------------------------------- */
#define RG_NONE       0
#define RG_VALVES     1
#define RG_SEQUENCES  2

/* ----------------------------------------------------------------------------------- *
 * Default valued for configurable parameter
 * ----------------------------------------------------------------------------------- */
#define DEBUG        0                           /* no debug info by default           */
#define SEQUENCE     0                           /* run sequence 0 by default          */

#define PID_FILE     "/var/run/yardcontrol.pid"

/* ----------------------------------------------------------------------------------- *
 * export some globals
 * ----------------------------------------------------------------------------------- */
extern pushbutton_t pushButtons[];
extern int debug;

#endif /* yardControl_h */
