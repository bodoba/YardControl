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
#define VALVE_A       PINBASE_0+0  // control valve A
#define VALVE_B       PINBASE_0+1  // control valve B
#define VALVE_C       PINBASE_0+2  // control valve C
#define VALVE_D       PINBASE_0+3  // control valve D

/* ----------------------------------------------------------------------------------- *
 * Output pins for control LEDs
 * ----------------------------------------------------------------------------------- */
#define LED_S2        PINBASE_1+0  // sequence 2 selected
#define LED_AUTO      PINBASE_1+1  // automatic mode enabled
#define LED_S1        PINBASE_1+2  // sequence 1 selected
#define LED_RUN       PINBASE_1+3  // selected sequence is running

/* ----------------------------------------------------------------------------------- *
 * input pins for push buttons
 * ----------------------------------------------------------------------------------- */
#define BUTTON_A      PINBASE_0+4   // switch valve A
#define BUTTON_B      PINBASE_0+5   // switch valve B
#define BUTTON_C      PINBASE_0+6   // switch valve C
#define BUTTON_D      PINBASE_0+7   // switch valve D

#define NC_1_4        PINBASE_1+4   // not connected
#define BUTTON_AUTO   PINBASE_1+5   // toggle automatic mode
#define BUTTON_SELECT PINBASE_1+6   // select between sequence 1 and 2
#define BUTTON_RUN    PINBASE_1+7   // run selected sequence

/* ----------------------------------------------------------------------------------- *
 * radio groups for bush buttons
 * ----------------------------------------------------------------------------------- */
#define RG_NONE       0
#define RG_VALVES     1
#define RG_SEQUENCES  2

#endif /* yardControl_h */
