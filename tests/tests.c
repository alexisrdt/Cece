#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cece/cece.h"

#define CC_FAIL(...) \
fprintf(stderr, "Function %s, line %d: ", __func__, __LINE__); \
fprintf(stderr, __VA_ARGS__); \
fprintf(stderr, "\n"); \
*pPassed = false

static bool ccCompareConstants(const CcConstant first, const CcConstant second)
{
	assert(first.type >= CC_CONSTANT_INT && first.type <= CC_CONSTANT_UNSIGNED_LONG_LONG);
	assert(second.type >= CC_CONSTANT_INT && second.type <= CC_CONSTANT_UNSIGNED_LONG_LONG);

	return first.type == second.type && first.value == second.value;
}

static int ccCompareInts(const void* const pFirstVoid, const void* const pSecondVoid)
{
	assert(pFirstVoid != nullptr);
	assert(pSecondVoid != nullptr);

	const int first = *(const int*)pFirstVoid;
	const int second = *(const int*)pSecondVoid;

	return (first > second) - (first < second);
}

static void ccTestFind(bool* const pPassed)
{
	assert(pPassed != nullptr);

	constexpr int array[] = {1, 2, 3, 4, 5, 10, INT_MAX};
	constexpr size_t elementCount = CC_LEN(array);
	constexpr size_t elementSize = sizeof(array[0]);

	const int* result;

	for(size_t index = 0; index < elementCount; ++index)
	{
		result = ccFind(&array[index], array, elementCount, elementSize, ccCompareInts);
		if(result != &array[index])
		{
			CC_FAIL("ccFind returned %p instead of %p.", (const void*)result, (const void*)&array[index]);
		}
	}

	result = ccFind(&(const int){8}, array, elementCount, elementSize, ccCompareInts);
	if(result != nullptr)
	{
		CC_FAIL("ccFind returned %p instead of %p.", (const void*)result, (const void*)nullptr);
	}

	result = ccFind(&(const int){INT_MIN}, array, elementCount, elementSize, ccCompareInts);
	if(result != nullptr)
	{
		CC_FAIL("ccFind returned %p instead of %p.", (const void*)result, (const void*)nullptr);
	}

	result = ccFind(&array[0], array, 0, elementSize, ccCompareInts);
	if(result != nullptr)
	{
		CC_FAIL("ccFind returned %p instead of %p.", (const void*)result, (const void*)nullptr);
	}
}

static void ccTestArguments(bool* const pPassed)
{
	CcOptions options;

	const char* const args1[] = {"-std=c"};
	if(ccParseArguments(CC_LEN(args1), args1, &options) != CC_ERROR_INVALID_ARGUMENT)
	{
		*pPassed = false;
		return;
	}
	if(options.input || options.output)
	{
		*pPassed = false;
		return;
	}

	const char* const args2[] = {"test.c"};
	if(ccParseArguments(CC_LEN(args2), args2, &options) != CC_SUCCESS)
	{
		*pPassed = false;
		return;
	}

	if(!options.input || !options.output || options.debug || options.version != CC_C23)
	{
		*pPassed = false;
		return;
	}

	if(strcmp(options.input, args2[0]) != 0 || strcmp(options.output, "test.s") != 0)
	{
		*pPassed = false;
		return;
	}

	free(options.input);
	free(options.output);

	const char* const args3[] = {
		"test.c",
		"-o",
		"output.s",
		"-std=c23"
	};
	if(ccParseArguments(CC_LEN(args3), args3, &options) != CC_SUCCESS)
	{
		*pPassed = false;
		return;
	}

	if(strcmp(options.input, args3[0]) != 0 || strcmp(options.output, "output.s") != 0 || options.version != CC_C23)
	{
		*pPassed = false;
	}

	free(options.input);
	free(options.output);
}

static void ccTestStrings(bool* const pPassed)
{
	assert(pPassed != nullptr);

	const struct
	{
		const char* string;
		bool isString;
		size_t length;
	} tests[] = {
		{.string = "", .isString = false},
		{.string = "abc", .isString = false},
		{.string = "123", .isString = false},
		{.string = "+=", .isString = false},
		{.string = "\"string\"", .isString = true, .length = 8}
	};
	constexpr size_t testCount = CC_LEN(tests);

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		CcToken token;
		const bool result = ccParseString(tests[testIndex].string, &token);

		if(result != tests[testIndex].isString)
		{
			CC_FAIL("Test string #%zu does not match.", testIndex);
			continue;
		}

		if(!result)
		{
			continue;
		}

		if(token.string.string != tests[testIndex].string)
		{
			CC_FAIL("Test string #%zu wrong string.", testIndex);
		}

		if(token.string.length != tests[testIndex].length)
		{
			CC_FAIL("Test string #%zu wrong length.", testIndex);
		}
	}
}

