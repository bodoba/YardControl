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
#include "logging.h"

#include <wiringPi.h>
#include <pcf8574.h>

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>


/* ----------------------------------------------------------------------------------- *
 * Some globals we can't do without... ;)
 * ----------------------------------------------------------------------------------- */
int    debug              = DEBUG;             // debug level
int    activeSequence     = SEQUENCE;          // sequence to run
bool   foreground         = false;             // run in foreground, not as daemon
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
void processSequence(void);

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
        writeLog(LOG_INFO, "** Start sequence %02d", activeSequence);
        sequenceStartTime = time(NULL);
    } else {
        // stop sequence
        sequenceInProgress = false;
        writeLog(LOG_INFO, "** Stop sequence %02d", activeSequence);
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
 * process active sequence
 * ----------------------------------------------------------------------------------- */
void processSequence() {
    static int lastStep =0;
    int offset = (int)time(NULL)-sequenceStartTime;
    int step = lastStep;

    while ( sequence[activeSequence][step].offset >= 0 ) {
        sequence_t *seqStep = &sequence[activeSequence][step];
        
        if (seqStep->offset <= offset && !seqStep->done) {
            seqStep->done = true;                    // mark step as done
            seqStep->valve->state = seqStep->state;  // Valve ON or OFF ?
            switchValve(seqStep->valve);             // switch Valve
            lastStep = step;                         // remember where we left off
            
            writeLog(LOG_INFO, " * S%02d:%02d t+%04d %c %s", activeSequence, step, offset,
                   seqStep->valve->name, seqStep->state? "ON":"OFF");
            
            break;                                   // we're done here for now
        } else if (seqStep->offset > offset) {       // skip the future
            break;
        }
        step++;
    }
    
    // end of sequence reached?
    if (sequence[activeSequence][step].offset < 0) {
        pushButtons[5].state=false;                 // simulate sequence button press
        runSequence( &pushButtons[5] );
        lastStep = 0;
    }
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
    
    // Process command line options
    for (int i=0; i<argc; i++) {
        if (!strcmp(argv[i], "-d")) {          // '-d' turns debug mode on
            debug++;
        }
        if (!strcmp(argv[i], "-c")) {          // '-c' specify configuration file name
            configFile = strdup(argv[++i]);
        }
        if (!strcmp(argv[i], "-f")) {          // '-f' forces forground mode
            foreground=true;;
        }
    }
    
    initLog(!foreground);
    
    // read configuration from file
    readConfig();

    if ( debug ) {
        // dump configuration
        dumpSequence( 0 );
        dumpSequence( 1 );
    }

    // Deamonize
    if (!foreground) {
        /* If we got a good PID, then we can exit the parent process                   */
        pid_t pid = fork();
        if (pid < 0) {
            exit(EXIT_FAILURE);
        } else  if (pid > 0) {
            exit(EXIT_SUCCESS);
        }
        
        umask(0);                           /* Change the file mode mask               */
        pid_t sid = setsid();               /* Create a new SID for the child process  */
        if (sid < 0) {
            writeLog(LOG_ERR, "Could not get SID");
            exit(EXIT_FAILURE);
        }
        
        if ((chdir("/tmp")) < 0) {          /* Change the current working directory    */
            writeLog(LOG_ERR, "Could not chage working dir to /tmp");
            exit(EXIT_FAILURE);
        }
        
        /* use /dev/null for the standard file descriptors                             */
        int fd = open("/dev/null", O_RDWR); /* Open /dev/null as STDIN                 */
        dup(fd);                            /* STDOUT to /dev/null                     */
        dup(fd);                            /* STDERR to /dev/null                     */
    } else {
        writeLog(LOG_INFO, "Running in foreground");
    }
    
    // Initialize IO ports
    setupIO();
    
    // Main loop
    time_t lastTime = 0;
    for ( ;; ) {                              // never stop working
        time_t now = time(NULL);

        if ( lastTime != now ) {              // do once a second
            lastTime = now;
            if (sequenceInProgress) {         // forward sequence
                processSequence();
            }
        }
        
        pollButtons(pushButtons);             // poll bush buttons

        delay(50);                            // have a rest
    }
    return 0;
}
