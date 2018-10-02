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
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "yardControl.h"
#include "readConfig.h"
#include "logging.h"

/* ----------------------------------------------------------------------------------- *
 * Some globals we can't do without
 * ----------------------------------------------------------------------------------- */
char *configFile    = CONFIG_FILE;            /* configuration file                    */
sequence_t  sequence[2][MAX_STEP];            /* two program sequences of max 40 steps */
starttime_t startTime[2][MAX_STARTTIMES+1];   /* 10 start times for each sequence      */

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
    int sequenceIdx, timeIdx, step = -1, offset=-1, lineNo=1;
    bool retval = false;
    
    // start with two empty sequences
    for ( int sequenceIdx=0; sequenceIdx <=1; sequenceIdx++ ) {
        sequence[sequenceIdx][0].offset = -1;
        for (timeIdx=0; timeIdx<=MAX_STARTTIMES; timeIdx++) {
            startTime[sequenceIdx][timeIdx].tm_hour  = -1;
        }
    }
    // inititalize counter;
    sequenceIdx = -1;
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
                        sequenceIdx = atoi (value);
                        if ( *value == '0' || *value == '1' ) {
                            timeIdx = 0;
                            step    = 0;
                            offset  = 0;
                        } else {
                            sequenceIdx = -1;
                            writeLog( LOG_ERR, "[%s:%04d] ERROR: Wrong sequence number '%s' must be 0 or 1\n",
                                     configFile, lineNo, value );
                        }
                    } else if (!strcmp(token, "TIME")) {
                        // expected format is "TIME hh:mm s
                        char *hh, *mm, *seq;
                        int hour, min, idx;
                        hh=value;
                        mm=value+3;
                        seq=value+6;
                        *(value+2)='\0';
                        *(value+5)='\0';
                        *(value+7)='\0';
                        hour = atoi(hh);
                        min  = atoi(mm);
                        idx  = atoi(seq);
                        if( hour>=0 && hour<24 && min>=0 && min<60 && (*seq=='0'||*seq=='1')) {
                            if ( timeIdx < MAX_STARTTIMES ) {
                                startTime[idx][timeIdx].tm_min  = min;
                                startTime[idx][timeIdx].tm_hour = hour;
                                timeIdx++;
                            } else {
                                writeLog( LOG_ERR, "[%s:%04d] ERROR: Maximum TIME statements of %02d exceeded\n",
                                         configFile, lineNo, MAX_STARTTIMES );
                            }
                        } else {
                            writeLog( LOG_ERR, "[%s:%04d] ERROR: TIME expected as hh:mm s\n", configFile, lineNo );
                        }
                    } else if (!strcmp(token, "PAUSE")) {
                        int time  = atoi(value);
                        if (time > 0 ) {
                            offset+=(time*TIME_SCALE-1);
                        } else {
                            writeLog( LOG_ERR, "[%s:%04d] ERROR: Wromg time in DELAY: %d\n", configFile, lineNo, time );
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
                                    writeLog( LOG_ERR, "[%s:%04d] ERROR: Sequence too long, ignoring line \n", configFile, lineNo );
                                } else {
                                    writeLog( LOG_ERR, "[%s:%04d] ERROR: Unknown VALVE: %s\n", configFile, lineNo, valve );
                                }
                            }
                        } else {
                            writeLog( LOG_ERR, "[%s:%04d] ERROR: Wromg time in VALVE: %d\n", configFile, lineNo, time );
                        }
                    } else {
                        writeLog( LOG_ERR, "[%s:%04d] WARNING: Skipping unknown command: %s\n", configFile, lineNo, token );
                    }
                }
            }
            free(line);
            n=0;
            length = getline(&line, &n, fp);
            lineNo++;
            retval = true;
        }
        fclose(fp);
    }
    return retval;
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
        printf("SEQUENCE %d\n", sequenceIdx);
        printf("# ----------------------------------------------------------------------------------- #\n");
        
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

    int timeIdx=0;
    while(startTime[sequenceIdx][timeIdx].tm_hour >= 0 ) {
        if ( startTime[sequenceIdx][timeIdx].tm_hour >= 0 ) {
            printf( "  TIME %02d:%02d %d\n", startTime[sequenceIdx][timeIdx].tm_hour,
                   startTime[sequenceIdx][timeIdx].tm_min,
                   sequenceIdx);
        }
        timeIdx++;
    }

}
