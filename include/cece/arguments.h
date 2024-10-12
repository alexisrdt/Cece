#ifndef CECE_ARGUMENTS_H
#define CECE_ARGUMENTS_H

#include <stddef.h>

#include "cece/result.h"

typedef enum CcVersion
{
	CC_C90,
	CC_C99,
	CC_C11,
	CC_C17,
	CC_C23
} CcVersion;

typedef struct CcOptions
{
	char* input;
	char* output;

	CcVersion version;

	size_t tabWidth;
} CcOptions;

CcResult ccParseArguments(unsigned int argumentCount, const char* const* arguments, CcOptions* pOptions);

#endif
