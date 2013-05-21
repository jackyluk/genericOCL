#ifndef DEBUG_H


#ifdef SHOWDEBUG
#include <stdio.h>
#define DEBUG printf
#endif

#ifndef DEBUG
#define DEBUG (void) 
#endif

#endif /* DEBUG_H */