static void ccTestCharacters(bool* const pPassed)
{
	assert(pPassed != nullptr);

	const struct
	{
		const char* string;
		bool isCharacter;
		int value;
		size_t length;
	} tests[] = {
		{.string = "", .isCharacter = false},
		{.string = "abc", .isCharacter = false},
		{.string = "123", .isCharacter = false},
		{.string = "+=", .isCharacter = false},
		{.string = "\"string\"", .isCharacter = false},
		{.string = "'c'", .isCharacter = true, .value = 'c', .length = 3},
		{.string = "'\\''", .isCharacter = true, .value = '\'', .length = 4},
	};
	constexpr size_t testCount = CC_LEN(tests);

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		CcToken token;
		const bool result = ccParseCharacter(tests[testIndex].string, &token);

		if(result != tests[testIndex].isCharacter)
		{
			CC_FAIL("Test character #%zu does not match.", testIndex);
			continue;
		}

		if(!result)
		{
			continue;
		}

		if(token.string.string != tests[testIndex].string)
		{
			CC_FAIL("Test character #%zu wrong string.", testIndex);
		}

		if(token.string.length != tests[testIndex].length)
		{
			CC_FAIL("Test character #%zu wrong length.", testIndex);
		}

		if(!ccCompareConstants(token.constant, (CcConstant){CC_CONSTANT_INT, tests[testIndex].value}))
		{
			CC_FAIL("Test character #%zu wrong constant.", testIndex);
		}
	}
}

static void ccTestTokens(bool* const pPassed)
{
	assert(pPassed != nullptr);

	const struct
	{
		const char* string;
		bool isToken;
		CcTokenType type;
		size_t length;
	} tests[] = {
		{.string = "", .isToken = false},
		{.string = "+=", .isToken = true, .type = CC_TOKEN_PLUS_EQUAL, .length = 2},
		{.string = "+++", .isToken = true, .type = CC_TOKEN_PLUS_PLUS, .length = 2},
		{.string = "+-", .isToken = true, .type = CC_TOKEN_PLUS, .length = 1},
		{.string = "[{()}]", .isToken = true, .type = CC_TOKEN_OPEN_BRACKET, .length = 1},
		{.string = "abc", .isToken = false},
		{.string = "123", .isToken = false},
		{.string = "\"string\"", .isToken = false},
		{.string = "'c'", .isToken = false}
	};
	constexpr size_t testCount = CC_LEN(tests);

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		CcToken token;
		const bool result = ccParseToken(tests[testIndex].string, &token);

		if(result != tests[testIndex].isToken)
		{
			CC_FAIL("Test token #%zu does not match.", testIndex);
			continue;
		}

		if(!result)
		{
			continue;
		}

		if(token.type != tests[testIndex].type)
		{
			CC_FAIL("Test token #%zu wrong type.", testIndex);
		}

		if(token.string.length != tests[testIndex].length)
		{
			CC_FAIL("Test token #%zu wrong length.", testIndex);
		}
	}
}

static void ccTestConstants(bool* const pPassed)
{
	assert(pPassed != nullptr);

	const struct
	{
		const char* string;
		bool isConstant;
		CcConstant constant;
		size_t length;
	} tests[] = {
		{.string = "", .isConstant = false},
		{.string = "abc", .isConstant = false},
		{.string = "[]", .isConstant = false},
		{.string = "57", .isConstant = true, .constant = {.type = CC_CONSTANT_INT, .value = 57}, .length = 2},
		{.string = "57llu", .isConstant = true, .constant = {.type = CC_CONSTANT_UNSIGNED_LONG_LONG, .value = 57}, .length = 5},
		{.string = "0x42L", .isConstant = true, .constant = {.type = CC_CONSTANT_LONG, .value = 0x42}, .length = 5},
		{.string = "2147483648", .isConstant = true, .constant = {.type = CC_CONSTANT_LONG_LONG, .value = (long long)INT_MAX + 1}, .length = 10},
		{.string = "0 a", .isConstant = true, .constant = {.type = CC_CONSTANT_INT, .value = 0}, .length = 1},
		{.string = " 1", .isConstant = false}
	};
	const size_t testCount = CC_LEN(tests);

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		CcToken token;
		const bool result = ccParseConstant(tests[testIndex].string, &token);

		if(result != tests[testIndex].isConstant)
		{
			CC_FAIL("Test constant #%zu does not match.", testIndex);
			continue;
		}

		if(!result)
		{
			continue;
		}

		if(token.string.string != tests[testIndex].string)
		{
			CC_FAIL("Test constant #%zu wrong string.", testIndex);
			continue;
		}

		if(token.string.length != tests[testIndex].length)
		{
			CC_FAIL("Test constant #%zu wrong length.", testIndex);
			continue;
		}

		if(!ccCompareConstants(token.constant, tests[testIndex].constant))
		{
			CC_FAIL("Test constant #%zu wrong constant.", testIndex);
		}
	}
}

