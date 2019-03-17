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

#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <time.h>

#include <wiringPi.h>
#include <mcp23017.h>

#include "yardControl.h"
#include "pushButton.h"
#include "readConfig.h"
#include "logging.h"
#include "daemon.h"
#include "mqttGateway.h"

/* ----------------------------------------------------------------------------------- *
 * Some globals we can't do without... ;)
 * ----------------------------------------------------------------------------------- */
int    debug              = DEBUG;             // debug level
int    activeSequence     = SEQUENCE;          // sequence to run
bool   foreground         = false;             // run in foreground, not as daemon
int    sequenceInProgress = false;             // sequence in progress
time_t sequenceStartTime;                      // time sequence was started
int    systemMode         = MANUAL_MODE;       // System modes

/* ----------------------------------------------------------------------------------- *
 * Prototypes
 * ----------------------------------------------------------------------------------- */
int  main(int rgc, char *argv[]);
void lockValveControl(bool on);
void processSequence(void);
void setup(void);

// Bush button actions
void setLed(pushbutton_t *button);
void switchValve(pushbutton_t *button);
void startSequence(pushbutton_t *button);
void selectSequence(pushbutton_t *button);
void automaticMode(pushbutton_t *button);

// MQTT interface
void pressButtonCB(char *payload, int payloadlen, char *topic, void *button);
void publishStatus(pushbutton_t *button);

/* ----------------------------------------------------------------------------------- *
 * Definition of the pushbuttons
 * ----------------------------------------------------------------------------------- */
pushbutton_t pushButtons[] = {
    // name, Button Pin, Led Pin, state, last reading, locked, radio group
    
    // Manual valves control, only one shall be active
    {'A', BUTTON_A,      VALVE_A,  false, -1, false, RG_VALVES,   &switchValve},
    {'B', BUTTON_B,      VALVE_B,  false, -1, false, RG_VALVES,   &switchValve},
    {'C', BUTTON_C,      VALVE_C,  false, -1, false, RG_VALVES,   &switchValve},
    {'D', BUTTON_D,      VALVE_D,  false, -1, false, RG_VALVES,   &switchValve},

    // select active program sequence
    {'S', BUTTON_SELECT, LED_S0,   false, -1, false, RG_NONE,     &selectSequence},

    // run active program sequence
    {'R', BUTTON_RUN,    LED_RUN,  false, -1, false, RG_NONE,     &startSequence},

    // toggle timer mode
    {'P', BUTTON_AUTO,   LED_AUTO, false, -1, false, RG_NONE,     &automaticMode},
    
    // end marker
    {'0', -1, -1, false, -1, false, -1},
};
#define BUTTON_IDX_SELECT 4
#define BUTTON_IDX_RUN    5
#define BUTTON_IDX_TIMER  6

/* ----------------------------------------------------------------------------------- *
 * Enable/Disable manual valve control (radio group: RG_VALVES)
 * ----------------------------------------------------------------------------------- */
