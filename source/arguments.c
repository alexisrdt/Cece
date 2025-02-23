#include "cece/arguments.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cece/memory.h"

CcResult ccParseArguments(const size_t argumentCount, const char* const* const arguments, CcOptions* const pOptions)
{
	// Validate arguments.
	assert(argumentCount > 0);
	assert(arguments != nullptr);
	for(size_t argumentIndex = 0; argumentIndex < argumentCount; ++argumentIndex)
	{
		assert(arguments[argumentIndex] != nullptr);
	}
	assert(pOptions != nullptr);

	CcResult result = CC_SUCCESS;

	*pOptions = (CcOptions){
		.version = CC_C23
	};

	struct
	{
		bool version: 1;
		bool debug: 1;
	} checks = {};

	for(size_t argumentIndex = 0; argumentIndex < argumentCount; ++argumentIndex)
	{
		if(strcmp(arguments[argumentIndex], "-h") == 0)
		{
			pOptions->usage = true;

			goto clear;
		}

		if(strcmp(arguments[argumentIndex], "-o") == 0)
		{
			if(pOptions->output)
			{
				fputs("Multiple outputs specified.\n", stderr);
				result = CC_ERROR_INVALID_ARGUMENT;
				goto clear;
			}

			++argumentIndex;
			if(argumentIndex == argumentCount)
			{
				fputs("Missing argument for -o.\n", stderr);
				result = CC_ERROR_INVALID_ARGUMENT;
				goto clear;
			}

			pOptions->output = strdup(arguments[argumentIndex]);
			if(!pOptions->output)
			{
				fputs("Failed to allocate memory.\n", stderr);
				result = CC_ERROR_OUT_OF_MEMORY;
				goto clear;
			}

			continue;
		}

		if(strncmp(arguments[argumentIndex], "-std=", 5) == 0)
		{
			if(checks.version)
			{
				fputs("Multiple versions specified.\n", stderr);
				result = CC_ERROR_INVALID_ARGUMENT;
				goto clear;
			}

			checks.version = true;

			const char* version = arguments[argumentIndex] + 5;
			if(version[0] != 'c' || version[1] == '\0' || version[2] == '\0' || version[3] != '\0')
			{
				fputs("Invalid version.\n", stderr);
				result = CC_ERROR_INVALID_ARGUMENT;
				goto clear;
			}

			++version;

			if(strcmp(version, "90") == 0)
			{
				pOptions->version = CC_C90;
			}
			else if(strcmp(version, "99") == 0)
			{
				pOptions->version = CC_C99;
			}
			else if(strcmp(version, "11") == 0)
			{
				pOptions->version = CC_C11;
			}
			else if(strcmp(version, "17") == 0)
			{
				pOptions->version = CC_C17;
			}
			else if(strcmp(version, "23") == 0)
			{
				pOptions->version = CC_C23;
			}
			else
			{
				fputs("Invalid version.\n", stderr);
				result = CC_ERROR_INVALID_ARGUMENT;
				goto clear;
			}

			continue;
		}

		if(strcmp(arguments[argumentIndex], "-g") == 0)
		{
			if(checks.debug)
			{
				fputs("Multiple debug specified.", stderr);
				result = CC_ERROR_INVALID_ARGUMENT;
				goto clear;
			}

			checks.debug = true;

			pOptions->debug = true;

			continue;
		}

		if(arguments[argumentIndex][0] == '-')
		{
			fprintf(stderr, "Unknown option: %s\n", arguments[argumentIndex]);
			result = CC_ERROR_INVALID_ARGUMENT;
			goto clear;
		}

		if(pOptions->input)
		{
			fputs("Multiple inputs specified.\n", stderr);
			result = CC_ERROR_INVALID_ARGUMENT;
			goto clear;
		}

		pOptions->input = strdup(arguments[argumentIndex]);
		if(!pOptions->input)
		{
			fputs("Failed to allocate memory.\n", stderr);
			result = CC_ERROR_OUT_OF_MEMORY;
			goto clear;
		}
	}

	if(!pOptions->input)
	{
		fputs("No input specified.\n", stderr);
		result = CC_ERROR_INVALID_ARGUMENT;
		goto clear;
	}

	if(!pOptions->output)
	{
		const char* const lastDot = strrchr(pOptions->input, '.');
		const char* const lastSlash = strrchr(pOptions->input, '/');
		const char* const lastBackslash = strrchr(pOptions->input, '\\');

		const char* const lastSeparator = lastSlash > lastBackslash ? lastSlash : lastBackslash;

		const size_t outputLength = (lastDot > lastSeparator ? (size_t)(lastDot - pOptions->input) : strlen(pOptions->input)) + 2;
		pOptions->output = malloc(outputLength + 1);
		if(!pOptions->output)
		{
			fputs("Failed to allocate memory.\n", stderr);
			result = CC_ERROR_OUT_OF_MEMORY;
			goto clear;
		}

		strncpy(pOptions->output, pOptions->input, outputLength);
		pOptions->output[outputLength - 2] = '.';
		pOptions->output[outputLength - 1] = 's';
		pOptions->output[outputLength] = '\0';
	}

	goto end;

	clear:
	CC_FREE(pOptions->input);
	CC_FREE(pOptions->output);

	end:
	return result;
}
