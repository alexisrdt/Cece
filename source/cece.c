#include "cece/cece.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cece/log.h"

CcResult ccReadFile(const char* filePath, CcString* pString)
{
	CcResult result = CC_SUCCESS;

	FILE* const file = fopen(filePath, "r");
	if(!file)
	{
		CC_LOG("Failed to open file \"%s\".", filePath);
		result = CC_ERROR_FILE_NOT_FOUND;
		return result;
	}

	size_t capacity = 1024;
	pString->string = malloc(capacity);
	if(!pString->string)
	{
		CC_LOG("Failed to allocate memory.");
		result = CC_ERROR_OUT_OF_MEMORY;
		goto error;
	}

	pString->length = 0;
	size_t toRead = capacity;
	while(true)
	{
		const size_t read = fread(pString->string + pString->length, sizeof(pString->string[0]), toRead, file);
		if(SIZE_MAX - pString->length < read)
		{
			CC_LOG("Failed to read file \"%s\".", filePath);
			result = CC_ERROR_UNKNOWN;
			goto error;
		}
		pString->length += read;

		if(read < toRead)
		{
			if(!feof(file))
			{
				CC_LOG("Failed to read file \"%s\".", filePath);
				result = CC_ERROR_UNKNOWN;
				goto error;
			}

			if(pString->length == SIZE_MAX)
			{
				CC_LOG("Failed to read file \"%s\".", filePath);
				result = CC_ERROR_UNKNOWN;
				goto error;
			}

			char* const newString = realloc(pString->string, pString->length + 1);
			if(!newString)
			{
				CC_LOG("Failed to allocate memory.");
				result = CC_ERROR_OUT_OF_MEMORY;
				goto error;
			}
			pString->string = newString;
			pString->string[pString->length] = '\0';

			fclose(file);

			return result;
		}

		if(capacity == SIZE_MAX)
		{
			CC_LOG("Failed to allocate memory.");
			result = CC_ERROR_UNKNOWN;
			goto error;
		}

		toRead = capacity;
		capacity = capacity > SIZE_MAX / 2 ? SIZE_MAX : capacity * 2;

		char* const newString = realloc(pString->string, capacity);
		if(!newString)
		{
			CC_LOG("Failed to allocate memory.");
			result = CC_ERROR_OUT_OF_MEMORY;
			goto error;
		}
		pString->string = newString;
	}

	error:
	CC_FREE(pString->string)
	fclose(file);
	return result;
}
