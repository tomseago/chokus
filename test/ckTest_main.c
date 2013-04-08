//
//  ckTest_main.c
//  chokus
//
//  Created by Tom Seago on 4/7/13.
//
//

#include "chokus.h"
#include <unistd.h>
#include <stdlib.h>

#include "ck_log.h"

#include "assert.h"




int testOps();



typedef int (*testFunc)();



int main(int argc, char** argv)
{
    ckLog_EnableSyslog(FALSE);
    ckLog_EnableConsole(TRUE);
    ckLog_SetConsoleLevel(CKLL_TRACE);
    gLogLevel = CKLL_TRACE;
    
    testOps();
    
    return 0;
}

