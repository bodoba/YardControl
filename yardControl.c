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

/* ----------------------------------------------------------------------------------- *
 * Prototypes
 * ----------------------------------------------------------------------------------- */
bool readButton( pushbutton_t *button );

/* ----------------------------------------------------------------------------------- *
 * Definition of the pushbuttons
 * ----------------------------------------------------------------------------------- */
pushbutton_t pushButtons[] = {

    // Manual valves control
    {BUTTON_A, VALVE_A, false, false, (time_t)0},
    {BUTTON_B, VALVE_B, false, false, (time_t)0},
    {BUTTON_C, VALVE_C, false, false, (time_t)0},
    {BUTTON_D, VALVE_D, false, false, (time_t)0},

    // end marker
    {-1, -1, false, false, (time_t)0},
};

/* ----------------------------------------------------------------------------------- *
 * Read push button
 * ----------------------------------------------------------------------------------- */
bool readButton( pushbutton_t *button ) {
    
    if ( digitalRead(button->btnPin) == 0 ) {
        button->state = button->state ? 0 : 1;
    }
    digitalWrite ( button->ledPin, button->state );
    
    return button->state;
}

/* ----------------------------------------------------------------------------------- *
 * Initial setup
 * ----------------------------------------------------------------------------------- */
void setup ( void ) {
    // initialize wiring PI and attached IO extender
    wiringPiSetup () ;
    pcf8574Setup (PINBASE_0, ADDR_IOEXT_0);

    // setup pin modes for buttons
    int btnIndex = 0;    
    while ( pushButtons[btnIndex].btnPin >= 0 ) {
        pinMode(pushButtons[btnIndex].btnPin, INPUT);
        pinMode(pushButtons[btnIndex].ledPin, OUTPUT);
        digitWrite(pushButtons[btnIndex].ledPin, LOW);
        btnIndex++;
    }
}

/* ----------------------------------------------------------------------------------- *
 * Initial setup
 * ----------------------------------------------------------------------------------- */
int main( int rgc, char *argv[] ) {

    // initialize system
    setup();
    
    // main loop
    for ( ;; ) {

        // update button values
        int btnIndex = 0;
        while ( pushButtons[btnIndex].btnPin >= 0 ) {
            readButton(&pushButtons[btnIndex]);
        }

        
        delay(50);
    }
    return 0;
}
