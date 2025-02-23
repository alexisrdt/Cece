#ifndef CECE_ARGUMENTS_H
#define CECE_ARGUMENTS_H

#include <stddef.h>

#include "cece/result.h"

/*
 * A C standard version.
 */
typedef enum CcVersion: long
{
	CC_C90,
	CC_C95 = 199409L,
	CC_C99 = 199901L,
	CC_C11 = 201112L,
	CC_C17 = 201710L,
	CC_C23 = 202311L
} CcVersion;

/*
 * The compiling options.
 *
 * Fields:
 * - input: The path to the file to compile.
 * - output: The path to write the result to.
 * - version: The version of the C standard to use.
 * - debug: Switch to compile in debug or release mode.
 */
typedef struct CcOptions
{
	char* input;
	char* output;

	CcVersion version;

	bool debug: 1;
	bool usage: 1;
} CcOptions;

/*
 * Parse arguments.
 *
 * Parameters:
 * - argumentCount: The number of arguments.
 * - arguments: The arguments.
 * - pOptions: A pointer to the object storing compiling options.
 *
 * Returns:
 * - CC_SUCCESS on success.
 * - CC_ERROR_INVALID_ARGUMENT if an invalid argument was passed.
 * - CC_ERROR_OUT_OF_MEMORY if file paths could not be allocated.
 */
CcResult ccParseArguments(size_t argumentCount, const char* const* arguments, CcOptions* pOptions);

#endif
