#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <string>

std::string stringf(const char *format, ...);
void log(const char *format, ...);

#endif
