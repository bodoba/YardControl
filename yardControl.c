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
#include "yardControl.h"
#include "pushButton.h"
#include "readConfig.h"
#include <wiringPi.h>
#include <pcf8574.h>

#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ----------------------------------------------------------------------------------- *
 * Some globals we can't do without... ;)
 * ----------------------------------------------------------------------------------- */
int    debug              = DEBUG;             // debug level
int    activeSequence     = SEQUENCE;          // sequence to run
int    sequenceInProgress = false;
time_t sequenceStartTime;                      // time sequence was started

/* ----------------------------------------------------------------------------------- *
 * System modes
 * ----------------------------------------------------------------------------------- */
enum Modes { MANUAL_MODE, AUTOMATIC_MODE } systemMode;

/* ----------------------------------------------------------------------------------- *
 * Prototypes
 * ----------------------------------------------------------------------------------- */
void setup(void);
int  main(int rgc, char *argv[]);
void lockValveControl(bool on);

// Bush button actions
void setLed( pushbutton_t *button );
void switchValve( pushbutton_t *button );
void runSequence( pushbutton_t *button );
void selectSequence( pushbutton_t *button );
void automaticMode( pushbutton_t *button );

/* ----------------------------------------------------------------------------------- *
 * Definition of the pushbuttons
 * ----------------------------------------------------------------------------------- */
pushbutton_t pushButtons[] = {
    // Button Pin, Led Pin, state, last reading, locked, radio group
    
    // Manual valves control, only one shall be active
    {'A', BUTTON_A,      VALVE_A,  false, -1, false, RG_VALVES,   &switchValve},
    {'B', BUTTON_B,      VALVE_B,  false, -1, false, RG_VALVES,   &switchValve},
    {'C', BUTTON_C,      VALVE_C,  false, -1, false, RG_VALVES,   &switchValve},
    {'D', BUTTON_D,      VALVE_D,  false, -1, false, RG_VALVES,   &switchValve},

    {'S', BUTTON_SELECT, LED_S1,   false, -1, false, RG_NONE,     &selectSequence},
    {'R', BUTTON_RUN,    LED_RUN,  false, -1, false, RG_NONE,     &runSequence},

    {'P', BUTTON_AUTO,   LED_AUTO, false, -1, false, RG_NONE,     &automaticMode},
    
    // end marker
    {'0', -1, -1, false, -1, false, -1},
};

/* ----------------------------------------------------------------------------------- *
 * Enable/Disable manual valve control (radio group: RG_VALVES)
 * ----------------------------------------------------------------------------------- */
void lockValveControl (bool on ) {
    int btnIndex = 0;
    if ( !on ) {
        // disable manual valve control
        while ( pushButtons[btnIndex].btnPin >= 0 ) {
            if (pushButtons[btnIndex].radioGroup == RG_VALVES) {
                pushbutton_t *btnValve = &pushButtons[btnIndex];
                btnValve->locked = true;
                btnValve->state  = false;
                switchValve( btnValve );
            }
            btnIndex++;
        }
    } else {
        // enable manual valve control
        while ( pushButtons[btnIndex].btnPin >= 0 ) {
            if (pushButtons[btnIndex].radioGroup == RG_VALVES) {
                pushButtons[btnIndex].locked = false;
            }
            btnIndex++;
        }
    }
}

/* ----------------------------------------------------------------------------------- *
 * Switch Valve
 * ----------------------------------------------------------------------------------- */
void switchValve( pushbutton_t *button ) {
    setLed( button );
}

/* ----------------------------------------------------------------------------------- *
 * start sequence
 * ----------------------------------------------------------------------------------- */
void runSequence( pushbutton_t *button ) {
    setLed( button );

    // enable/disable manual valve control
    lockValveControl(!button->state);
    
    // enable/disable sequence change
    pushButtons[4].locked = button->state;
    
    if ( button->state && sequence[activeSequence][0].offset >=0 ) {
        // start sequence
        sequenceInProgress = true;
        int step = 0;
        while ( sequence[activeSequence][step].offset >= 0 ) {
            sequence[activeSequence][step].done = false;
            step++;
        }
        printf("** Start sequence %02d\n", activeSequence);
        sequenceStartTime = time(NULL);
    } else {
        // stop sequence
        sequenceInProgress = false;
        printf("** Stop sequence %02d\n", activeSequence);
        // switch all valves off
        int btnIndex = 0;
        while ( pushButtons[btnIndex].btnPin >= 0 ) {
            if (pushButtons[btnIndex].radioGroup == RG_VALVES) {
                pushbutton_t *btnValve = &pushButtons[btnIndex];
                btnValve->state  = false;
                switchValve( btnValve );
            }
            btnIndex++;
        }
    }
}

