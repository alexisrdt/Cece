#ifndef CECE_LOG_H
#define CECE_LOG_H

#include <stddef.h>

#define CC_LOG(...) \
ccLogError(__FILE__, __LINE__, __func__, __VA_ARGS__)

void ccLogError(const char* file, size_t line, const char* function, const char* format, ...);

#endif
