#ifndef DEBUG_H

#include <stdio.h>
#include <stdarg.h>
#ifdef SHOWDEBUG
#define DEBUG(...) fprintf(stderr, __VA_ARGS__) 
#endif

#ifndef DEBUG
#define DEBUG(...)
#endif

#endif /* DEBUG_H */
