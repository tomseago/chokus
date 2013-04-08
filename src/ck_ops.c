//
//  ck_ops.c
//  chokus
//
//  Created by Tom Seago on 4/7/13.
//
//

#include "chokus.h"
#include <stdlib.h>

#include "ck_ops.h"



// Amount to realloc each time the log array fills up
#define REALLOC_AMOUNT 10



/******************************************************************************/

static ckError allocRule(ckOpLimitRule** ruleOut)
{
    ckOpLimitRule* rule = NULL;
    
    if (!ruleOut) return cke_BadParam;
    
    rule = malloc(sizeof(ckOpLimitRule));
    if (!rule) return cke_OutOfMemory;
    
    memset(rule, 0, sizeof(ckOpLimitRule));
    
    *ruleOut = rule;
    return cke_Success;
}



ckError ckOpLimitRule_CreateSliding(ckOpLimitRule** ruleOut, int allowed, ckTime window, ckBool wait)
{
    ckError err = cke_General;
    
    CHK_ERR( allocRule(ruleOut) );

    (*ruleOut)->allowedCount = allowed;
    (*ruleOut)->waitForSuccess = wait;

    (*ruleOut)->type = olrt_SlidingWindow;
    (*ruleOut)->data.sliding.window = window;
    
    err = cke_Success;
    
error:
    return err;
}



ckError ckOpLimitRule_CreateBucket(ckOpLimitRule** ruleOut, int allowed, ckTime length, ckTime epoch, ckBool wait)
{
    ckError err = cke_General;
    
    CHK_ERR( allocRule(ruleOut) );
    
    (*ruleOut)->allowedCount = allowed;
    (*ruleOut)->waitForSuccess = wait;
    
    (*ruleOut)->type = olrt_Buckets;
    (*ruleOut)->data.bucket.length = length;
    (*ruleOut)->data.bucket.epoch = epoch;
    
    err = cke_Success;
    
error:
    return err;
}

void ckOpLimitRule_Free(ckOpLimitRule** rule)
{
    if (rule && *rule) free(*rule);
    *rule = NULL;
}



/******************************************************************************/

ckError ckOpLog_Create(ckOpLog** logOut)
{
    ckOpLog* log = NULL;
    
    if (!logOut) return cke_BadParam;
    
    log = malloc(sizeof(ckOpLog));
    if (!log) return cke_OutOfMemory;
    
    memset(log, 0, sizeof(ckOpLog));
    
    log->logCap = REALLOC_AMOUNT;
    log->log = malloc(log->logCap * sizeof(ckTime));
    if (!log->log)
    {
        free(log);
        return cke_OutOfMemory;
    }
    
    *logOut = log;
    return cke_Success;
}



void ckOpLog_Free(ckOpLog** log)
{
    if (log && *log) {
        if ((*log)->log) free((*log)->log);
        free(*log);
    }
    
    *log = NULL;
}



/**
 * Clears the overflow condition from a given log, which removes all recorded
 * log values.
 */
//void resetOverflow(ckOpLog* log)
//{
//    log->overflowedAt = 0;
//    log->logStart = 0;
//    log->logLen = 0;
//}


ckError ckOpLog_Log(ckOpLog* log, ckTime time)
{
    if (!log || !log->log) return cke_BadParam;

    // Start out by deciding if we want to log it at all. If in an overflow
    // condition we don't log until the overflow is cleared.
    if (log->opType && log->opType->overflowLimit)
    {
        
        if (log->overflowedAt) {
            if ((time - log->overflowedAt) > log->opType->overflowLimit)
            {
                // We had previously overflowed, but the condition is now cleared
                //resetOverflow(log);
                log->overflowedAt = 0;
            }
            else
            {
                // The overflow is still in effect so we don't actually add this
                return cke_Success;
            }
        }
        // else  It has not previously overflowed, but maybe it will now after we
        // add the thing we're about to add
    }
    
    // Ensure there is space for it
    if (log->logStart + log->logLen >= log->logCap)
    {
        // No more space. Let's see if we can shift stuff down by a good amount
        // in which case we don't realloc.
        if (log->logStart > log->logCap/2)
        {
            // Shift stuff down
            memmove(log->log, (log->log + log->logStart), (log->logLen * sizeof(ckTime)));
            log->logStart = 0;
        }
        else
        {
            // Let's grow the array by a fixed number of elements
            ckTime* newArray = realloc(log->log, (log->logCap + REALLOC_AMOUNT) * sizeof(ckTime));
            if (!newArray) return cke_OutOfMemory;
            log->log = newArray;
            log->logCap += REALLOC_AMOUNT;
        }
    }
    
    // There is definitely space to add at least one more log entry, so do that
    *(log->log + log->logStart + log->logLen) = time;
    log->logLen++;
    
    // Now check to see if we've just introduced an overflow condition
    if (log->opType && log->opType->overflowLimit)
    {
        ckTime* first = log->log + log->logStart;
        ckTime* last = first + log->logLen - 1;
        ckTime overflowWindowStart = time - log->opType->overflowLimit;
        int count = 0;
        
        while((*last >= overflowWindowStart) && (last >= first))
        {
            count++;
            last--;
            
            if (count >= log->opType->overflowCount)
            {
                // Overflow condition has occurred!!!!
                
                // TODO: Log, generate an event, etc.... Probably at a higher level though
                log->overflowedAt = time;
                break;
            }
        }
    }
    
    return cke_Success;
}