static void ccTestIdentifiers(bool* const pPassed)
{
	assert(pPassed != nullptr);

	const struct
	{
		const char* string;
		bool isIdentifier;
		CcTokenType type;
		size_t length;
	} tests[] = {
		{.string = "", .isIdentifier = false},
		{.string = "int", .isIdentifier = true, .type = CC_TOKEN_INT},
		{.string = "int5a", .isIdentifier = true, .type = CC_TOKEN_IDENTIFIER, .length = 5},
		{.string = "int_t", .isIdentifier = true, .type = CC_TOKEN_IDENTIFIER, .length = 5},
		{.string = " int", .isIdentifier = false},
		{.string = "7a84de", .isIdentifier = false},
		{.string = "_my_var_", .isIdentifier = true, .type = CC_TOKEN_IDENTIFIER, .length = 8},
		{.string = "[float]", .isIdentifier = false},
		{.string = "fl;oat", .isIdentifier = true, .type = CC_TOKEN_IDENTIFIER, .length = 2},
		{.string = "float{}", .isIdentifier = true, .type = CC_TOKEN_FLOAT},
		{.string = "do int", .isIdentifier = true, .type = CC_TOKEN_DO},
		{.string = "double", .isIdentifier = true, .type = CC_TOKEN_DOUBLE}
	};
	const size_t testCount = CC_LEN(tests);

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		CcToken token;
		const bool result = ccParseIdentifier(tests[testIndex].string, &token);

		if(result != tests[testIndex].isIdentifier)
		{
			CC_FAIL("Test identifier #%zu does not match.", testIndex);
			continue;
		}

		if(!result)
		{
			continue;
		}

		if(token.string.string != tests[testIndex].string)
		{
			CC_FAIL("Test identifier #%zu wrong string.", testIndex);
			continue;
		}

		if(token.type != tests[testIndex].type)
		{
			CC_FAIL("Test identifier #%zu wrong type.", testIndex);
			continue;
		}

		if(token.type != CC_TOKEN_IDENTIFIER)
		{
			continue;
		}

		if(token.string.length != tests[testIndex].length)
		{
			CC_FAIL("Test identifier #%zu wrong length.", testIndex);
			continue;
		}
	}
}

static void ccTestLex(bool* const pPassed)
{
	assert(pPassed != nullptr);

	const struct
	{
		const char* string;
		size_t tokenCount;
		CcToken tokens[10];
	} tests[] = {
		{.string = "", .tokenCount = 0},
		{.string = "a + b", .tokenCount = 3, .tokens = {
			{.type = CC_TOKEN_IDENTIFIER},
			{.type = CC_TOKEN_PLUS},
			{.type = CC_TOKEN_IDENTIFIER}
		}},
		{.string = "i-*=p[\"s\"'c'5ul/x", .tokenCount = 10, .tokens = {
			{.type = CC_TOKEN_IDENTIFIER},
			{.type = CC_TOKEN_MINUS},
			{.type = CC_TOKEN_STAR_EQUAL},
			{.type = CC_TOKEN_IDENTIFIER},
			{.type = CC_TOKEN_OPEN_BRACKET},
			{.type = CC_TOKEN_STRING},
			{.type = CC_TOKEN_CONSTANT},
			{.type = CC_TOKEN_CONSTANT},
			{.type = CC_TOKEN_SLASH},
			{.type = CC_TOKEN_IDENTIFIER}
		}}
	};
	constexpr size_t testCount = CC_LEN(tests);

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		CcTokenList tokenList;
		const CcResult result = ccLex((CcConstString){.string = tests[testIndex].string, .length = strlen(tests[testIndex].string)}, &tokenList);

		if(result != CC_SUCCESS)
		{
			CC_FAIL("Test lex #%zu failed.", testIndex);
			continue;
		}

		if(tokenList.count != tests[testIndex].tokenCount)
		{
			CC_FAIL("Test lex #%zu wrong token count.", testIndex);
			free(tokenList.tokens);
			continue;
		}

		for(size_t tokenIndex = 0; tokenIndex < tokenList.count; ++tokenIndex)
		{
			if(tokenList.tokens[tokenIndex].type != tests[testIndex].tokens[tokenIndex].type)
			{
				CC_FAIL("Test lex #%zu wrong #%zu token.", testIndex, tokenIndex);
			}
		}

		free(tokenList.tokens);
	}
}

