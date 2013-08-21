#include <stdio.h>
#include <stdarg.h>

static FILE *logfile = NULL;

void logToFile(const char *fmt, ...){
	va_list ap;
	if(logfile == NULL){
		logfile = fopen("/tmp/novelcl.log","w+");
	}
	if(logfile){
		va_start(ap, fmt);
		vfprintf(logfile, fmt, ap);
		va_end(ap);
	}else{
		printf("LOG ERROR!\n");
	}
}
