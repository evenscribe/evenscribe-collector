#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void warn(const char *format, ...);
void info(const char *format, ...);
void error(const char *format, ...);

#endif // LOG_H
