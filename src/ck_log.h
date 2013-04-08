//
//  ck_log.h
//  chokus
//
//  Created by Tom Seago on 4/6/13.
//
//

#ifndef ck_log_h
#define ck_log_h

extern void ckLog_EnableSyslog(ckBool enabled);
extern void ckLog_SetSyslogLevel(ckLogLevel level);
extern void ckLog_EnableConsole(ckBool enabled);
extern void ckLog_SetConsoleLevel(ckLogLevel level);

#endif