static void ccTestParentheses(bool* const pPassed)
{
	const struct
	{
		const CcToken* tokens;
		size_t count;
		size_t start;
		CcDirection direction;
		bool result;
		size_t end;
	} tests[] = {
		{(const CcToken[]){{.type = CC_TOKEN_PLUS}}, 1, 0, CC_DIRECTION_FORWARD, true, 0},
		{(const CcToken[]){{.type = CC_TOKEN_OPEN_PARENTHESIS}}, 1, 0, CC_DIRECTION_FORWARD, false, 0},
		{(const CcToken[]){{.type = CC_TOKEN_CLOSE_PARENTHESIS}}, 1, 0, CC_DIRECTION_BACKWARD, false, 0},
		{(const CcToken[]){{.type = CC_TOKEN_OPEN_PARENTHESIS}, {.type = CC_TOKEN_CLOSE_PARENTHESIS}}, 2, 0, CC_DIRECTION_FORWARD, true, 1},
		{(const CcToken[]){{.type = CC_TOKEN_OPEN_PARENTHESIS}, {.type = CC_TOKEN_CLOSE_PARENTHESIS}}, 2, 1, CC_DIRECTION_BACKWARD, true, 0},
		{(const CcToken[]){{.type = CC_TOKEN_CLOSE_PARENTHESIS}, {.type = CC_TOKEN_OPEN_PARENTHESIS}, {.type = CC_TOKEN_CLOSE_PARENTHESIS}}, 3, 0, CC_DIRECTION_FORWARD, false, 0},
		{(const CcToken[]){{.type = CC_TOKEN_OPEN_PARENTHESIS}, {.type = CC_TOKEN_CLOSE_PARENTHESIS}, {.type = CC_TOKEN_OPEN_PARENTHESIS}}, 3, 2, CC_DIRECTION_BACKWARD, false, 2},
		{(const CcToken[]){{.type = CC_TOKEN_OPEN_PARENTHESIS}, {.type = CC_TOKEN_PLUS}, {.type = CC_TOKEN_OPEN_PARENTHESIS},  {.type = CC_TOKEN_PLUS}, {.type = CC_TOKEN_CLOSE_PARENTHESIS}, {.type = CC_TOKEN_PLUS}, {.type = CC_TOKEN_CLOSE_PARENTHESIS}, {.type = CC_TOKEN_PLUS}}, 8, 0, CC_DIRECTION_FORWARD, true, 6},
		{(const CcToken[]){{.type = CC_TOKEN_PLUS}, {.type = CC_TOKEN_OPEN_PARENTHESIS}, {.type = CC_TOKEN_PLUS}, {.type = CC_TOKEN_OPEN_PARENTHESIS},  {.type = CC_TOKEN_PLUS}, {.type = CC_TOKEN_CLOSE_PARENTHESIS}, {.type = CC_TOKEN_PLUS}, {.type = CC_TOKEN_CLOSE_PARENTHESIS}}, 8, 7, CC_DIRECTION_BACKWARD, true, 1},
	};
	constexpr size_t testCount = CC_LEN(tests);

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		size_t tokenIndex = tests[testIndex].start;
		const bool result = ccSkipParentheses(&tokenIndex, &(const CcConstTokenList){tests[testIndex].tokens, tests[testIndex].count}, tests[testIndex].direction);

		if(result != tests[testIndex].result)
		{
			CC_FAIL("Test parentheses #%zu: wrong result.", testIndex);
			continue;
		}

		if(tokenIndex != tests[testIndex].end)
		{
			CC_FAIL("Test parentheses #%zu: wrong end pointer.", testIndex);
		}
	}
}

