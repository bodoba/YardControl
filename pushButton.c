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
#include <wiringPi.h>
#include <stdio.h>

#include "pushButton.h"

/* ----------------------------------------------------------------------------------- *
 * poll Buttons
 * ----------------------------------------------------------------------------------- */
void pollButtons(pushbutton_t pushButtons[]) {
    int btnIndex = 0;
    while ( pushButtons[btnIndex].btnPin >= 0 ) {
        readButton(&pushButtons[btnIndex], pushButtons);
        btnIndex++;
    }
}

/* ----------------------------------------------------------------------------------- *
 * Process push button
 * ----------------------------------------------------------------------------------- */
bool readButton( pushbutton_t *button, pushbutton_t *buttonList) {
    // respect locked state
    if ( !button->locked) {
        // read the button pin
        int newReading = digitalRead(button->btnPin);
        
        // if there has been a change
        if ( newReading != button->lastReading ) {
            button->lastReading = newReading;
            // button pressed toggles state
            if ( newReading == 0 ) {
                button->state = button->state ? false : true;
                
                // if a radio group has been defined clear state of all buttons in this group
                if ( button->state && button->radioGroup > 0 ) {
                    int btnIndex = 0;
                    // clear state of active members in radio group
                    while ( buttonList[btnIndex].btnPin >= 0 ) {
                        if ( buttonList[btnIndex].radioGroup == button->radioGroup   // same radio group
                            && buttonList[btnIndex].btnPin != button->btnPin         // not myself
                            && buttonList[btnIndex].state ) {                        // active
                            // clear state
                            buttonList[btnIndex].state = false;
                            // trigger callback function
                            if ( buttonList[btnIndex].callback != NULL ) {
                                (*buttonList[btnIndex].callback)(&buttonList[btnIndex]);
                            }
                        }
                        btnIndex++;
                    }
                }
                
                // trigger callback function
                if ( button->callback != NULL ) {
                    (*button->callback)(button);
                }
            }
        }
    }
    return button->state;
}