void lockValveControl (bool on ) {
    int btnIndex = 0;
    if ( !on ) {
        writeLog(LOG_INFO, "Lock manual valve control");
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
        writeLog(LOG_INFO, "Unlock manual valve control");
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
 * Publish button status
 * ----------------------------------------------------------------------------------- */
void publishStatus(pushbutton_t *button) {
    char topic[strlen(mqttBroker.prefix)+12], message[32];
    sprintf(message, "{\"state\":\"%s\"}", button->state ? "ON" : "OFF");
    sprintf(topic,   "%s/Valve_%c", mqttBroker.prefix, button->name);
    mqttPublish(topic, message);
}

/* ----------------------------------------------------------------------------------- *
 * Switch Valve
 * ----------------------------------------------------------------------------------- */
void switchValve( pushbutton_t *button ) {
    writeLog ( LOG_INFO, "Turn valve %c %s", button->name, button->state? "ON":"OFF" );
    // led is conntected to valve
    setLed( button );
    publishStatus(button);
}

/* ----------------------------------------------------------------------------------- *
 * Switch Valve with MQTT command
 * ----------------------------------------------------------------------------------- */
void pressButtonCB(char *payload, int payloadlen, char *topic, void *user_data) {
    pushbutton_t *button = (pushbutton_t*)user_data;
    // writeLog(LOG_INFO, "Received MQTT message: %s: %s", topic, payload);
    if (button->locked) {             // Do not allow changes of locked buttons over MQTT
        // writeLog(LOG_INFO, "Button %c locked!", button->name);
        publishStatus(button);
    } else {
        bool oldState = button->state;
        if (!strncmp(payload, "{\"state\":\"ON\"}", payloadlen) || !strncmp(payload, "{\"state\":\"1\"}", payloadlen)){
            button->state = true;
        } else if (!strncmp(payload, "{\"state\":\"OFF\"}", payloadlen) || !strncmp(payload, "{\"state\":\"0\"}", payloadlen)){
            button->state = false;
        } else {
            writeLog(LOG_ERR, "Received unknown MQTT message: %s", payload);
        }
        if (button->state != oldState) {
            // if a radio group has been defined clear state of all buttons in this group
            processRadioGroup( button, pushButtons);
            // call button action
            if (button->callback) {
                (button->callback)(button);
            }
        }
    }
}

/* ----------------------------------------------------------------------------------- *
 * start sequence
 * ----------------------------------------------------------------------------------- */
void startSequence( pushbutton_t *button ) {
    setLed(button);
    publishStatus(button);

    if ( systemMode == MANUAL_MODE ) {
        // enable/disable manual valve control
        lockValveControl(!button->state);
    
        // enable/disable sequence change
        pushButtons[4].locked = button->state;
    }
    
    if ( button->state && sequence[activeSequence][0].offset >=0 ) {
        writeLog(LOG_INFO, "Start sequence %02d", activeSequence);
        sequenceInProgress = true;            // start sequence
        int step = 0;
        while ( sequence[activeSequence][step].offset >= 0 ) {
            sequence[activeSequence][step].done = false;
            step++;
        }
        sequenceStartTime = time(NULL);
    } else {
        writeLog(LOG_INFO, "Stop sequence %02d", activeSequence);
        sequenceInProgress = false;           // stop sequence processing
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
    digitalWrite ( LED_S0, button->state ? LOW : HIGH);
    digitalWrite ( LED_S1, button->state ? HIGH : LOW);
    activeSequence = button->state ? 1:0;
    publishStatus(button);
    writeLog(LOG_INFO,"Activated Sequence %d", button->state ? 1:0);
}

/* ----------------------------------------------------------------------------------- *
 * run in timer mode
 * ----------------------------------------------------------------------------------- */
void automaticMode( pushbutton_t *button ) {
    setLed( button );
    publishStatus(button);

    // enable/disable sequence start
    pushButtons[BUTTON_IDX_RUN].state  = false;
    startSequence( &pushButtons[BUTTON_IDX_RUN] );       // stop sequence in progress
    pushButtons[BUTTON_IDX_RUN].locked = button->state;
    digitalWrite (LED_RUN, LOW);

    // enable/disable sequence change
    pushButtons[BUTTON_IDX_SELECT].locked = button->state;
    
    // enable/disable manual valve control
    lockValveControl(!button->state);
    
    // set system mode
    systemMode = button->state ? AUTOMATIC_MODE:MANUAL_MODE;

    writeLog(LOG_INFO, "Set mode to %s", systemMode == MANUAL_MODE ? "manual" : "automatic");
}

/* ----------------------------------------------------------------------------------- *
 * Set LED of push button
 * ----------------------------------------------------------------------------------- */
void setLed( pushbutton_t *button ) {
    digitalWrite ( button->ledPin, button->state ? HIGH : LOW);
}

/* ----------------------------------------------------------------------------------- *
 * process active sequence
 * ----------------------------------------------------------------------------------- */
void processSequence() {
    static int lastStep =0;
    int offset = (int)time(NULL)-sequenceStartTime;
    if ( !sequence[activeSequence][0].done ) {
         lastStep = 0;
    }
    int step = lastStep;

    while ( sequence[activeSequence][step].offset >= 0 ) {
        sequence_t *seqStep = &sequence[activeSequence][step];
        
        if (seqStep->offset <= offset && !seqStep->done) {
            seqStep->done = true;                    // mark step as done
            seqStep->valve->state = seqStep->state;  // Valve ON or OFF ?

            //writeLog(LOG_INFO, "S%02d(%02d) t+%04d: turn valve %c %s", activeSequence, step, offset,
            //         seqStep->valve->name, seqStep->state? "ON":"OFF");

            switchValve(seqStep->valve);             // switch Valve
            lastStep = step;                         // remember where we left off
            break;                                   // we're done here for now
        } else if (seqStep->offset > offset) {       // skip the future
            break;
        }
        step++;
    }
    
    // end of sequence reached?
    if (sequence[activeSequence][step].offset < 0) {
        pushButtons[5].state=false;                 // simulate sequence button press
        startSequence( &pushButtons[5] );
    }
}

/* ----------------------------------------------------------------------------------- *
 * Setup IO ports
 * ----------------------------------------------------------------------------------- */
void setupIO ( void ) {
    // initialize wiring PI and attached IO extender
    wiringPiSetup () ;
    mcp23017Setup (PINBASE_0, ADDR_IOEXT_0);

    // setup pin modes for buttons
    int btnIndex = 0;    
    while (pushButtons[btnIndex].btnPin >= 0 ) {
        pinMode(pushButtons[btnIndex].btnPin, INPUT);
        pullUpDnControl (pushButtons[btnIndex].btnPin, PUD_UP) ;
        pinMode(pushButtons[btnIndex].ledPin, OUTPUT);
        digitalWrite(pushButtons[btnIndex].ledPin,
                     pushButtons[btnIndex].state ? HIGH : LOW );
        btnIndex++;
    }

    pinMode(LED_S0, OUTPUT);
    pinMode(LED_S1, OUTPUT);

    digitalWrite (LED_S0, HIGH);
    digitalWrite (LED_S1, LOW);
}

/* ----------------------------------------------------------------------------------- *
 * Things that need to be done regulary
 * ----------------------------------------------------------------------------------- */
void houseKeeping(void) {
    writeLog(LOG_INFO, "Do housekeeping");

    // publish Status of all buttons
    int btnIndex = 0;
    while ( pushButtons[btnIndex].btnPin >= 0 ) {
        publishStatus(&pushButtons[btnIndex]);
        btnIndex++;
    }
}

/* ----------------------------------------------------------------------------------- *
 * Main
 * ----------------------------------------------------------------------------------- */
int main( int argc, char *argv[] ) {
    bool dumpConfig = false;
    
    // Process command line options
    for (int i=0; i<argc; i++) {
        if (!strcmp(argv[i], "-d")) {          // '-d' turns debug mode on
            debug++;
        }
        if (!strcmp(argv[i], "-c")) {          // '-c' specify configuration file name
            configFile = strdup(argv[++i]);
        }
        if (!strcmp(argv[i], "-f")) {          // '-f' forces forground mode
            foreground=true;
        }
        if (!strcmp(argv[i], "-n")) {          // '-n' dont start read config and dump result
            dumpConfig=true;
        }
    }
    
    // initialize logging channel
    initLog(!foreground);
    setLogLevel(LOG_NOTICE+debug);
    
    // read configuration from file
    readConfig();

    if (!foreground) {                           // run in background
        daemonize(PID_FILE);
    } else {
        writeLog(LOG_NOTICE, "Running in foreground");
    }
    
    if ( dumpConfig ) {
        // dump configuration
        dumpSequence( 0 );
        dumpSequence( 1 );
        exit(1);
    }

    // initialize MQTT connection to broker
    if (mqttBroker.address) {
        mqttIncoming_t subscriptions[] = {
            {"/YardControl/Command/Valve_A", &pressButtonCB, (void*)&pushButtons[0]},
            {"/YardControl/Command/Valve_B", &pressButtonCB, (void*)&pushButtons[1]},
            {"/YardControl/Command/Valve_C", &pressButtonCB, (void*)&pushButtons[2]},
            {"/YardControl/Command/Valve_D", &pressButtonCB, (void*)&pushButtons[3]},
            {"/YardControl/Command/Valve_S", &pressButtonCB, (void*)&pushButtons[4]},
            {"/YardControl/Command/Valve_R", &pressButtonCB, (void*)&pushButtons[5]},
            {"/YardControl/Command/Valve_P", &pressButtonCB, (void*)&pushButtons[6]},
            {NULL, NULL, NULL},
        };
        
        if (mqttInit(mqttBroker.address, mqttBroker.port, mqttBroker.keepalive, subscriptions)) {
            writeLog(LOG_INFO, "Connected MQTT boker at %s:%d", mqttBroker.address, mqttBroker.port);
        }
    }
    
    // Initialize IO ports
    setupIO();
    
    if (systemMode == AUTOMATIC_MODE) {
        writeLog(LOG_INFO, "Starting up in automatic mode");
        pushButtons[BUTTON_IDX_TIMER].state = true;
        automaticMode( &pushButtons[BUTTON_IDX_TIMER] );
    }
    
    // publish Status of all buttons
    int btnIndex = 0;
    while ( pushButtons[btnIndex].btnPin >= 0 ) {
        publishStatus(&pushButtons[btnIndex]);
        btnIndex++;
    }
    
    // Main loop
    time_t lastTime = 0;
    int    lastHouseKeeping = 0;
    for ( ;; ) {                                 // never stop working
        time_t now = time(NULL);

        if ( lastTime != now ) {                 // only work do once a second
            lastTime = now;
            struct tm *timestamp = localtime(&now);
            if ((timestamp->tm_min % 5 == 0) && timestamp->tm_hour != lastHouseKeeping) {
                // do housekeeping every 5 minutes
                lastHouseKeeping = timestamp->tm_hour;
                houseKeeping();
            }
    
            if (systemMode == AUTOMATIC_MODE) {
                int timeIdx=0;
                while(startTime[activeSequence][timeIdx].tm_hour >= 0 ) {
                    if ( timestamp->tm_hour == startTime[activeSequence][timeIdx].tm_hour
                        && timestamp->tm_min == startTime[activeSequence][timeIdx].tm_min
                        && !sequenceInProgress ) {
                        writeLog( LOG_INFO, "Autostart sequence %02d", activeSequence );
                        pushButtons[5].state=true;              // simulate sequence button press
                        startSequence( &pushButtons[5] );
                    }
                    timeIdx++;
                }
            }
            if (sequenceInProgress) {         // forward sequence
                processSequence();
            }
        }
        
        pollButtons(pushButtons);             // poll bush buttons
        delay(50);                            // have a rest
    }
    return 0;
}
