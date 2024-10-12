#include "cece/pre.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cece/log.h"
#include "cece/memory.h"

CcResult ccTrigraphs(CcString* pString, size_t tabWidth, CcFilePosition** pPositions, size_t* pPositionCount)
{
	*pPositionCount = 0;

	if(pString->length < 3)
	{
		*pPositions = NULL;
		return CC_SUCCESS;
	}

	*pPositions = malloc(pString->length / 3 * sizeof((*pPositions)[0]));
	if(!*pPositions)
	{
		CC_LOG("Failed to allocate memory.");
		return CC_ERROR_OUT_OF_MEMORY;
	}

	const char trigraphs[] = {
		'<', '{',
		'>', '}',
		'(', '[',
		')', ']',
		'=', '#',
		'/', '\\',
		'\'', '^',
		'!', '|',
		'-', '~'
	};
	const size_t trigraphCount = CC_LEN(trigraphs) / 2;

	size_t line = 1;
	size_t column = 1;
	for(size_t i = 0; pString->length >= 3 && i < pString->length - 2; ++i, ++column)
	{
		if(pString->string[i] == '\n')
		{
			++line;
			column = 0;

			continue;
		}

		if(pString->string[i] == '\t')
		{
			column += tabWidth - ((column - 1) % tabWidth) - 1;

			continue;
		}

		if(pString->string[i] == '?' && pString->string[i + 1] == '?')
		{
			for(size_t j = 0; j < trigraphCount; ++j)
			{
				if(pString->string[i + 2] == trigraphs[j * 2])
				{
					(*pPositions)[*pPositionCount].line = line;
					(*pPositions)[*pPositionCount].column = column;

					++*pPositionCount;

					pString->string[i] = trigraphs[j * 2 + 1];
					memmove(pString->string + i + 1, pString->string + i + 3, pString->length - i - 2);
					pString->length -= 2;

					column += 2;

					break;
				}
			}
		}
	}

	CcFilePosition* const newPositions = realloc(*pPositions, *pPositionCount * sizeof((*pPositions)[0]));
	if(!newPositions)
	{
		CC_LOG("Failed to allocate memory.");
		CC_FREE(*pPositions)
		return CC_ERROR_OUT_OF_MEMORY;
	}
	*pPositions = newPositions;

	char* const newString = realloc(pString->string, pString->length + 1);
	if(!newString)
	{
		CC_LOG("Failed to allocate memory.");
		CC_FREE(*pPositions)
		return CC_ERROR_OUT_OF_MEMORY;
	}
	pString->string = newString;

	return CC_SUCCESS;
}

CcResult ccLineBreaks(CcString* pString, size_t tabWidth, const CcFilePosition* trigraphs, size_t trigraphCount, CcFilePosition** pPositions, size_t* pPositionCount)
{
	*pPositionCount = 0;

	if(pString->length < 2)
	{
		*pPositions = NULL;

		if(pString->length == 0 || pString->string[pString->length - 1] != '\n')
		{
			fprintf(stderr, "1:1: Missing newline at end of file.\n");

			pString->length += 1;
			char* const newString = realloc(pString->string, pString->length + 1);
			if(!newString)
			{
				CC_LOG("Failed to allocate memory.");
				return CC_ERROR_OUT_OF_MEMORY;
			}
			pString->string = newString;

			pString->string[pString->length - 1] = '\n';
			pString->string[pString->length] = '\0';
		}

		return CC_SUCCESS;
	}

	*pPositions = malloc(pString->length / 2 * sizeof((*pPositions)[0]));
	if(!*pPositions)
	{
		CC_LOG("Failed to allocate memory.");
		return CC_ERROR_OUT_OF_MEMORY;
	}

	size_t line = 1;
	size_t column = 1;
	size_t trigraphIndex = 0;
	for(size_t i = 0; pString->length >= 2 && i <= pString->length - 1; ++i, ++column)
	{
		if(pString->string[i] == '\n')
		{
			++line;
			column = 0;
		}

		if(pString->string[i] == '\t')
		{
			column += tabWidth - ((column - 1) % tabWidth) - 1;
		}

		if(pString->string[i] == '\\' && pString->string[i + 1] == '\n')
		{
			(*pPositions)[*pPositionCount].line = line;
			(*pPositions)[*pPositionCount].column = column;

			++*pPositionCount;

			memmove(pString->string + i, pString->string + i + 2, pString->length - i - 1);
			pString->length -= 2;

			--i;


			if(trigraphIndex < trigraphCount && trigraphs[trigraphIndex].line == line && trigraphs[trigraphIndex].column == column)
			{
				++trigraphIndex;
			}

			++line;
			column = 0;
		}

		if(trigraphIndex < trigraphCount && trigraphs[trigraphIndex].line <= line && trigraphs[trigraphIndex].column <= column)
		{
			column += 2;
			++trigraphIndex;
		}
	}

	if(*pPositionCount == 0)
	{
		CC_FREE(*pPositions)
		*pPositions = NULL;
	}
	else
	{
		CcFilePosition* const newPositions = realloc(*pPositions, *pPositionCount * sizeof((*pPositions)[0]));
		if(!newPositions)
		{
			CC_LOG("Failed to allocate memory.");
			CC_FREE(*pPositions)
			return CC_ERROR_OUT_OF_MEMORY;
		}
		*pPositions = newPositions;
	}

	const bool missingNewline = pString->string[pString->length - 1] != '\n';
	pString->length += missingNewline;

	char* const newString = realloc(pString->string, pString->length + 1);
	if(!newString)
	{
		CC_LOG("Failed to allocate memory.");
		CC_FREE(*pPositions)
		return CC_ERROR_OUT_OF_MEMORY;
	}
	pString->string = newString;

	if(missingNewline)
	{
		pString->string[pString->length - 1] = '\n';
		pString->string[pString->length] = '\0';

		fprintf(stderr, "%zu:%zu: Missing newline at end of file.\n", line, column - 1);
	}

	return CC_SUCCESS;
}