static void ccTestExpressions(bool* const pPassed)
{
	assert(pPassed != nullptr);

	const struct
	{
		const CcToken* tokens;
		size_t count;
		bool result;
		const CcNode* solution;
		size_t solutionCount;
	} tests[] = {
		{(const CcToken[]){
			{.type = CC_TOKEN_PLUS},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 5}}
		}, 2, false, nullptr, 0},
		{(const CcToken[]){
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 5}},
			{.type = CC_TOKEN_PLUS}
		}, 2, false, nullptr, 0},
		{(const CcToken[]){
			{.type = CC_TOKEN_OPEN_PARENTHESIS},
			{.type = CC_TOKEN_OPEN_PARENTHESIS},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 5}},
			{.type = CC_TOKEN_STAR}, {.type = CC_TOKEN_OPEN_PARENTHESIS},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 26}},
			{.type = CC_TOKEN_PLUS},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 27}},
			{.type = CC_TOKEN_STAR},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 28}},
			{.type = CC_TOKEN_PLUS},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 29}},
			{.type = CC_TOKEN_CLOSE_PARENTHESIS},
			{.type = CC_TOKEN_PLUS},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 37}},
			{.type = CC_TOKEN_CLOSE_PARENTHESIS},
			{.type = CC_TOKEN_CLOSE_PARENTHESIS}
		}, 17, true, (const CcNode[]){
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 5}},
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 26}},
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 27}},
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 28}},
			{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_MUL, 2, 3}},
			{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_SUM, 1, 4}},
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 29}},
			{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_SUM, 5, 6}},
			{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_MUL, 0, 7}},
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 37}},
			{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_SUM, 8, 9}}
		}, 11},
		{
			(const CcToken[]){
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 0}},
				{.type = CC_TOKEN_AMPERSAND},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 1}},
				{.type = CC_TOKEN_LEFT_SHIFT},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 2}},
				{.type = CC_TOKEN_EQUAL_EQUAL},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 3}},
				{.type = CC_TOKEN_PLUS},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 4}},
				{.type = CC_TOKEN_STAR},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 5}},
				{.type = CC_TOKEN_BAR_BAR},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 6}},
				{.type = CC_TOKEN_CARET},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 7}},
				{.type = CC_TOKEN_LESS},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 8}},
				{.type = CC_TOKEN_RIGHT_SHIFT},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 9}},
				{.type = CC_TOKEN_SLASH},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 10}},
				{.type = CC_TOKEN_AMPERSAND_AMPERSAND},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 11}},
				{.type = CC_TOKEN_MINUS},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 12}},
				{.type = CC_TOKEN_GREATER_EQUAL},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 13}},
				{.type = CC_TOKEN_NOT_EQUAL},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 14}},
				{.type = CC_TOKEN_LESS_EQUAL},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 15}},
				{.type = CC_TOKEN_BAR},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 16}},
				{.type = CC_TOKEN_GREATER},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 17}},
				{.type = CC_TOKEN_PERCENT},
				{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 18}}
			}, 37, true, (const CcNode[])
			{
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 0}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 1}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 2}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_LS, 1, 2}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 3}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 4}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 5}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_MUL, 5, 6}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_SUM, 4, 7}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_EQ, 3, 8}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_AND, 0, 9}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 6}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 7}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 8}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 9}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 10}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_DIV, 14, 15}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_RS, 13, 16}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_LE, 12, 17}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_XOR, 11, 18}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 11}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 12}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_DIF, 20, 21}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 13}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_GEQ, 22, 23}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 14}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 15}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_LEQ, 25, 26}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_NEQ, 24, 27}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 16}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 17}},
				{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 18}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_MOD, 30, 31}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_GE, 29, 32}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_OR, 28, 33}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_LAND, 19, 34}},
				{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_LOR, 10, 35}}
			}, 37}
	};
	constexpr size_t testCount = CC_LEN(tests);

	CcNode nodes[64];
	size_t children[1];

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		CcTree tree = {.nodes = nodes, .children = children};
		CcConstTokenList tokens = {tests[testIndex].tokens, tests[testIndex].count};

		const bool result = ccParseExpression(&(CcTreeBuilder){.pTree = &tree, .tokens = &tokens});
		if(result != tests[testIndex].result)
		{
			CC_FAIL("Parse expression #%zu: wrong result.", testIndex);
			continue;
		}

		if(!result)
		{
			continue;
		}

		if(tree.count != tests[testIndex].solutionCount)
		{
			CC_FAIL("Parse expression #%zu: wrong count.", testIndex);
			return;
		}

		for(size_t nodeIndex = 0; nodeIndex < tree.count; ++nodeIndex)
		{
			if(tree.nodes[nodeIndex].type != tests[testIndex].solution[nodeIndex].type)
			{
				CC_FAIL("Parse expression #%zu: node #%zu: wrong type.", testIndex, nodeIndex);
				continue;
			}

			switch(tree.nodes[nodeIndex].type)
			{
				case CC_NODE_CONSTANT:
					if(!ccCompareConstants(tree.nodes[nodeIndex].constant, tests[testIndex].solution[nodeIndex].constant))
					{
						CC_FAIL("Parse expression #%zu: node #%zu: wrong constant.", testIndex, nodeIndex);
						break;
					}
					break;

				case CC_NODE_BIN_OP:
					if(tree.nodes[nodeIndex].binOpNode.op != tests[testIndex].solution[nodeIndex].binOpNode.op)
					{
						CC_FAIL("Parse expression #%zu: node #%zu: wrong binop type.", testIndex, nodeIndex);
						break;
					}

					if(tree.nodes[nodeIndex].binOpNode.leftNode != tests[testIndex].solution[nodeIndex].binOpNode.leftNode)
					{
						CC_FAIL("Parse expression #%zu: node #%zu: wrong binop left node", testIndex, nodeIndex);
					}

					if(tree.nodes[nodeIndex].binOpNode.rightNode != tests[testIndex].solution[nodeIndex].binOpNode.rightNode)
					{
						CC_FAIL("Parse expression #%zu: node #%zu: wrong binop right node", testIndex, nodeIndex);
					}
					break;

				default:
					CC_FAIL("Parse expression #%zu: node #%zu: unexpected node type.", testIndex, nodeIndex);
					break;
			}
		}
	}
}

