#ifndef DEBUG_H
#include <stdio.h>
#include <stdarg.h>

extern void logToFile(const char *fmt, ...);
#ifdef SHOWDEBUG
#define DEBUG(...) logToFile(__VA_ARGS__)
#endif

#ifndef DEBUG
#define DEBUG(...)  
#endif

#endif /* DEBUG_H */