/**
 * Counts the number of events back until the given absolute time limit (>= limit).
 * Anything prior to that limit not only doesn't get counted, but also gets cleared.
 */
int ckOpLog_CountAndExpire(ckOpLog* log, ckTime limit, ckBool doExpire)
{
    
    ckTime* first = NULL;
    ckTime* last = NULL;
    int count = 0;

    if (!log) return 0;
    
    // Are we in an overflow condition?
    if (log->overflowedAt && (log->overflowedAt > limit))
    {
        // Yeppers report it
        return LOG_OVERFLOW;
        
        // While we could go ahead and nuke storage here up to the limit the caller
        // cares about, it's not that useful of an optimization because it will just
        // happen later anyway.
    }
    
    // Nothing to do if there is nothing recorded
    if (log->logLen == 0 || !log->log) return 0;
    
    // Setup some markers and then use them
    first = log->log + log->logStart;
    last = first + log->logLen - 1;
    
    while((*last >= limit) && (last >= first))
    {
        count++;
        last--;
    }
    
    // Expire things out if we are asked to do so.
    if (doExpire && (last >= first))
    {
        // there are some limits that are no longer needed. Nuke them.
        int distance = (int)(last - first) + 1;
        
        log->logStart += distance;
        log->logLen -= distance;
    }
    
    return count;
}


/**
 * Kind of like count and expire except does an early out when the count is exceeded.
 */
ckBool ckOpLog_HasToManyEvents(ckOpLog* log, ckTime limit, int maxCount)
{
    ckTime* first = NULL;
    ckTime* last = NULL;
    int count = 0;
    
    if (!log || !limit || !maxCount) return FALSE;
    
    // Are we in an overflow condition?
    if (log->overflowedAt && (log->overflowedAt > limit))
    {
        // Yeppers report it
        return TRUE;
        
        // While we could go ahead and nuke storage here up to the limit the caller
        // cares about, it's not that useful of an optimization because it will just
        // happen later anyway.
    }

    // Nothing to do if there is nothing recorded
    if (log->logLen == 0 || !log->log) return FALSE;
    
    // Setup some markers and then use them
    first = log->log + log->logStart;
    last = first + log->logLen - 1;
    
    while((*last >= limit) && (last >= first))
    {
        count++;
        last--;
        
        // Check for early out condition
        if (count > maxCount) return TRUE;
    }
    
    // Made it, so we didn't hit the count limit. All is well
    return FALSE;
}


ckTime ckOpLog_FindTimeoutForSlidingWindow(ckOpLog* log, ckTime now, ckTime windowSize, int maxCount)
{
    ckTime* first = NULL;
    ckTime* last = NULL;
    ckTime prevValue = 0;
    ckTime limit = now - windowSize;
    int count = 0;
    
    if (!log || !limit || !maxCount) return 0;
    
    // Nothing to do if there is nothing recorded
    if (log->logLen == 0 || !log->log) return 0;
    
    // Setup some markers and then use them
    first = log->log + log->logStart;
    last = first + log->logLen - 1;
    prevValue = *last;
    
    while((*last >= limit) && (last >= first))
    {
        count++;        

        // Check for early out condition
        if (count > maxCount) break;

        prevValue = *last;
        last--;
    }
    
    if (last < first) {
        // There actually wasn't a violation, so no timeout is needed
        return 0;
    }
    
    // There is at least 1, so we know last has a value
    // And the value at last is the time at which the violation happened
    // So we add the window distance + 1ms and we'll be free to do one more then
    return prevValue + windowSize + 1;
}


