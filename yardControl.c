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

#define PINBASE 64

#define LED_A PINBASE+0
#define LED_B PINBASE+1
#define LED_C PINBASE+2
#define LED_D PINBASE+3

#define BTN_A PINBASE+4
#define BTN_B PINBASE+5
#define BTN_C PINBASE+6
#define BTN_D PINBASE+7

int main( int rgc, char *argv ) {
    wiringPiSetup () ;
    pcf8574Setup (PINBASE, 0x38);
    
    pinMode (LED_A, OUTPUT);
    pinMode (LED_B, OUTPUT);
    pinMode (LED_C, OUTPUT);
    pinMode (LED_D, OUTPUT);
    
    pinMode (BTN_A, INPUT);
    pinMode (BTN_B, INPUT);
    pinMode (BTN_C, INPUT);
    pinMode (BTN_D, INPUT);
    
    digitalWrite (LED_A,  LOW) ;
    digitalWrite (LED_B,  LOW) ;
    digitalWrite (LED_C,  LOW) ;
    digitalWrite (LED_D,  LOW) ;
    
    int stateA=0;
    int stateB=0;
    int stateC=0;
    int stateD=0;
    
    for ( ;; ) {
        if ( digitalRead(BTN_A) == 0 ) { stateA = stateA ? 0 : 1; }
        if ( digitalRead(BTN_B) == 0 ) { stateB = stateB ? 0 : 1; }
        if ( digitalRead(BTN_C) == 0 ) { stateC = stateC ? 0 : 1; }
        if ( digitalRead(BTN_D) == 0 ) { stateD = stateD ? 0 : 1; }
        
        digitalWrite ( LED_A, stateA );
        digitalWrite ( LED_B, stateB );
        digitalWrite ( LED_C, stateC );
        digitalWrite ( LED_D, stateD );
        
        delay(50);
        
    }
    return 0;
}
