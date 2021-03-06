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

#include "logging.h"
#include <syslog.h>
#include <string.h>

/* ----------------------------------------------------------------------------------- *
 * local data
 * ----------------------------------------------------------------------------------- */
static int  logLevel  = LOG_ERR;
static bool useSyslog = true;
static const char * logLevelText[] = {  "emergency",
                                        "alert",
                                        "citical",
                                        "error",
                                        "warning",
                                        "notice",
                                        "info",
                                        "debug"
};

/* ----------------------------------------------------------------------------------- *
 * init logging
 * ----------------------------------------------------------------------------------- */
void initLog( bool syslog ) {
    useSyslog = syslog;
    
    if ( useSyslog) {
        openlog(NULL, LOG_PID, LOG_USER);            // use syslog to create a trace
    }
}

/* ----------------------------------------------------------------------------------- *
 * set Loglevel
 * ----------------------------------------------------------------------------------- */
int setLogLevel( int level ) {
    logLevel = level;
    if (logLevel > LOG_DEBUG ) {
        logLevel = LOG_DEBUG;
    }
    writeLog(LOG_NOTICE, "Set log level to %s", logLevelText[level]);
    return logLevel;
}
    
/* ----------------------------------------------------------------------------------- *
 * return Loglevel
 * ----------------------------------------------------------------------------------- */
int getLogLevel( void ) {
    return logLevel;
}

/* ----------------------------------------------------------------------------------- *
 * write log entry
 * ----------------------------------------------------------------------------------- */
void writeLog( int level, const char* format, ...) {
    va_list valist;
    if( level <= logLevel ) {
        time_t now = time(NULL);
        struct tm *timestamp = localtime(&now);
        char fmt[512];
        va_start(valist, format);
    
        if ( useSyslog ) {
            sprintf(fmt, "<%s> %s\n", logLevelText[level], format);
            vsyslog( level, fmt, valist );
        } else {
            sprintf(fmt, "%04d-%02d-%02d %02d:%02d:%02d <%s> %s\n",
                    timestamp->tm_year+1900, timestamp->tm_mon+1, timestamp->tm_mday,
                    timestamp->tm_hour, timestamp->tm_min, timestamp->tm_sec,
                    logLevelText[level] , format);
            vprintf( fmt, valist );
        }
    }
}
