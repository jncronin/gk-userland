#ifndef SYSLOG_H
#define SYSLOG_H

#define LOG_PID         1
#define LOG_CONS        2
#define LOG_NDELAY      4
#define LOG_ODELAY      8
#define LOG_NOWAIT      16

#define LOG_KERN        0
#define LOG_USER        1
#define LOG_MAIL        2
#define LOG_NEWS        3
#define LOG_UUCP        4
#define LOG_DAEMON      5
#define LOG_AUTH        6
#define LOG_CRON        7
#define LOG_LPR         8
#define LOG_LOCAL0      9
#define LOG_LOCAL1      10
#define LOG_LOCAL2      11
#define LOG_LOCAL3      12
#define LOG_LOCAL4      13
#define LOG_LOCAL5      14
#define LOG_LOCAL6      15
#define LOG_LOCAL7      16

#define LOG_EMERG       0
#define LOG_ALERT       1
#define LOG_CRIT        2
#define LOG_ERR         3
#define LOG_WARNING     4
#define LOG_NOTICE      5
#define LOG_INFO        6
#define LOG_DEBUG       7

void  closelog(void);
void  openlog(const char *, int, int);
int   setlogmask(int);
void  syslog(int, const char *, ...);

#endif
