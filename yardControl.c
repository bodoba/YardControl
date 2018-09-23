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
#include <stdio.h>
#include <pcf8574.h>

/* ----------------------------------------------------------------------------------- *
 * Prototypes
 * ----------------------------------------------------------------------------------- */
void setup(void);
int  main( int rgc, char *argv[] );

// Bush button actions
void setLed( pushbutton_t *button );
void switchValve( pushbutton_t *button );
void runSequence( pushbutton_t *button );
void automaticMode( pushbutton_t *button );

/* ----------------------------------------------------------------------------------- *
 * Definition of the pushbuttons
 * ----------------------------------------------------------------------------------- */
pushbutton_t pushButtons[] = {
    // Button Pin, Led Pin, state, last reading, locked, radio group
    
    // Manual valves control, only one shall be active
    {BUTTON_A,    VALVE_A,  false, -1, false, RG_VALVES,    &switchValve},
    {BUTTON_B,    VALVE_B,  false, -1, false, RG_VALVES,    &switchValve},
    {BUTTON_C,    VALVE_C,  false, -1, false, RG_VALVES,    &switchValve},
    {BUTTON_D,    VALVE_D,  false, -1, false, RG_VALVES,    &switchValve},

    {BUTTON_P1,   LED_P1,   false, -1, false, RG_SEQUENCES, &runSequence},
    {BUTTON_P2,   LED_P2,   false, -1, false, RG_SEQUENCES, &runSequence},

    {BUTTON_AUTO, LED_AUTO, false, -1, false, RG_NONE,      &automaticMode},
    
    // end marker
    {-1, -1, false, -1, false, -1},
};

/* ----------------------------------------------------------------------------------- *
 * Modes
 * ----------------------------------------------------------------------------------- */
enum Modes { MANUAL_MODE, AUTOMATIC_MODE } systemMode;

/* ----------------------------------------------------------------------------------- *
 * Switch Valve
 * ----------------------------------------------------------------------------------- */
void switchValve( pushbutton_t *button ) {
    setLed( button );
}

/* ----------------------------------------------------------------------------------- *
 * runProgram Sequence
 * ----------------------------------------------------------------------------------- */
void runSequence( pushbutton_t *button ) {
    setLed( button );
}

/* ----------------------------------------------------------------------------------- *
 * Run in automatic mode
 * ----------------------------------------------------------------------------------- */
void automaticMode( pushbutton_t *button ) {
    setLed( button );
    if ( button->state ) {
        // disable manual valve control
        for (int buttonIdx=0; buttonIdx<=3; buttonIdx++ ) {
            pushbutton_t *btnValve = &pushButtons[buttonIdx];
            btnValve->locked = true;
            btnValve->state  = false;
            switchValve( btnValve );
        }
        
    } else {
        // enable manual valve control
        for (int buttonIdx=0; buttonIdx<=3; buttonIdx++ ) {
            pushButtons[buttonIdx].locked = false;
        }
    }
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
void setup ( void ) {
    // initialize wiring PI and attached IO extender
    wiringPiSetup () ;
    pcf8574Setup (PINBASE_0, ADDR_IOEXT_0);
    pcf8574Setup (PINBASE_1, ADDR_IOEXT_1);

    // setup pin modes for buttons
    int btnIndex = 0;    
    while ( pushButtons[btnIndex].btnPin >= 0 ) {
        pinMode(pushButtons[btnIndex].btnPin, INPUT);
        pinMode(pushButtons[btnIndex].ledPin, OUTPUT);
        digitalWrite(pushButtons[btnIndex].ledPin, pushButtons[btnIndex].state ? LOW : HIGH);
        btnIndex++;
    }
}

/* ----------------------------------------------------------------------------------- *
 * Main
 * ----------------------------------------------------------------------------------- */
int main( int rgc, char *argv[] ) {

    // initialize system
    setup();
    
    // main loop
    for ( ;; ) {
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
