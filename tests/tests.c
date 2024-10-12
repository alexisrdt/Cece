#include <stdlib.h>
#include <string.h>

#include "cece/cece.h"

static int ccCompareInts(const void* pFirstVoid, const void* pSecondVoid)
{
	const int first = *(const int*)pFirstVoid;
	const int second = *(const int*)pSecondVoid;

	return (first > second) - (first < second);
}

static void ccTestFind(int* pExitValue)
{
	const int array[] = {1, 2, 3, 4, 5, 10, INT_MAX};
	const size_t count = CC_LEN(array);

	size_t result;

	for(size_t index = 0; index < count; ++index)
	{
		result = ccFind(array, count, sizeof(array[0]), &array[index], ccCompareInts);
		if(result != index)
		{
			CC_LOG("ccFind returned %zu instead of %zu.", result, index);
			*pExitValue = EXIT_FAILURE;
			return;
		}
	}

	result = ccFind(array, count, sizeof(array[0]), &(int){8}, ccCompareInts);
	if(result != count)
	{
		CC_LOG("ccFind returned %zu instead of %zu.", result, count);
		*pExitValue = EXIT_FAILURE;
		return;
	}

	result = ccFind(array, count, sizeof(array[0]), &(int){INT_MIN}, ccCompareInts);
	if(result != count)
	{
		CC_LOG("ccFind returned %zu instead of %zu.", result, count);
		*pExitValue = EXIT_FAILURE;
		return;
	}
}

static CcString ccDynamicString(const char* string)
{
	const size_t length = strlen(string);
	const CcString dynamicString = {malloc(length + 1), length};
	if(!dynamicString.string)
	{
		CC_LOG("Failed to allocate memory.");
		return dynamicString;
	}
	strcpy(dynamicString.string, string);

	return dynamicString;
}

static void ccTestTrigraph(CcString* pString, const CcConstString* pSolution, const CcFilePosition* positionsSolution, size_t positionsSolutionCount, int* pExitValue)
{
	CcFilePosition* positions;
	size_t positionCount;
	if(ccTrigraphs(pString, 4, &positions, &positionCount) != CC_SUCCESS)
	{
		CC_LOG("ccTrigraphs failed.");
		*pExitValue = EXIT_FAILURE;
		return;
	}
	if(pString->length != pSolution->length || strlen(pString->string) != pString->length || strcmp(pString->string, pSolution->string) != 0)
	{
		CC_LOG("ccTrigraphs failed.");
		*pExitValue = EXIT_FAILURE;
		return;
	}

	if(positionCount != positionsSolutionCount)
	{
		CC_LOG("ccTrigraphs failed.");
		*pExitValue = EXIT_FAILURE;
		return;
	}
	for(size_t i = 0; i < positionCount; ++i)
	{
		if(positions[i].line != positionsSolution[i].line || positions[i].column != positionsSolution[i].column)
		{
			CC_LOG("ccTrigraphs failed.");
			*pExitValue = EXIT_FAILURE;
			return;
		}
	}

	CC_FREE(positions)
}

