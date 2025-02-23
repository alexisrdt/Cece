#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cece/cece.h"

int main(int argumentCount, char** arguments)
{
	CcResult result = CC_SUCCESS;

	// Check that there are command line arguments.
	if(argumentCount <= 1)
	{
		ccPrintUsage(stderr);

		result = CC_ERROR_INVALID_ARGUMENT;
		goto end;
	}

	--argumentCount;
	++arguments;

	constexpr size_t limit = ccSizeMax / sizeof(arguments[0]);

#if INT_MAX <= SIZE_MAX
	if(limit < (size_t)argumentCount)
#else
	if((unsigned int)limit < (unsigned int)argumentCount)
#endif
	{
		fputs("Too many arguments.\n", stderr);
		result = CC_ERROR_INVALID_ARGUMENT;
		goto end;
	}

	const size_t argumentsSize = argumentCount * sizeof(arguments[0]);
	const char** const argumentsConst = malloc(argumentsSize);
	if(!argumentsConst)
	{
		fputs("Out of memory.\n", stderr);
		result = CC_ERROR_OUT_OF_MEMORY;
		goto end;
	}
	memcpy(argumentsConst, arguments, argumentsSize);

	// Parse command line arguments.
	CcOptions options;
	result = ccParseArguments(argumentCount, argumentsConst, &options);
	free(argumentsConst);
	if(result != CC_SUCCESS)
	{
		goto end;
	}

	// If usage, print usage.
	if(options.usage)
	{
		ccPrintUsage(stdout);

		goto end;
	}

	// Compile file.
	result = ccCompile(&options);
	free(options.input);
	free(options.output);

	end:
	return result == CC_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}
