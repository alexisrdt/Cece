#include "cece/arguments.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cece/log.h"
#include "cece/memory.h"

CcResult ccParseArguments(unsigned int argumentCount, const char* const* arguments, CcOptions* pOptions)
{
	CcResult result = CC_SUCCESS;

	pOptions->input = NULL;
	pOptions->output = NULL;
	pOptions->version = CC_C17;
	pOptions->tabWidth = 4;

	struct
	{
		bool version: 1;
		bool tabWidth: 1;
	} checks = {0};

	for(size_t argumentIndex = 1; argumentIndex < argumentCount; ++argumentIndex)
	{
		if(strcmp(arguments[argumentIndex], "-o") == 0)
		{
			if(pOptions->output)
			{
				CC_LOG("Multiple outputs specified.");
				result = CC_ERROR_INVALID_ARGUMENT;
				goto error;
			}

			++argumentIndex;
			if(argumentIndex == argumentCount)
			{
				CC_LOG("Missing argument for -o.");
				result = CC_ERROR_INVALID_ARGUMENT;
				goto error;
			}

			pOptions->output = malloc(strlen(arguments[argumentIndex]) + 1);
			if(!pOptions->output)
			{
				CC_LOG("Failed to allocate memory.");
				result = CC_ERROR_OUT_OF_MEMORY;
				goto error;
			}
			strcpy(pOptions->output, arguments[argumentIndex]);

			continue;
		}

		if(strncmp(arguments[argumentIndex], "-std=", 5) == 0)
		{
			if(checks.version)
			{
				CC_LOG("Multiple versions specified.");
				result = CC_ERROR_INVALID_ARGUMENT;
				goto error;
			}

			checks.version = true;

			const char* version = arguments[argumentIndex] + 5;
			if(version[0] != 'c' || version[3] != '\0')
			{
				CC_LOG("Invalid version.");
				result = CC_ERROR_INVALID_ARGUMENT;
				goto error;
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
				CC_LOG("Invalid version.");
				result = CC_ERROR_INVALID_ARGUMENT;
				goto error;
			}

			continue;
		}

		if(strncmp(arguments[argumentIndex], "-tab-width=", 11) == 0)
		{
			if(checks.tabWidth)
			{
				CC_LOG("Multiple tab widths specified.");
				result = CC_ERROR_INVALID_ARGUMENT;
				goto error;
			}

			checks.tabWidth = true;

			const char* const tabWidth = arguments[argumentIndex] + 11;
			char* end;
			unsigned long long value = strtoull(tabWidth, &end, 10);
			if(*end != '\0' || value < 1 || value > 64)
			{
				CC_LOG("Invalid tab width.");
				result = CC_ERROR_INVALID_ARGUMENT;
				goto error;
			}

			pOptions->tabWidth = value;

			continue;
		}

		if(pOptions->input)
		{
			CC_LOG("Multiple inputs specified.");
			result = CC_ERROR_INVALID_ARGUMENT;
			goto error;
		}

		pOptions->input = malloc(strlen(arguments[argumentIndex]) + 1);
		if(!pOptions->input)
		{
			CC_LOG("Failed to allocate memory.");
			result = CC_ERROR_OUT_OF_MEMORY;
			goto error;
		}
		strcpy(pOptions->input, arguments[argumentIndex]);
	}

	if(!pOptions->input)
	{
		CC_LOG("No input specified.");
		result = CC_ERROR_INVALID_ARGUMENT;
		goto error;
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
			CC_LOG("Failed to allocate memory.");
			result = CC_ERROR_OUT_OF_MEMORY;
			goto error;
		}

		strncpy(pOptions->output, pOptions->input, outputLength);
		pOptions->output[outputLength - 2] = '.';
		pOptions->output[outputLength - 1] = 's';
		pOptions->output[outputLength] = '\0';
	}

	return result;

	error:
	CC_FREE(pOptions->input)
	CC_FREE(pOptions->output)
	return result;
}
