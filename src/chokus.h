//
//  chokus.h
//  chokus
//
//  Created by Tom Seago on 4/6/13.
//
//

#ifndef chokus_h
#define chokus_h

#include "config.h"
#include <stddef.h>
#include <limits.h>

#include "bsafe.h"
#include "bstrlib.h"

typedef unsigned char ckBool;
typedef long long ckTime;

typedef enum {
    cke_Success     = 0,
    cke_General     = -1,
    cke_BadParam    = -2,
    cke_OutOfMemory = -3
} ckError;

#define CKTIME_1SEC     (1000000)
#define CKTIME_1MINUTE  (60 * CKTIME_1SEC)
#define CKTIME_1HOUR    (60 * CKTIME_1MINUTE)

#define LOG_OVERFLOW    (INT_MAX)


#ifndef FALSE
#define FALSE           0
#endif

#ifndef TRUE
#define TRUE            1
#endif

#define STRING_FROM_BOOL(b) ((b) ? "true" : "false")

#define CHK_ERR( x ) { err = (x); if (err != cke_Success) goto error; }

/**
 * Logging
 */

typedef enum {
    CKLL_ERR = 0,
    CKLL_WARN = 1,
    CKLL_INFO = 2,
    CKLL_DEBUG = 3,
    CKLL_TRACE = 4
} ckLogLevel;

#ifndef NO_EXTERN_LOGGING_DEFS
extern int gLogLevel;
#endif // #ifndef NO_EXTERN_LOGGING_DEFS

void ckLog_Log(ckLogLevel level, const char* format, ...);
ckTime ck_Now();


#define CK_DEBUG    5
#define CK_INFO

#define ck_Trace(...) { if (gLogLevel>=CKLL_TRACE) { ckLog_Log(CKLL_TRACE, __VA_ARGS__); } }
#define ck_Debug(...) { if (gLogLevel>=CKLL_DEBUG) { ckLog_Log(CKLL_DEBUG, __VA_ARGS__); } }
#define ck_Info(...) { if (gLogLevel>=CKLL_INFO) { ckLog_Log(CKLL_INFO, __VA_ARGS__); } }
#define ck_Warn(...) { if (gLogLevel>=CKLL_WARN) { ckLog_Log(CKLL_WARN, __VA_ARGS__); } }
#define ck_Warning(...) { if (gLogLevel>=CKLL_WARN) { ckLog_Log(CKLL_WARN, __VA_ARGS__); } }
#define ck_Err(...) { if (gLogLevel>=CKLL_ERR) { ckLog_Log(CKLL_ERR, __VA_ARGS__); } }
#define ck_Error(...) { if (gLogLevel>=CKLL_ERR) { ckLog_Log(CKLL_ERR, __VA_ARGS__); } }


#endif
