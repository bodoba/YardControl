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
#include "readConfig.h"

/* ----------------------------------------------------------------------------------- *
 * Some globals we can't do without
 * ----------------------------------------------------------------------------------- */
char *configFile    = CONFIG_FILE;            /* configuration file                    */
sequence_t  sequence[2][MAX_STEP];            /* two program sequences of max 40 steps */
starttime_t startTime[2];                     /* start times for each sequence         */

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
    int sequenceIdx = -1, step = -1, offset=-1, lineNo=1;
    
    // start with two empty sequences
    sequence[0][0].offset = -1;
    startTime[0].tm_hour  = -1;
    
    sequence[1][0].offset = -1;
    startTime[1].tm_hour  = -1;
    
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
                
                while ( *cursor == ' ' || *cursor == '\t' ) cursor++;
                
                if ( *cursor != '#') {                          /* skip '#' comments   */
                    char *token = cursor;
                    char *value = nextValue(&cursor);
                    
                    if (!strcmp(token, "SEQUENCE")) {
                        sequenceIdx++;
                        step   = 0;
                        offset = 0;
                    } else if (!strcmp(token, "TIME")) {
                        // expected format is hh:mm
                        char *hh, *mm;
                        int hour, min;
                        hh=value;
                        mm=value+3;
                        *(value+2)='\0';
                        *(value+5)='\0';
                        hour=atoi(hh);
                        min =atoi(mm);
                        if( hour>=0 && hour<24 && min>=0 && min<60 ) {
                            startTime[sequenceIdx].tm_min  = min;
                            startTime[sequenceIdx].tm_hour = hour;
                        } else {
                            printf ( "[%s:%04d] ERROR: TIME expected as hh:mm\n", configFile, lineNo );
                        }
                    } else if (!strcmp(token, "PAUSE")) {
                        int time  = atoi(value);
                        if (time > 0 ) {
                            offset+=(time*TIME_SCALE-1);
                        } else {
                            printf ( "[%s:%04d] ERROR: Wromg time in DELAY: %d\n", configFile, lineNo, time );
                        }
                    } else if (!strcmp(token, "VALVE")) {
                        char *valve = cursor;
                        int time  = atoi(nextValue(&cursor));
                        int buttonIdx;
                        if (time > 0 ) {
                            buttonIdx = toupper(*valve) - 'A';  // A->0, B->1, C->2, D->3
                            if ( buttonIdx >= 0 && buttonIdx <= 3 && step < (MAX_STEP-2)) {   // Add step to sequence
                                
                                // turn valve on
                                sequence[sequenceIdx][step].offset = offset;
                                sequence[sequenceIdx][step].valve  = &pushButtons[buttonIdx];
                                sequence[sequenceIdx][step].state  = true;
                                step++;
                                offset += (time*TIME_SCALE);
                                
                                // turn valve off
                                sequence[sequenceIdx][step].offset = offset;
                                sequence[sequenceIdx][step].valve  = &pushButtons[buttonIdx];
                                sequence[sequenceIdx][step].state  = false;
                                step++;
                                offset++;
                                
                                // Add End marker
                                sequence[sequenceIdx][step].offset = -1;
                            } else {
                                if ( step >= (MAX_STEP-2) ) {
                                    printf ( "[%s:%04d] ERROR: Sequence too long, ignoring line \n", configFile, lineNo );
                                } else {
                                    printf ( "[%s:%04d] ERROR: Unknown VALVE: %s\n", configFile, lineNo, valve );
                                }
                            }
                        } else {
                            printf ( "[%s:%04d] ERROR: Wromg time in VALVE: %d\n", configFile, lineNo, time );
                        }
                    } else {
                        printf ( "[%s:%04d] WARNING: Skipping unknown command: %s\n", configFile, lineNo, token );
                    }
                }
            }
            free(line);
            n=0;
            length = getline(&line, &n, fp);
            lineNo++;
        }
        fclose(fp);
    }
    
    dumpSequence( 0 );
    dumpSequence( 1 );
    
    return true;
}

/* ----------------------------------------------------------------------------------- *
 * Dump sequence definition
 * ----------------------------------------------------------------------------------- */
void dumpSequence( int sequenceIdx ) {
    int step = 0;
    int lastON = 0, lastOFF = 0;
    sequence_t *seq = sequence[sequenceIdx];
    
    if ( seq[0].offset >=0 ) {
        printf("# ----------------------------------------------------------------------------------- #\n");
        printf("# Sequence %d                                                                          #\n", sequenceIdx );
        printf("# ----------------------------------------------------------------------------------- #\n");
        printf("SEQUENCE START\n");
        if ( startTime[sequenceIdx].tm_hour >= 0 ) {
            printf( "  START %02d:%02d\n", startTime[sequenceIdx].tm_hour, startTime[sequenceIdx].tm_min);
        } else {
            printf("#                     no START time defined, sequence will not run in automatic mode\n");
        }
        while ( seq[step].offset >= 0 ) {
            if ( seq[step].state ) {
                if ( seq[step].offset > (lastOFF+1) ) {
                    printf("  PAUSE %d\n", (seq[step].offset-lastOFF)/TIME_SCALE );
                }
                lastON = seq[step].offset;
            } else {
                printf("  VALVE %c %d\n", seq[step].valve->name, (seq[step].offset-lastON)/TIME_SCALE );
                lastOFF = seq[step].offset;
            }
            printf("#                     %03d t+%04d %c %s\n",
                   step,
                   seq[step].offset,
                   seq[step].valve->name,
                   seq[step].state? "ON":"OFF");
            step++;
        }
    } else {
        printf("# ----------------------------------------------------------------------------------- #\n");
        printf("# Sequence %d not defined                                                             #\n", sequenceIdx );
        printf("# ----------------------------------------------------------------------------------- #\n");
    }
}
