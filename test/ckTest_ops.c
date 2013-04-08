//
//  ckTest_ops.c
//  chokus
//
//  Created by Tom Seago on 4/7/13.
//
//


#include "chokus.h"
#include "assert.h"

#include "ckTest.h"

//#include <unistd.h>
//#include <stdlib.h>

//#include "ck_config.h"

#include "ck_ops.h"


int testOpsLogCounting()
{
    ckError err;
    
    int count = 0;
    ckOpLog* log = NULL;
    
    
    CHK_ERR( ckOpLog_Create(&log) );
    
    CHK_ERR( ckOpLog_Log(log, 10) );
    CHK_ERR( ckOpLog_Log(log, 20) );
    CHK_ERR( ckOpLog_Log(log, 30) );
    CHK_ERR( ckOpLog_Log(log, 40) );
    CHK_ERR( ckOpLog_Log(log, 50) );
    CHK_ERR( ckOpLog_Log(log, 60) );
    CHK_ERR( ckOpLog_Log(log, 70) );
    
    count = ckOpLog_CountAndExpire(log, 0, TRUE);
    assert( count == 7 );
    assert( log->logStart == 0 );
    
    count = ckOpLog_CountAndExpire(log, 15, TRUE);
    assert( count == 6 );
    assert( log->logStart == 1 );
    
    count = ckOpLog_CountAndExpire(log, 15, TRUE);
    assert( count == 6 );
    assert( log->logLen == 6 );
    
    count = ckOpLog_CountAndExpire(log, 20, TRUE);
    assert( count == 6 );
    assert( log->logStart == 1 );
    
    
    count = ckOpLog_CountAndExpire(log, 1000, TRUE);
    assert( count == 0 );
    assert( log->logLen == 0 );
    

error:
    assert( err == cke_Success);
    ck_Info("ok... testOpsLogCounting");

    ckOpLog_Free(&log);
    return err;
}


int testOpsLogCountingWithOverflow()
{
    ckError err;
    
    int count = 0;
    
    ckOpLog* log = NULL;
    ckOpType* otSliding = NULL;
    
    CHK_ERR( ckOpType_Create(&otSliding) );

    otSliding->overflowCount = 3;
    otSliding->overflowLimit = 50;
    
    CHK_ERR( ckOpLog_Create(&log) );
    log->opType = otSliding;
    
    CHK_ERR( ckOpLog_Log(log, 10) );
    assert( log->overflowedAt == 0 );
    
    CHK_ERR( ckOpLog_Log(log, 20) );
    assert( log->overflowedAt == 0 );

    CHK_ERR( ckOpLog_Log(log, 30) );
    assert( log->overflowedAt == 30 );
    count = ckOpLog_CountAndExpire(log, 0, TRUE);
    assert( count == LOG_OVERFLOW );
    
    CHK_ERR( ckOpLog_Log(log, 40) );
    count = ckOpLog_CountAndExpire(log, 0, TRUE);
    assert( count == LOG_OVERFLOW );

    
    CHK_ERR( ckOpLog_Log(log, 100) );
    assert( log->overflowedAt == 0 );
    count = ckOpLog_CountAndExpire(log, 15, TRUE);
    assert( count == 3 );


    
error:
    assert( err == cke_Success);
    ck_Info("ok... testOpsLogCountingWithOverflow");

    ckOpLog_Free(&log);
    return err;
}


int testOpsLogRealloc()
{
    ckError err;
    
    int count = 0;
    int i = 0;
    ckOpLog* log = NULL;
    
    
    CHK_ERR( ckOpLog_Create(&log) );
    
    for (i=0; i<100; i++)
    {
        CHK_ERR( ckOpLog_Log(log, i*10) );
    }
    
    count = ckOpLog_CountAndExpire(log, 985, TRUE);
    assert( count == 1 );

    for (i=100; i<200; i++)
    {
        CHK_ERR( ckOpLog_Log(log, i*10) );
    }
    
    count = ckOpLog_CountAndExpire(log, 1985, TRUE);
    assert( count == 1 );
    assert( log->logLen < 150 );
    
error:
    assert( err == cke_Success);
    ck_Info("ok... testOpsLogRealloc");

    ckOpLog_Free(&log);
    return err;
}