/**
 * The main thing that outside people care about. They want to add something to the log
 * and then see if this operation has been violated.
 */
ckError ckOpLog_CheckRules(ckOpLog* log, ckTime now, ckRuleResult* ruleResultOut, ckTime* waitTimeOut)
{
    ckOpLimitRule* rule = NULL;
    ckTime oldestLimit = LLONG_MAX;

    if (!log) return cke_BadParam;
    if (!log->opType) return cke_BadParam;
    if (!ruleResultOut || !waitTimeOut) return cke_BadParam;
    
    *ruleResultOut = ckrrOk;
    *waitTimeOut = 0;
    
    rule = log->opType->firstRule;
    
    while(rule)
    {
        // Figure out the absolute time to check this rule against        
        ckTime limit = 0;
//        ckTime now = ck_Now();
        ckTime bucketIx = 0;
//        int count = 0;
        
        switch (rule->type) {
            case olrt_SlidingWindow:
                limit = now - rule->data.sliding.window;
                break;

            case olrt_Buckets:
            {
                ckTime distance = now - rule->data.bucket.epoch;
                bucketIx = distance / rule->data.bucket.length;
                limit = bucketIx * rule->data.bucket.length;
            }
                break;
                
            default:
                ck_Err("Unknown rule type!");
                break;
        }
        
        if (limit < oldestLimit)
        {
            oldestLimit = limit;
        }
        
        // How many things have happened in the window?
        //count = ckOpLog_CountAndExpire(log, limit, FALSE);
        
        // And how do we feel about that?
        if ( ckOpLog_HasToManyEvents(log, limit, rule->allowedCount) )
        {
            // VIOLATION!!!!
            // A nasty one, or a nice one?
            ckRuleResult thisResult = rule->waitForSuccess ? ckrrWait : ckrrError;
            
            if ((*ruleResultOut == ckrrError) || (thisResult == ckrrError))
            {
                *ruleResultOut = ckrrError;
            }
            else if ((*ruleResultOut == ckrrWait) || (thisResult == ckrrWait))
            {
                // Still wait
                *ruleResultOut = ckrrWait;
            }
            // else both are Ok, and thus we wouldn't be in here. Leave it alone
            
            // Now how about the timeout value? Only care in wait condition
            if (*ruleResultOut == ckrrWait)
            {
                ckTime thisTimeout = 0;
                
                switch(rule->type) {
                    case olrt_SlidingWindow:
                        thisTimeout = ckOpLog_FindTimeoutForSlidingWindow(log, now, rule->data.sliding.window, rule->allowedCount);
                        break;
                        
                    case olrt_Buckets:
                        thisTimeout = (bucketIx + 1) * rule->data.bucket.length;
                        break;
                        
                    default:
                        break;
                }
                
                thisTimeout -= now;
                
                if (thisTimeout > *waitTimeOut)
                {
                    *waitTimeOut = thisTimeout;
                }
            }
            
        }
        
        rule = rule->next;
    } // while(rule)
    
    if (oldestLimit < LLONG_MAX)
    {
        ckOpLog_CountAndExpire(log, oldestLimit, TRUE);
    }
    
    return cke_Success;
}


ckError ckOpType_Create(ckOpType** typeOut)
{
    ckOpType* type = NULL;
    
    if (!typeOut) return cke_BadParam;
    
    type = malloc(sizeof(ckOpType));
    if (!type) return cke_OutOfMemory;
    
    memset(type, 0, sizeof(ckOpType));
    
    *typeOut = type;
    return cke_Success;
}



void ckOpType_Free(ckOpType** type)
{
    ckOpLimitRule* cursor = NULL;
    
    if (!type || !*type) return;

    bdestroy((*type)->name);
    
    cursor = (*type)->firstRule;
    while(cursor)
    {
        ckOpLimitRule* toFree = cursor;
        
        cursor = cursor->next;
        ckOpLimitRule_Free(&toFree);
    }
    
    free(*type);
    *type = NULL;
}


ckError ckOpType_AddRule(ckOpType* type, ckOpLimitRule* rule)
{
    if (!type) return cke_BadParam;
    
    if (!type->firstRule) {
        type->firstRule = rule;
    } else {
        // Just walk to the end of the list. Not efficient, but you don't do this
        // all that often.
        ckOpLimitRule* cursor;

        cursor = type->firstRule;
        while (cursor->next) cursor = cursor->next;
        
        cursor->next = rule;
    }

    rule->next = NULL;

    
    return cke_Success;
}

