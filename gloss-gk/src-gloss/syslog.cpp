#include <syslog.h>
#include <cstdio>
#include <cstdarg>

extern "C" void openlog(const char *ident, int option, int facility)
{

}

extern "C" void syslog(int priority, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    fprintf(stderr, "SYSLOG [%d]: ", priority);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

extern "C" void closelog(void)
{
}

extern "C" void vsyslog(int priority, const char *format, va_list ap)
{
    fprintf(stderr, "SYSLOG [%d]: ", priority);
    vfprintf(stderr, format, ap);
}
