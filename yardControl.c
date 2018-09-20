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
bool readButton(pushbutton_t *button);
void setup(void);
int  main( int rgc, char *argv[] );

/* ----------------------------------------------------------------------------------- *
 * Definition of the pushbuttons
 * ----------------------------------------------------------------------------------- */
pushbutton_t pushButtons[] = {
    // Button Pin, Led Pin, state, last reading, locked, radio group
    
    // Manual valves control, only one shall be active
    {BUTTON_A,    VALVE_A,  false, -1, false, 1},
    {BUTTON_B,    VALVE_B,  false, -1, false, 1},
    {BUTTON_C,    VALVE_C,  false, -1, false, 1},
    {BUTTON_D,    VALVE_D,  false, -1, false, 1},

    {BUTTON_P1,   LED_P1,   false, -1, false, 1},
    {BUTTON_P2,   LED_P2,   false, -1, false, 1},

    {BUTTON_AUTO, LED_AUTO, false, -1, false, 1},
    
    // end marker
    {-1, -1, false, -1, false, -1},
};

/* ----------------------------------------------------------------------------------- *
 * Process push button
 * ----------------------------------------------------------------------------------- */
bool readButton( pushbutton_t *button ) {
    // respect locked state
    if ( !button->locked ) {
        // read the button pin
        int newReading = digitalRead(button->btnPin);
        
        // if there has been a change
        if ( newReading != button->lastReading ) {
            button->lastReading = newReading;
            // button pressed toggles state
            if ( newReading == 0 ) {
                button->state = button->state ? false : true;
            }
            
            // if a radio group has been defined clear state of all buttons in this group
            if ( button->state && button->radioGroup > 0 ) {
                int btnIndex = 0;
                while ( pushButtons[btnIndex].btnPin >= 0 ) {
                    if ( pushButtons[btnIndex].radioGroup == button->radioGroup ) {
                        pushButtons[btnIndex].state = false;
                        digitalWrite( pushButtons[btnIndex].ledPin, HIGH);
                    }
                    btnIndex++;
                }
                // set myself to true again
                button->state=true;
            }
            
            // set indicator led
            digitalWrite ( button->ledPin, button->state ? LOW : HIGH);
        }
    }
    return button->state;
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
//        digitalWrite(pushButtons[btnIndex].ledPin, pushButtons[btnIndex].state ? LOW : HIGH);
        digitalWrite(pushButtons[btnIndex].ledPin, LOW);

        btnIndex++;
    }
}

/* ----------------------------------------------------------------------------------- *
 * M  A  I  N
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
            btnIndex++;
        }

        delay(50);
    }
    return 0;
}
