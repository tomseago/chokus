//
//  ck_ops.h
//  chokus
//
//  Created by Tom Seago on 4/7/13.
//
//

#ifndef ck_ops_h
#define ck_ops_h

#include "chokus.h"

struct ckOpType;

/******************************************************************************
 * ckOpLimitRule
 ******************************************************************************/

typedef enum {
    /**
     * A sliding window looks for a count of events within the last t amount
     * of time, anchored at the current moment.
     */
    olrt_SlidingWindow = 0,
    
    /**
     * Buckets are anchored at the epoch and have a boundary every t time
     * apart. Thus one bucket can get filled, but be immediately ok just
     * a moment later because a new bucket has started.
     */
    olrt_Buckets
    
//    /**
//     * Distance is simply the elapsed time since the last occurrence.
//     */
//    olrt_Distance
} ckOpLimitRuleType;


struct ckOpLimitRule {
    ckOpLimitRuleType type;
    
    int allowedCount;
    
    union {
        struct {
            ckTime window;
        } sliding;
        
        struct {
            ckTime length;
            ckTime epoch;
        } bucket;
        
//        struct {
//            ckTime elapsed;
//        } distance;
    } data;
    
    // True if we should just wait until the time at which the request
    // would complete successfully or false if we should just immediately
    // return an error condition. (which has precendence???)
    ckBool waitForSuccess;
    
    
    struct ckOpLimitRule* next;
};
typedef struct ckOpLimitRule ckOpLimitRule;


ckError ckOpLimitRule_CreateSliding(ckOpLimitRule** ruleOut, int allowed, ckTime window, ckBool wait);

ckError ckOpLimitRule_CreateBucket(ckOpLimitRule** ruleOut, int allowed, ckTime length, ckTime epoch, ckBool wait);

//ckError ckOpLimitRule_CreateDistance(ckOpLimitRule** ruleOut, ckTime elapsed);
void ckOpLimitRule_Free(ckOpLimitRule** rule);

typedef enum {
    ckrrOk = 0,
    ckrrWait,
    ckrrError
} ckRuleResult;

/******************************************************************************
 * ckOpLog
 *
 * A record of events that have occurred for a specific id. Such as login
 * attempts from a particular IP address. The IP address would be the id and
 * the ckOpType (see below) would be 'login attempt' or something similar.
 *
 * These are created as events occur and are persisted via whichever storage
 * mechanism the server is using.
 ******************************************************************************/

struct ckOpLog {
    struct ckOpType* opType;
    bstring id;

    ckTime* log;
    int logStart;
    int logLen;
    int logCap;
    
    ckTime overflowedAt;
    
    // Convenience pointer for storage mechanisms
    void* storageData;
};
typedef struct ckOpLog ckOpLog;


ckError ckOpLog_Create(ckOpLog** logOut);
void ckOpLog_Free(ckOpLog** log);

ckError ckOpLog_Log(ckOpLog* log, ckTime time);
ckError ckOpLog_CountAndExpire(ckOpLog* log, ckTime limit, ckBool doExpire);

ckError ckOpLog_CheckRules(ckOpLog* log, ckTime now, ckRuleResult* ruleResultOut, ckTime* waitTimeOut);

/******************************************************************************
 * ckOpType
 *
 * A type or class of operation which the server knows about. This is holds
 * the rules about how individual instances of that operation are allowed to
 * behave.
 *
 * These are defined in the configuration file.
 ******************************************************************************/

struct ckOpType {
    bstring name;
    
    // For recording state storage overflow.
    int overflowCount;
    ckTime overflowLimit;

    ckOpLimitRule* firstRule;
    
    // Convenience pointer for storage mechanisms
    void* storageData;
};
typedef struct ckOpType ckOpType;


ckError ckOpType_Create(ckOpType** typeOut);
void ckOpType_Free(ckOpType** type);

ckError ckOpType_AddRule(ckOpType* type, ckOpLimitRule* rule);



#endif
