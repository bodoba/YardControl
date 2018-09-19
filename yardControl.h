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
#include <stdtypes.h>
#include <wiringPi.h>
#include <pcf8574.h>
#include <time.h>

#define ADDR_IOEXT_0   0x38
#define PINBASE_0        64

#define VALVE_A  PINBASE_0+0
#define VALVE_B  PINBASE_0+1
#define VALVE_C  PINBASE_0+2
#define VALVE_D  PINBASE_0+3

#define BUTTON_A PINBASE_0+4
#define BUTTON_B PINBASE_0+5
#define BUTTON_C PINBASE_0+6
#define BUTTON_D PINBASE_0+7

typedef struct {
    int     btnPin;
    int     ledPin;
    boolean    status;
    boolean    last_status;
    boolean    locked;
    time_t  lastChange;
} pushbutton_t;


#endif /* yardControl_h */