static void ccTestStatements(bool* const pPassed)
{
	assert(pPassed != nullptr);

	const struct
	{
		const CcToken* tokens;
		size_t count;
		bool result;
		const CcNode* solution;
		size_t solutionCount;
	} tests[] = {
		{(const CcToken[]){
			{.type = CC_TOKEN_RETURN}
		}, 1, false,nullptr, 0},
		{(const CcToken[]){
			{.type = CC_TOKEN_SEMICOLON}
		}, 1, false,nullptr, 0},
		{(const CcToken[]){
			{.type = CC_TOKEN_RETURN},
			{.type = CC_TOKEN_SEMICOLON}
		}, 2, true, (const CcNode[]){
			{.type = CC_NODE_RETURN, .returnNode = SIZE_MAX}
		}, 1},
		{(const CcToken[]){
			{.type = CC_TOKEN_RETURN},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 1}},
			{.type = CC_TOKEN_PLUS},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 2}},
			{.type = CC_TOKEN_SEMICOLON},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 3}}
		}, 6, true, (const CcNode[]){
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 1}},
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 2}},
			{.type = CC_NODE_BIN_OP, .binOpNode = {CC_BIN_OP_SUM, 0, 1}},
			{.type = CC_NODE_RETURN, .returnNode = 2}
		}, 4}
	};
	constexpr size_t testCount = CC_LEN(tests);

	CcNode nodes[64];
	size_t children[1];

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		CcTree tree = {.nodes = nodes, .children = children};
		CcConstTokenList tokens = {.tokens = tests[testIndex].tokens, .count = tests[testIndex].count};

		const bool result = ccParseStatement(&(CcTreeBuilder){.pTree = &tree, .tokens = &tokens});
		if(result != tests[testIndex].result)
		{
			CC_FAIL("Parse statement #%zu: wrong result.", testIndex);
			continue;
		}

		if(!result)
		{
			continue;
		}

		if(tree.count != tests[testIndex].solutionCount)
		{
			CC_FAIL("Parse statement #%zu: wrong node count.", testIndex);
			continue;
		}

		for(size_t nodeIndex = 0; nodeIndex < tree.count; ++nodeIndex)
		{
			if(tree.nodes[nodeIndex].type != tests[testIndex].solution[nodeIndex].type)
			{
				CC_FAIL("Parse statement #%zu: node #%zu: wrong type.", testIndex, nodeIndex);
				continue;
			}

			if(tree.nodes[nodeIndex].type == CC_NODE_RETURN)
			{
				if(tree.nodes[nodeIndex].returnNode != tests[testIndex].solution[nodeIndex].returnNode)
				{
					CC_FAIL("Parse statement #%zu: node #%zu: wrong value node.", testIndex, nodeIndex);
				}
			}
		}
	}
}

