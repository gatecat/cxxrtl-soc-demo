#include "log.h"

#include <stdlib.h>
#include <string>
#include <iostream>

std::string vstringf(const char *fmt, va_list ap)
{
	std::string string;
	char *str = NULL;

#if defined(_WIN32) || defined(__CYGWIN__)
	int sz = 64 + strlen(fmt), rc;
	while (1) {
		va_list apc;
		va_copy(apc, ap);
		str = (char *)realloc(str, sz);
		rc = vsnprintf(str, sz, fmt, apc);
		va_end(apc);
		if (rc >= 0 && rc < sz)
			break;
		sz *= 2;
	}
#else
	if (vasprintf(&str, fmt, ap) < 0)
		str = NULL;
#endif

	if (str != NULL) {
		string = str;
		free(str);
	}

	return string;
}

std::string stringf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	std::string result = vstringf(format, ap);
	va_end(ap);
	return result;
}

void logv(const char *format, va_list ap)
{
	std::string str = vstringf(format, ap);
	if (str.empty())
		return;
	std::cerr << str;
}


void log(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	logv(format, ap);
	va_end(ap);
}