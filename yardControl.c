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
#include <wiringPi.h>
#include <pcf8574.h>

#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>

/* ----------------------------------------------------------------------------------- *
 * Some globals we can't do without... ;)
 * ----------------------------------------------------------------------------------- */
int  debug          = DEBUG;                     /* debug level                        */
int  activeSequence = SEQUENCE;                  /* sequence to run                    */
char *configFile    = CONFIG_FILE;               /* configuration file                 */

/* ----------------------------------------------------------------------------------- *
 * System modes
 * ----------------------------------------------------------------------------------- */
enum Modes { MANUAL_MODE, SEQUENCE_MODE, AUTOMATIC_MODE } systemMode;

/* ----------------------------------------------------------------------------------- *
 * Prototypes
 * ----------------------------------------------------------------------------------- */
void setup(void);
int  main( int rgc, char *argv[] );
void lockValveControl (bool on );
bool readConfig (void);

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
    {BUTTON_A,      VALVE_A,  false, -1, false, RG_VALVES,   &switchValve},
    {BUTTON_B,      VALVE_B,  false, -1, false, RG_VALVES,   &switchValve},
    {BUTTON_C,      VALVE_C,  false, -1, false, RG_VALVES,   &switchValve},
    {BUTTON_D,      VALVE_D,  false, -1, false, RG_VALVES,   &switchValve},

    {BUTTON_SELECT, LED_S1,   false, -1, false, RG_NONE,     &selectSequence},
    {BUTTON_RUN,    LED_RUN,  false, -1, false, RG_NONE,     &runSequence},

    {BUTTON_AUTO,   LED_AUTO, false, -1, false, RG_NONE,     &automaticMode},
    
    // end marker
    {-1, -1, false, -1, false, -1},
};

/* ----------------------------------------------------------------------------------- *
 * Read config file
 * ----------------------------------------------------------------------------------- */
char *nextValue( char **cursor) {
    while (**cursor && **cursor != ' ') (*cursor)++;                   /*   skip token */
    **cursor = '\0'; (*cursor)++;                                      /* end of token */
    while (**cursor && **cursor == ' ') (*cursor)++;                   /* skip spaces  */
    return *cursor;
}

bool readConfig(void) {
    FILE *fp = NULL;
    fp = fopen(configFile, "rb");
    int sequence = -1;
    
    if (fp) {
        char  *line=NULL;
        char  *cursor;
        size_t n=0;
        size_t length = getline(&line, &n, fp);
        
        while ( length != -1) {
            if ( length > 1 ) {                              /* skip empty lines       */
                cursor = line;
                if ( line[length-1] == '\n' ) {             /* remove trailing newline */
                    line[length-1] = '\0';
                }
                
                if ( *cursor != '#') {                          /* skip '#' comments   */
                    char *token = cursor;
                    char *value = nextValue(&cursor);
                    
                    if (!strcmp(token, "SEQUENCE")) {
                        if ( sequence >= 0 ) {
                            printf("SEQUENCE END %02d", sequence);
                        }
                        sequence++;
                        printf("SEQUENCE START %02d", sequence);
                    } else if (!strcmp(token, "VALVE")) {
                        char *valve = cursor;
                        char *time  = nextValue(&cursor);
                        printf( "VALVE %s on for %s minutes\n", valve, time);
                    }
                }
            }
            free(line);
            n=0;
            length = getline(&line, &n, fp);
        }
        fclose(fp);
    }
    return true;
}



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

    systemMode = button->state ? SEQUENCE_MODE:MANUAL_MODE;
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
    openlog(NULL, LOG_PID, LOG_USER);       /* use syslog to create a trace            */
    
    /* ------------------------------------------------------------------------------- */
    /* Process command line options                                                    */
    /* ------------------------------------------------------------------------------- */
    
    /* FIXME: Use getopt_long and provide some help to the user just in case...        */
    for (int i=0; i<argc; i++) {
        if (!strcmp(argv[i], "-d")) {       /* '-d' turns debug mode on                */
            debug++;
        }
        if (!strcmp(argv[i], "-c")) {       /* '-c' specify configuration file         */
            configFile = strdup(argv[++i]);
        }
    }
    readConfig();
    
    setupIO();                              /* initialize IO ports                     */
    
    for ( ;; ) {                            /* never end working                       */
        // process bush buttons
        int btnIndex = 0;
        while ( pushButtons[btnIndex].btnPin >= 0 ) {
            readButton(&pushButtons[btnIndex], pushButtons);
            btnIndex++;
        }
        delay(50);
    }
    return 0;
}
