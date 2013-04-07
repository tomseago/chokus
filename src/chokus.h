//
//  chokus.h
//  chokus
//
//  Created by Tom Seago on 4/6/13.
//
//

#ifndef chokus_h
#define chokus_h

#include <config.h>
#include <stddef.h>

#include <bsafe.h>
#include <bstrlib.h>

typedef unsigned char ck_bool;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define STRING_FROM_BOOL(b) ((b) ? "true" : "false")

/**
 * Logging
 */

typedef enum {
    CK_LL_ERR = 0,
    CK_LL_WARN = 1,
    CK_LL_INFO = 2,
    CK_LL_DEBUG = 3,
    CK_LL_TRACE = 4
} ck_logLevel;

#ifndef NO_EXTERN_LOGGING_DEFS
extern int gLogLevel;

extern void ck_log(ck_logLevel level, const char* format, ...);
#endif // #ifndef NO_EXTERN_LOGGING_DEFS


#define CK_DEBUG    5
#define CK_INFO

#define ck_trace(...) { if (gLogLevel>=CK_LL_TRACE) { ck_log(CK_LL_TRACE, __VA_ARGS__); } }
#define ck_debug(...) { if (gLogLevel>=CK_LL_DEBUG) { ck_log(CK_LL_DEBUG, __VA_ARGS__); } }
#define ck_info(...) { if (gLogLevel>=CK_LL_INFO) { ck_log(CK_LL_INFO, __VA_ARGS__); } }
#define ck_warn(...) { if (gLogLevel>=CK_LL_WARN) { ck_log(CK_LL_WARN, __VA_ARGS__); } }
#define ck_err(...) { if (gLogLevel>=CK_LL_ERR) { ck_log(CK_LL_ERR, __VA_ARGS__); } }

#endif
