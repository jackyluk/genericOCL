#ifndef DEBUG_H


#ifdef SHOWDEBUG
#include <stdio.h>
#include <stdarg.h>
#define DEBUG(...) fprintf(stderr, __VA_ARGS__) 
#endif

#ifndef DEBUG
#define DEBUG (void) 
#endif

#endif /* DEBUG_H */