int testOpsSlidingWindow()
{
    ckError err;
    
    ckOpType* otSliding = NULL;
    ckOpLimitRule* ruleSliding = NULL;
    
    ckOpLog* log = NULL;
    
    ckRuleResult ruleResult;
    ckTime waitTime;
    ckTime now;
    
    int i;
    
    CHK_ERR( ckOpType_Create(&otSliding) );
    
    CHK_ERR( ckOpLimitRule_CreateSliding(&ruleSliding, 2, CKTIME_1SEC, TRUE) );
    
    CHK_ERR( ckOpType_AddRule(otSliding, ruleSliding) );
    ruleSliding = NULL; // Because the type now owns it

    //////////////
    CHK_ERR( ckOpLog_Create(&log) );
    log->opType = otSliding;
    
    now = 10;
    CHK_ERR( ckOpLog_Log(log, now) );
    CHK_ERR( ckOpLog_CheckRules(log, now, &ruleResult, &waitTime) );
    assert( ruleResult == ckrrOk );
    
    // 2nd one within the window is ok, but since this has filled up
    // the window, anything that would come in before we got beyond
    // the 10+window mark is a violation and will have to wait until
    // it is at the 15+window mark to not be a violation
    now += 5;
    CHK_ERR( ckOpLog_Log(log, now) );
    CHK_ERR( ckOpLog_CheckRules(log, now, &ruleResult, &waitTime) );
    assert( ruleResult == ckrrOk );
    
    
    // But this 3rd one is no good, so you should wait until the end
    // of the window which started with the 1st event 
    now += 5;
    CHK_ERR( ckOpLog_Log(log, now) );
    // 2nd on within the window is ok
    CHK_ERR( ckOpLog_CheckRules(log, now, &ruleResult, &waitTime) );
    assert( ruleResult == ckrrWait );
    assert( waitTime == 15 + CKTIME_1SEC + 1 - now);
    
    // Wait for that long of time before next op happens
    now += waitTime;
    CHK_ERR( ckOpLog_Log(log, now));
    // Should NOT be a violation....
    CHK_ERR( ckOpLog_CheckRules(log, now, &ruleResult, &waitTime) );
    assert( ruleResult == ckrrOk );

    // And as long as we keep adding 1 second we should be ok
    for (i=0; i<3; i++) {
        now += CKTIME_1SEC;
        CHK_ERR( ckOpLog_Log(log, now));
        // Should NOT be a violation....
        CHK_ERR( ckOpLog_CheckRules(log, now, &ruleResult, &waitTime) );
        assert( ruleResult == ckrrOk );
    }

error:
    assert( err == cke_Success);
    ck_Info("ok... testOpsSlidingWindow");

    ckOpType_Free(&otSliding);
    ckOpLimitRule_Free(&ruleSliding);
    return err;
}



/**
 * re-run the basic thing from previous test but proving that as long as the
 * 3rd entry is in that 10 to 15 space it's ok
 */
int testOpsSlidingWindow2()
{
    ckError err;
    
    ckOpType* otSliding = NULL;
    ckOpLimitRule* ruleSliding = NULL;
    
    ckOpLog* log = NULL;
    
    ckRuleResult ruleResult;
    ckTime waitTime;
    ckTime now;
    
    int i;
    
    CHK_ERR( ckOpType_Create(&otSliding) );
    
    CHK_ERR( ckOpLimitRule_CreateSliding(&ruleSliding, 2, CKTIME_1SEC, TRUE) );
    
    CHK_ERR( ckOpType_AddRule(otSliding, ruleSliding) );
    ruleSliding = NULL; // Because the type now owns it
    
    //////////////
    CHK_ERR( ckOpLog_Create(&log) );
    log->opType = otSliding;
    
    now = 10;
    CHK_ERR( ckOpLog_Log(log, now) );
    CHK_ERR( ckOpLog_CheckRules(log, now, &ruleResult, &waitTime) );
    assert( ruleResult == ckrrOk );
    
    // 2nd one within the window is ok, but since this has filled up
    // the window, anything that would come in before we got beyond
    // the 10+window mark is a violation and will have to wait until
    // it is at the 15+window mark to not be a violation
    now += 5;
    CHK_ERR( ckOpLog_Log(log, now) );
    CHK_ERR( ckOpLog_CheckRules(log, now, &ruleResult, &waitTime) );
    assert( ruleResult == ckrrOk );
    
    
    // So instead of doing an obvious violation by banging away
    // immediately, we wait until basically the minimum point at
    // which we can re-fill the window
    now = 10 + CKTIME_1SEC + 1;
    CHK_ERR( ckOpLog_Log(log, now) );
    CHK_ERR( ckOpLog_CheckRules(log, now, &ruleResult, &waitTime) );
    assert( ruleResult == ckrrOk );

    // And as long as we keep adding 1 second we should be ok
    for (i=0; i<3; i++) {
        now += CKTIME_1SEC;
        CHK_ERR( ckOpLog_Log(log, now));
        // Should NOT be a violation....
        CHK_ERR( ckOpLog_CheckRules(log, now, &ruleResult, &waitTime) );
        assert( ruleResult == ckrrOk );
    }
    
error:
    assert( err == cke_Success);
    ck_Info("ok... testOpsSlidingWindow2");
    
    ckOpType_Free(&otSliding);
    ckOpLimitRule_Free(&ruleSliding);
    return err;
}



int testOps()
{
    testOpsLogCounting();
    testOpsLogCountingWithOverflow();
    testOpsLogRealloc();
    
    testOpsSlidingWindow();
    testOpsSlidingWindow2();
    
    return 0;
}