/* ----------------------------------------------------------------------------------- *
 * Select sequence to run
 * ----------------------------------------------------------------------------------- */
void selectSequence( pushbutton_t *button ) {
    digitalWrite ( LED_S1, button->state ? LOW : HIGH);
    digitalWrite ( LED_S2, button->state ? HIGH : LOW);
    activeSequence = button->state ? 1:0;
}

/* ----------------------------------------------------------------------------------- *
 * run in automatic mode
 * ----------------------------------------------------------------------------------- */
void automaticMode( pushbutton_t *button ) {
    setLed( button );

    // enable/disable manual valve control
    lockValveControl(!button->state);

    // enable/disable sequence change
    pushButtons[4].locked = button->state;

    // enable/disable sequence start
    pushButtons[5].state  = false;
    pushButtons[5].locked = button->state;
    digitalWrite (LED_RUN, HIGH);

    // set system mode
    systemMode = button->state ? AUTOMATIC_MODE:MANUAL_MODE;
}

/* ----------------------------------------------------------------------------------- *
 * Set LED of push button
 * ----------------------------------------------------------------------------------- */
void setLed( pushbutton_t *button ) {
    digitalWrite ( button->ledPin, button->state ? LOW : HIGH);
}

/* ----------------------------------------------------------------------------------- *
 * Initial setup
 * ----------------------------------------------------------------------------------- */
void setupIO ( void ) {
    // initialize wiring PI and attached IO extender
    wiringPiSetup () ;
    pcf8574Setup (PINBASE_0, ADDR_IOEXT_0);
    pcf8574Setup (PINBASE_1, ADDR_IOEXT_1);

    // setup pin modes for buttons
    int btnIndex = 0;    
    while ( pushButtons[btnIndex].btnPin >= 0 ) {
        pinMode(pushButtons[btnIndex].btnPin, INPUT);
        pinMode(pushButtons[btnIndex].ledPin, OUTPUT);
        digitalWrite(pushButtons[btnIndex].ledPin,
                     pushButtons[btnIndex].state ? LOW : HIGH);
        btnIndex++;
    }

    digitalWrite ( LED_S1, HIGH);
    digitalWrite ( LED_S2, LOW);
    
    systemMode = MANUAL_MODE;
}

/* ----------------------------------------------------------------------------------- *
 * Main
 * ----------------------------------------------------------------------------------- */
int main( int argc, char *argv[] ) {
    openlog(NULL, LOG_PID, LOG_USER);       // use syslog to create a trace
    
    // Process command line options
    for (int i=0; i<argc; i++) {
        if (!strcmp(argv[i], "-d")) {       // '-d' turns debug mode on
            debug++;
        }
        if (!strcmp(argv[i], "-c")) {       // '-c' specify configuration file name
            configFile = strdup(argv[++i]);
        }
    }
    
    // read configuration from file
    readConfig();

    if ( debug ) {
        // dump configuration
        dumpSequence( 0 );
        dumpSequence( 1 );
    }

    // Initialize IO ports
    setupIO();
    
    // Main loop
    time_t lastTime;
    for ( ;; ) {                            // never stop working
        time_t now = time(NULL);
        int lastStep = 0;
        // forward sequence
        if (sequenceInProgress) {
            int offset = (int)now-sequenceStartTime;
            if ( lastTime != now ) {
                int step = lastStep;
                while ( sequence[activeSequence][step].offset >= 0 ) {
                    sequence_t *seqStep = &sequence[activeSequence][step];

                    if (seqStep->offset <= offset && !seqStep->done) {
                        seqStep->done = true;                    // mark step as done
                        seqStep->valve->state = seqStep->state;  // Valve ON or OFF ?
                        switchValve(seqStep->valve);             // switch Valve
                        lastStep = step;                         // remember where we left off
                        
                        printf(" * S%02d:%02d t+%04d %c %s\n", activeSequence, step, offset,
                               seqStep->valve->name, seqStep->state? "ON":"OFF");

                        break;
                    } else {
                        printf(" * S%02d:%02d t+%04d Skipping...\n",a ctiveSequence, step, offset );
                    }
                    step++;
                }
                // sequence end?
                if (sequence[activeSequence][step].offset < 0) {
                    printf(" * S%02d:%02d t+%04d Last command\n", activeSequence, step, offset);
                    // simulate sequence button press
                    pushButtons[5].state=false;
                    runSequence( &pushButtons[5] );
                    lastStep = 0;
                }
            }
        }
        
        // process bush buttons
        int btnIndex = 0;
        while ( pushButtons[btnIndex].btnPin >= 0 ) {
            readButton(&pushButtons[btnIndex], pushButtons);
            btnIndex++;
        }
        delay(50);
        lastTime = now;
    }
    return 0;
}
