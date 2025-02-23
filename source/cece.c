#include "cece/cece.h"

#include <assert.h>
#include <stdlib.h>

// Initial buffer size for file reading.
static constexpr size_t ccInitialSize = 1024;

void ccPrintUsage(FILE* const file)
{
	assert(file != nullptr);

	fputs("Usage: cece [options] <file-to-compile>\n", file);
}

CcResult ccReadFile(const char* const path, CcString* const pString)
{
	// Validate arguments.
	assert(path != nullptr);
	assert(pString != nullptr);

	pString->string = nullptr;

	CcResult result = CC_SUCCESS;

	// Open file.
	FILE* const file = fopen(path, "r");
	if(!file)
	{
		result = CC_ERROR_FILE_NOT_FOUND;
		goto error;
	}

	// Allocate initial buffer.
	size_t capacity = ccInitialSize;
	pString->string = malloc(capacity);
	if(!pString->string)
	{
		result = CC_ERROR_OUT_OF_MEMORY;
		goto error;
	}

	pString->length = 0;
	size_t toRead = capacity;
	while(true)
	{
		const size_t read = fread(pString->string + pString->length, sizeof(pString->string[0]), toRead, file);
		pString->length += read;

		if(read < toRead)
		{
			if(!feof(file))
			{
				result = CC_ERROR_UNKNOWN;
				goto error;
			}

			if(pString->length >= ccSizeMax)
			{
				result = CC_ERROR_OUT_OF_MEMORY;
				goto error;
			}

			char* const newString = realloc(pString->string, pString->length + 1);
			if(!newString)
			{
				result = CC_ERROR_OUT_OF_MEMORY;
				goto error;
			}
			pString->string = newString;
			pString->string[pString->length] = '\0';

			result = CC_SUCCESS;
			goto end;
		}

		if(capacity >= ccSizeMax)
		{
			result = CC_ERROR_OUT_OF_MEMORY;
			goto error;
		}

		const size_t newCapacity = capacity > ccSizeMax / 2 ? ccSizeMax : capacity * 2;
		toRead = newCapacity - capacity;
		capacity = newCapacity;

		char* const newString = realloc(pString->string, capacity);
		if(!newString)
		{
			result = CC_ERROR_OUT_OF_MEMORY;
			goto error;
		}
		pString->string = newString;
	}

	error:
	CC_FREE(pString->string);
	pString->length = 0;

	end:
	fclose(file);

	return result;
}

CcResult ccCompile(const CcOptions* const pOptions)
{
	CcResult result = CC_SUCCESS;

	// Get source code.
	CcString source = {};
	result = ccReadFile(pOptions->input, &source);
	if(result != CC_SUCCESS)
	{
		switch(result)
		{
			case CC_ERROR_FILE_NOT_FOUND:
				fprintf(stderr, "Failed to open file \"%s\".\n", pOptions->input);
				break;

			case CC_ERROR_OUT_OF_MEMORY:
				fputs("Out of memory.\n", stderr);
				break;

			default:
				fputs("Unknown error occured.\n", stderr);
				break;
		}

		goto end;
	}

	const CcConstString constString = {.string = source.string, .length = source.length};
	CcTokenList tokenList;
	result = ccLex(constString, &tokenList);
	if(result != CC_SUCCESS)
	{
		goto end;
	}

	CcTree tree;
	result = ccParse(&(const CcConstTokenList){tokenList.tokens, tokenList.count}, &tree);
	ccFreeTokenList(&tokenList);
	if(result != CC_SUCCESS)
	{
		goto end;
	}

	ccFreeTree(&tree);
	
	end:
	free(source.string);
	return result;
}
