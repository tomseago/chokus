//
//  ck_log.c
//  chokus
//
//  Created by Tom Seago on 4/6/13.
//
//

#define NO_EXTERN_LOGGING_DEFS 1

#include <chokus.h>

#include <stdio.h>

int gLogLevel;

ck_bool syslogEnabled = FALSE;
ck_logLevel syslogLevel = CK_LL_TRACE;

ck_bool consoleEnabled = TRUE;
ck_logLevel consoleLevel = CK_LL_TRACE;

char* gLevels[] = {
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG",
    "TRACE"
};

void ck_log(ck_logLevel level, const char* format, ...)
{
    va_list list;
    
    // Sanity checking
    if (!format) return;
    if ((level < CK_LL_ERR) || (level > CK_LL_TRACE))
    {
        level = CK_LL_TRACE;
    }

    va_start(list, format);

    // Write to stdout
    if (consoleEnabled && level >= consoleLevel)
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

void ck_enableSyslog(ck_bool enabled)
{
    syslogEnabled = enabled;
}

void ck_setSyslogLevel(ck_logLevel level)
{
    syslogLevel = level;
}

void ck_enableConsole(ck_bool enabled)
{
    consoleEnabled = enabled;
}

void ck_setConsoleLevel(ck_logLevel level)
{
    consoleLevel = level;
}

