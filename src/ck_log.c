//
//  ck_log.c
//  chokus
//
//  Created by Tom Seago on 4/6/13.
//
//

#define NO_EXTERN_LOGGING_DEFS 1

#include "chokus.h"

#include <stdio.h>
#include <sys/time.h>

int gLogLevel;

ckBool syslogEnabled = FALSE;
ckLogLevel syslogLevel = CKLL_TRACE;

ckBool consoleEnabled = TRUE;
ckLogLevel consoleLevel = CKLL_TRACE;

char* gLevels[] = {
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG",
    "TRACE"
};


ckTime ck_Now()
{
    struct timeval st;
    
    gettimeofday(&st, NULL);
    
    return (st.tv_sec * CKTIME_1SEC) + st.tv_usec;
}



void ckLog_Log(ckLogLevel level, const char* format, ...)
{
    va_list list;
    
    // Sanity checking
    if (!format) return;
    if ((level < CKLL_ERR) || (level > CKLL_TRACE))
    {
        level = CKLL_TRACE;
    }

    va_start(list, format);

    // Write to stdout
    if (consoleEnabled && level <= consoleLevel)
    {
        // Format out the time
        // TODO - that^
        
        printf("%s", gLevels[level]);
        printf(": ");
        
        vprintf(format, list);
        printf("\n");
    }
    
    // TODO: Write to syslog
    
    va_end(list);
}

void ckLog_EnableSyslog(ckBool enabled)
{
    syslogEnabled = enabled;
}

void ckLog_SetSyslogLevel(ckLogLevel level)
{
    syslogLevel = level;
}

void ckLog_EnableConsole(ckBool enabled)
{
    consoleEnabled = enabled;
}

void ckLog_SetConsoleLevel(ckLogLevel level)
{
    consoleLevel = level;
}

