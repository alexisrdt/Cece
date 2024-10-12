#ifndef CECE_PRE_H
#define CECE_PRE_H

#include <stddef.h>

#include "cece/result.h"

/*
 * A string.
 *
 * Fields:
 * - string: A string. Cece always makes it length + 1-wide and null-terminated.
 * - length: The length of the string.
 */
typedef struct CcString
{
	char* string;
	size_t length;
} CcString;

/*
 * A file position.
 *
 * Fields:
 * - line: The line number.
 * - column: The column number.
 */
typedef struct CcFilePosition
{
	size_t line;
	size_t column;
} CcFilePosition;

CcResult ccTrigraphs(CcString* pString, size_t tabWidth, CcFilePosition** pPositions, size_t* pPositionCount);

CcResult ccLineBreaks(CcString* pString, size_t tabWidth, const CcFilePosition* trigraphs, size_t trigraphCount, CcFilePosition** pPositions, size_t* pPositionCount);

#endif