static void ccTestFunctions(bool* const pPassed)
{
	assert(pPassed != nullptr);

	const struct
	{
		const CcToken* tokens;
		size_t count;
		bool result;
		CcNode* solution;
		size_t solutionCount;
		const size_t* childrenSolution;
		size_t childrenSolutionCount;
	} tests[] = {
		{(const CcToken[]){
			{.type = CC_TOKEN_INT},
			{.type = CC_TOKEN_IDENTIFIER, .string = {"some_func", 9}},
			{.type = CC_TOKEN_OPEN_PARENTHESIS},
			{.type = CC_TOKEN_VOID},
			{.type = CC_TOKEN_CLOSE_PARENTHESIS},
			{.type = CC_TOKEN_OPEN_BRACE},
			{.type = CC_TOKEN_RETURN},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 1}},
			{.type = CC_TOKEN_PLUS},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 2}},
			{.type = CC_TOKEN_SEMICOLON},
			{.type = CC_TOKEN_RETURN},
			{.type = CC_TOKEN_SEMICOLON},
			{.type = CC_TOKEN_RETURN},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 0}},
			{.type = CC_TOKEN_SEMICOLON},
			{.type = CC_TOKEN_CLOSE_BRACE}
		}, 17, true, (CcNode[]){
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 1}},
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 2}},
			{.type = CC_NODE_BIN_OP, .binOpNode = {.op = CC_BIN_OP_SUM}},
			{.type = CC_NODE_RETURN},
			{.type = CC_NODE_RETURN, .returnNode = SIZE_MAX},
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 0}},
			{.type = CC_NODE_RETURN},
			{.type = CC_NODE_FUNCTION, .function = {.statementsStart = 2, .statementsCount = 3}}
		}, 8, (const size_t[]){
			6,
			4,
			3
		}, 3},
		{(const CcToken[]){
			{.type = CC_TOKEN_INT},
			{.type = CC_TOKEN_IDENTIFIER, .string = {"some_other_func", 15}},
			{.type = CC_TOKEN_OPEN_PARENTHESIS},
			{.type = CC_TOKEN_CLOSE_PARENTHESIS},
			{.type = CC_TOKEN_OPEN_BRACE},
			{.type = CC_TOKEN_CLOSE_BRACE}
		}, 6, true, (CcNode[]){
			{.type = CC_NODE_FUNCTION, .function = {.statementsCount = 0}}
		}, 1, nullptr, 0}
	};
	constexpr size_t testCount = CC_LEN(tests);

	tests[0].solution[7].function.name = tests[0].tokens[1].string;
	tests[1].solution[0].function.name = tests[1].tokens[1].string;

	CcNode nodes[64];
	size_t children[64];

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		CcTree tree = {.nodes = nodes, .children = children};
		CcConstTokenList tokens = {tests[testIndex].tokens, tests[testIndex].count};

		CcTreeBuilder builder = {.pTree = &tree, .tokens = &tokens, .lastIndex = CC_LEN(children)};

		const bool result = ccParseFunction(&builder);
		if(result != tests[testIndex].result)
		{
			CC_FAIL("Parse function #%zu: wrong result.", testIndex);
			continue;
		}

		if(!result)
		{
			continue;
		}

		if(tree.count != tests[testIndex].solutionCount)
		{
			CC_FAIL("Parse function #%zu: wrong count.", testIndex);
			continue;
		}

		if(builder.childCount != tests[testIndex].childrenSolutionCount)
		{
			CC_FAIL("Parse function #%zu: wrong child count.", testIndex);
			continue;
		}

		for(size_t childIndex = 0; childIndex < builder.childCount; ++childIndex)
		{
			if(tree.children[childIndex] != tests[testIndex].childrenSolution[childIndex])
			{
				CC_FAIL("Parse function #%zu: child #%zu: wrong offset.", testIndex, childIndex);
			}
		}

		for(size_t nodeIndex = 0; nodeIndex < tree.count; ++nodeIndex)
		{
			if(tree.nodes[nodeIndex].type != tests[testIndex].solution[nodeIndex].type)
			{
				CC_FAIL("Parse function #%zu: node #%zu: wrong type.", testIndex, nodeIndex);
				continue;
			}

			if(tree.nodes[nodeIndex].type == CC_NODE_FUNCTION)
			{
				if(
					tree.nodes[nodeIndex].function.name.string != tests[testIndex].solution[nodeIndex].function.name.string ||
					tree.nodes[nodeIndex].function.name.length != tests[testIndex].solution[nodeIndex].function.name.length
				)
				{
					CC_FAIL("Parse function #%zu: node #%zu: wrong function name.", testIndex, nodeIndex);
				}

				if(
					tree.nodes[nodeIndex].function.statementsCount != tests[testIndex].solution[nodeIndex].function.statementsCount || (
						tests[testIndex].childrenSolutionCount > 0 && tree.nodes[nodeIndex].function.statementsStart != tests[testIndex].solution[nodeIndex].function.statementsStart
					)
				)
				{
					CC_FAIL("Parse function #%zu: node #%zu: wrong function statements.", testIndex, nodeIndex);
				}
			}
		}
	}
}

