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

int forkAnd(testFunc toRun)
{
#ifndef CK_NO_TEST_FORK
    pid_t kid;
    
    kid = fork();
    if (kid) {
        // Parent
        int stat_loc;
        int options;
        struct rusage rusage;
        
        
    } else {
        // Child
        exit(toRun());
    }
#else
    return toRun();
#endif
}



int main(int argc, char** argv)
{
    ckLog_EnableSyslog(FALSE);
    ckLog_EnableConsole(TRUE);
    ckLog_SetConsoleLevel(CKLL_TRACE);
    gLogLevel = CKLL_TRACE;
    
    forkAnd(testOps);
    
	return 0;
}