static void ccTestTrigraphs(int* pExitValue)
{
	CcString strings[] = {
		ccDynamicString(""),
		ccDynamicString("??"),
		ccDynamicString("?X<?X>"),
		ccDynamicString("This ?X= is a test.\nTab\t?X!"),
		ccDynamicString("?X<\n\t?X>\n \t?X(\n  \t?X)\n   \t?X=\n    \t?X/\n\t\t?X'\n\t \t?X!\n\t  \t?X-\n\t   \t?X<\n\t    \t?X>\n\t\t\t?X(")
	};
	const size_t testCount = CC_LEN(strings);
	for(size_t i = 0; i < testCount; ++i)
	{
		if(!strings[i].string)
		{
			CC_LOG("Failed to allocate memory.");
			*pExitValue = EXIT_FAILURE;
			return;
		}
	}
	strings[2].string[1] = '?';
	strings[2].string[4] = '?';
	strings[3].string[6] = '?';
	strings[3].string[25] = '?';
	strings[4].string[1] = '?';
	strings[4].string[6] = '?';
	strings[4].string[12] = '?';
	strings[4].string[19] = '?';
	strings[4].string[27] = '?';
	strings[4].string[36] = '?';
	strings[4].string[42] = '?';
	strings[4].string[49] = '?';
	strings[4].string[57] = '?';
	strings[4].string[66] = '?';
	strings[4].string[76] = '?';
	strings[4].string[83] = '?';

	const CcConstString solutions[] = {
		{"", 0},
		{"??", 2},
		{"{}", 2},
		{"This # is a test.\nTab\t|", 23},
		{"{\n\t}\n \t[\n  \t]\n   \t#\n    \t\\\n\t\t^\n\t \t|\n\t  \t~\n\t   \t{\n\t    \t}\n\t\t\t[", 61}
	};

	const CcFilePosition* const positionsSolutions[] = {
		NULL,
		NULL,
		(const CcFilePosition[]){{1, 1}, {1, 4}},
		(const CcFilePosition[]){{1, 6}, {2, 5}},
		(const CcFilePosition[]){{1, 1}, {2, 5}, {3, 5}, {4, 5}, {5, 5}, {6, 9}, {7, 9}, {8, 9}, {9, 9}, {10, 9}, {11, 13}, {12, 13}}
	};

	const size_t positionsSolutionCount[] = {
		0,
		0,
		2,
		2,
		12
	};

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		ccTestTrigraph(&strings[testIndex], &solutions[testIndex], positionsSolutions[testIndex], positionsSolutionCount[testIndex], pExitValue);
		CC_FREE(strings[testIndex].string)
	}
}

static void ccTestLineBreak(CcString* pString, const CcFilePosition* trigraphs, size_t trigraphCount, const CcConstString* pSolution, const CcFilePosition* positionsSolution, size_t positionsSolutionCount, int* pExitValue)
{
	CcFilePosition* positions;
	size_t positionCount;
	if(ccLineBreaks(pString, 4, trigraphs, trigraphCount, &positions, &positionCount) != CC_SUCCESS)
	{
		CC_LOG("ccLineBreaks failed.");
		*pExitValue = EXIT_FAILURE;
		return;
	}

	if(pString->length != pSolution->length || strlen(pString->string) != pString->length || strcmp(pString->string, pSolution->string) != 0)
	{
		CC_LOG("ccLineBreaks failed.");
		*pExitValue = EXIT_FAILURE;
		return;
	}

	if(positionCount != positionsSolutionCount)
	{
		CC_LOG("ccLineBreaks failed.");
		*pExitValue = EXIT_FAILURE;
		return;
	}
	for(size_t i = 0; i < positionCount; ++i)
	{
		if(positions[i].line != positionsSolution[i].line || positions[i].column != positionsSolution[i].column)
		{
			CC_LOG("ccLineBreaks failed.");
			*pExitValue = EXIT_FAILURE;
			return;
		}
	}

	CC_FREE(positions)
}

static void ccTestLineBreaks(int* pExitValue)
{
	CcString strings[] = {
		ccDynamicString(""),
		ccDynamicString("\n"),
		ccDynamicString("This \\\nis a\\\n test.")
	};
	const size_t testCount = CC_LEN(strings);
	for(size_t i = 0; i < testCount; ++i)
	{
		if(!strings[i].string)
		{
			CC_LOG("Failed to allocate memory.");
			*pExitValue = EXIT_FAILURE;
			return;
		}
	}

	const CcConstString solutions[] = {
		{"\n", 1},
		{"\n", 1},
		{"This is a test.\n", 16}
	};

	const CcFilePosition* const trigraphs[] = {
		NULL,
		NULL,
		(const CcFilePosition[]){{1, 6}}
	};

	const size_t trigraphCounts[] = {
		0,
		0,
		1
	};

	const CcFilePosition* const positionsSolutions[] = {
		NULL,
		NULL,
		(const CcFilePosition[]){{1, 6}, {2, 5}}
	};

	const size_t positionsSolutionCounts[] = {
		0,
		0,
		2
	};

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		ccTestLineBreak(&strings[testIndex], trigraphs[testIndex], trigraphCounts[testIndex], &solutions[testIndex], positionsSolutions[testIndex], positionsSolutionCounts[testIndex], pExitValue);
		CC_FREE(strings[testIndex].string)
	}
}

int main(void)
{
	int exitValue = EXIT_SUCCESS;

	ccTestFind(&exitValue);

	ccTestTrigraphs(&exitValue);

	ccTestLineBreaks(&exitValue);

	return exitValue;
}