static void ccTestProgram(bool* const pPassed)
{
	assert(pPassed != nullptr);

	const struct
	{
		const CcToken* tokens;
		size_t count;
		bool result;
		CcNode* solution;
		size_t solutionCount;
		const size_t* childrenSolution;
		size_t childrenSolutionCount;
	} tests[] = {
		(const CcToken[]){
			{.type = CC_TOKEN_INT},
			{.type = CC_TOKEN_IDENTIFIER, .string = {"some_func", 9}},
			{.type = CC_TOKEN_OPEN_PARENTHESIS},
			{.type = CC_TOKEN_CLOSE_PARENTHESIS},
			{.type = CC_TOKEN_OPEN_BRACE},
			{.type = CC_TOKEN_RETURN},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 1}},
			{.type = CC_TOKEN_SEMICOLON},
			{.type = CC_TOKEN_CLOSE_BRACE},
			{.type = CC_TOKEN_INT},
			{.type = CC_TOKEN_IDENTIFIER, .string = {"main", 4}},
			{.type = CC_TOKEN_OPEN_PARENTHESIS},
			{.type = CC_TOKEN_VOID},
			{.type = CC_TOKEN_CLOSE_PARENTHESIS},
			{.type = CC_TOKEN_OPEN_BRACE},
			{.type = CC_TOKEN_RETURN},
			{.type = CC_TOKEN_CONSTANT, .constant = {CC_CONSTANT_INT, 0}},
			{.type = CC_TOKEN_SEMICOLON},
			{.type = CC_TOKEN_CLOSE_BRACE}
		}, 19, true, (CcNode[]){
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 1}},
			{.type = CC_NODE_RETURN, .returnNode = 0},
			{.type = CC_NODE_FUNCTION, .function = {.statementsStart = 0, .statementsCount = 1}},
			{.type = CC_NODE_CONSTANT, .constant = {CC_CONSTANT_INT, 0}},
			{.type = CC_NODE_RETURN, .returnNode = 3},
			{.type = CC_NODE_FUNCTION, .function = {.statementsStart = 1, .statementsCount = 1}},
			{.type = CC_NODE_PROGRAM, .program = {.childrenStart = 3, .childrenCount = 2}}
		}, 7, (const size_t[]){
			1,
			4,
			5, 2
		}, 4
	};
	constexpr size_t testCount = CC_LEN(tests);

	tests[0].solution[2].function.name = tests[0].tokens[1].string;
	tests[0].solution[5].function.name = tests[0].tokens[10].string;

	CcNode nodes[64];
	size_t children[64];

	for(size_t testIndex = 0; testIndex < testCount; ++testIndex)
	{
		CcTree tree = {.nodes = nodes, .children = children};
		CcConstTokenList tokens = {tests[testIndex].tokens, tests[testIndex].count};

		CcTreeBuilder builder = {.pTree = &tree, .tokens = &tokens, .lastIndex = CC_LEN(children)};
		const bool result = ccParseProgram(&builder);

		if(result != tests[testIndex].result)
		{
			CC_FAIL("Parse program #%zu: wrong result.", testIndex);
			continue;
		}

		if(!result)
		{
			continue;
		}

		if(tree.count != tests[testIndex].solutionCount)
		{
			CC_FAIL("Parse program #%zu: wrong count.", testIndex);
			continue;
		}

		if(builder.childCount != tests[testIndex].childrenSolutionCount)
		{
			CC_FAIL("Parse program #%zu: wrong child count.", testIndex);
			continue;
		}

		for(size_t childIndex = 0; childIndex < builder.childCount; ++childIndex)
		{
			if(tree.children[childIndex] != tests[testIndex].childrenSolution[childIndex])
			{
				CC_FAIL("Parse program #%zu: child #%zu: wrong child.", testIndex, childIndex);
			}
		}

		for(size_t nodeIndex = 0; nodeIndex < tree.count; ++nodeIndex)
		{
			if(tree.nodes[nodeIndex].type != tests[testIndex].solution[nodeIndex].type)
			{
				CC_FAIL("Parse program #%zu: node #%zu: wrong type.", testIndex, nodeIndex);
				continue;
			}

			if(tree.nodes[nodeIndex].type == CC_NODE_PROGRAM)
			{
				if(tree.nodes[nodeIndex].program.childrenCount != tests[testIndex].solution[nodeIndex].program.childrenCount)
				{
					CC_FAIL("Parse program #%zu: node #%zu: wrong program child count.", testIndex, nodeIndex);
					continue;
				}

				if(tree.nodes[nodeIndex].program.childrenStart != tests[testIndex].solution[nodeIndex].program.childrenStart)
				{
					CC_FAIL("Parse program #%zu: node #%zu: wrong program children start.", testIndex, nodeIndex);
					continue;
				}
			}
		}
	}
}

int main(void)
{
	bool passed = true;

	ccTestFind(&passed);

	ccTestArguments(&passed);

	ccTestStrings(&passed);
	ccTestCharacters(&passed);
	ccTestTokens(&passed);
	ccTestConstants(&passed);
	ccTestIdentifiers(&passed);

	ccTestLex(&passed);

	ccTestParentheses(&passed);
	ccTestExpressions(&passed);
	ccTestStatements(&passed);
	ccTestFunctions(&passed);
	ccTestProgram(&passed);

	return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
