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
#include <stdbool.h>

#ifndef pushButton_h
#define pushButton_h

/* ----------------------------------------------------------------------------------- *
 * Definition of a push button
 * ----------------------------------------------------------------------------------- */
typedef struct pushbutton_t {
    char    name;                  // button name
    int     btnPin;                // Input pin
    int     ledPin;                // LED indicating button state
    bool    state;                 // button state
    int     lastReading;           // last button reading
    bool    locked;                // when locked, button value can't be changed manually
    int     radioGroup;            // >  0 defines radio button group
    void    (*callback) (struct pushbutton_t *button); // callback for button state change
} pushbutton_t;

/* ----------------------------------------------------------------------------------- *
 * Prototypes
 * ----------------------------------------------------------------------------------- */
bool readButton(pushbutton_t *button, pushbutton_t *buttonList);  // read single button
void pollButtons(pushbutton_t pushButtons[]);                     // poll all buttons

#endif /* pushButton_h */
