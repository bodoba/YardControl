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

bushbutton_t pushButtons[] = {
    {BUTTON_A, VALVE_A, false, false, (time_t)0},
    {BUTTON_B, VALVE_B, false, false, (time_t)0},
    {BUTTON_C, VALVE_C, false, false, (time_t)0},
    {BUTTON_D, VALVE_D, false, false, (time_t)0},
    {0, 0, false, false, (time_t)0},
};

int main( int rgc, char *argv[] ) {
    wiringPiSetup () ;
    pcf8574Setup (PINBASE_0, ADDR_IOEXT_0);
    
    pinMode (VALVE_A, OUTPUT);
    pinMode (VALVE_B, OUTPUT);
    pinMode (VALVE_C, OUTPUT);
    pinMode (VALVE_D, OUTPUT);
    
    pinMode (BUTTON_A, INPUT);
    pinMode (BUTTON_B, INPUT);
    pinMode (BUTTON_C, INPUT);
    pinMode (BUTTON_D, INPUT);
    
    digitalWrite (VALVE_A,  LOW) ;
    digitalWrite (VALVE_B,  LOW) ;
    digitalWrite (VALVE_C,  LOW) ;
    digitalWrite (VALVE_D,  LOW) ;
    
    int stateA=0;
    int stateB=0;
    int stateC=0;
    int stateD=0;
    
    for ( ;; ) {
        if ( digitalRead(BUTTON_A) == 0 ) { stateA = stateA ? 0 : 1; }
        if ( digitalRead(BUTTON_B) == 0 ) { stateB = stateB ? 0 : 1; }
        if ( digitalRead(BUTTON_C) == 0 ) { stateC = stateC ? 0 : 1; }
        if ( digitalRead(BUTTON_D) == 0 ) { stateD = stateD ? 0 : 1; }
        
        digitalWrite ( VALVE_A, stateA );
        digitalWrite ( VALVE_B, stateB );
        digitalWrite ( VALVE_C, stateC );
        digitalWrite ( VALVE_D, stateD );
        
        delay(50);
    }
    return 0;
}
