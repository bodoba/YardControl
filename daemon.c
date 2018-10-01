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
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <syslog.h>

#include "daemon.h"

/* ----------------------------------------------------------------------------------- *
 * Local prototype
 * ----------------------------------------------------------------------------------- */
static void sigendCB( int sigval );
static void shutdown_daemon(void);
static void writePid(const char* pidFile);

/* ----------------------------------------------------------------------------------- *
 * Some globals we can't do without... ;)
 * ----------------------------------------------------------------------------------- */
int    pidFilehandle = 0;                      // PID file kept open for daemona

/* ----------------------------------------------------------------------------------- *
 * Daemonize
 * ----------------------------------------------------------------------------------- */
void daemonize(const char *pidFile) {
    
    // If we got a good PID, then we can exit the parent process
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    } else  if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    umask(0);                                // Change the file mode mask
    pid_t sid = setsid();                    // Create a new SID for the child process
    if (sid < 0) {
        syslog(LOG_ERR, "Could not get SID");
        exit(EXIT_FAILURE);
    }
    
    if ((chdir("/tmp")) < 0) {               // Change the current working directory
        syslog(LOG_ERR, "Could not chage working dir to /tmp");
        exit(EXIT_FAILURE);
    }
    
    // use /dev/null for the standard file descriptors
    int fd = open("/dev/null", O_RDWR);      // Open /dev/null as STDIN
    dup(fd);                                 // STDOUT to /dev/null
    dup(fd);                                 // STDERR to /dev/null
    
    writePid(piFile);                              // write PID to file
    
    signal(SIGHUP,  sigendCB);               // catch hangup signal
    signal(SIGTERM, sigendCB);               // catch term signal
    signal(SIGINT,  sigendCB);               // catch interrupt signal
}

/* ----------------------------------------------------------------------------------- *
 * Write PID file
 * ----------------------------------------------------------------------------------- */
void writePid(const char *pidFile) {
    pidFilehandle = open(pidFile, O_RDWR|O_CREAT, 0600);
    
    if (pidFilehandle != -1 ) {                           // Open failed
        if (lockf(pidFilehandle,F_TLOCK,0) != -1) {       // Try to lock the pid file
            char buffer[10];
            sprintf(buffer,"%d\n",getpid());              // Get and format PID
            write(pidFilehandle, buffer, strlen(buffer)); // write pid to lockfile
        } else {
            syslog(LOG_CRIT, "Could not lock PID lock file %s, exiting", PID_FILE);
            exit(EXIT_FAILURE);
        }
    } else {
        syslog(LOG_CRIT, "Could not open PID lock file %s, exiting", PID_FILE);
        exit(EXIT_FAILURE);
    }
}

/* ----------------------------------------------------------------------------------- *
 * there are many ways to die
 * ----------------------------------------------------------------------------------- */
void sigendCB(int sigval)
{
    switch(sigval)
    {
        case SIGHUP:
            syslog(LOG_WARNING, "Received SIGHUP signal.");
            break;
        case SIGINT:
        case SIGTERM:
            syslog(LOG_INFO, "Daemon exiting");
            shutdown_daemon();
            exit(EXIT_SUCCESS);
            break;
        default:
            syslog(LOG_WARNING, "Unhandled signal %s", strsignal(sigval));
            break;
    }
}
/* ----------------------------------------------------------------------------------- *
 * shutdwown deamon
 * ----------------------------------------------------------------------------------- */
void shutdown_daemon(void) {
    syslog(LOG_INFO, "Yard Control shutting down");
    close(pidFilehandle);
    unlink(PID_FILE);
}

