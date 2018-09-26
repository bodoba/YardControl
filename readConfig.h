* *********************************************************************************** */
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

#ifndef readConfig_h
#define readConfig_h

#include <stdio.h>
#include "pushButton.h"

/* ----------------------------------------------------------------------------------- *
 * A step in a sequence
 * ----------------------------------------------------------------------------------- */
typedef struct sequence_t {
    int          offset;     // offset after sequence start this action shall be triggered
    pushbutton_t *valve;     // valve to be switched
    bool         state;      // new state of valve
} sequence_t;

/* ----------------------------------------------------------------------------------- *
 * Start time
 * ----------------------------------------------------------------------------------- */
typedef struct starttime_t {
    int tm_min;
    int tm_hour;
} starttime_t;

/* ----------------------------------------------------------------------------------- *
 * Some globals we can't do without
 * ----------------------------------------------------------------------------------- */
extern char *configFile    = CONFIG_FILE;     /* configuration file                    */
extern sequence_t  sequence[2][MAX_STEP];     /* two program sequences of max 40 steps */
extern starttime_t startTime[2];              /* start times for each sequence         */

/* ----------------------------------------------------------------------------------- *
 * Settings
 * ----------------------------------------------------------------------------------- */
#define MAX_STEP        100  // max 100 steps per sequence (50 commands)
#define TIME_SCALE        1  // unit scale fpr secuence, set to 60 to get minutes

/* ----------------------------------------------------------------------------------- *
 * Prototypes
 * ----------------------------------------------------------------------------------- */
bool readConfig (void);                  // read and parse config file
void dumpSequence(int sequenceIdx);      // dump configuration in config file format

#endif /* readConfig_h */
