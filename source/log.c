#include "cece/log.h"

#include <stdarg.h>
#include <stdio.h>

void ccLogError(const char* file, size_t line, const char* function, const char* format, ...)
{
	fprintf(stderr, "In file %s, at line %zu, in function %s: ", file, line, function);

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fprintf(stderr, "\n");
}
