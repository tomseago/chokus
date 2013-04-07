//
//  ck_log.h
//  chokus
//
//  Created by Tom Seago on 4/6/13.
//
//

#ifndef ck_log_h
#define ck_log_h

extern void ck_enableSyslog(ck_bool enabled);
extern void ck_setSyslogLevel(ck_logLevel level);
extern void ck_enableConsole(ck_bool enabled);
extern void ck_setConsoleLevel(ck_logLevel level);

#endif
