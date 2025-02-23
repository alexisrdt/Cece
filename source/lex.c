#include "cece/lex.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cece/memory.h"

typedef struct CcMapping
{
	CcConstString string;
	CcTokenType token;
} CcMapping;

#define CC_STRING(string) \
{string, sizeof(string) - 1}

// Keywords must be stored from longest to shortest and in alphabetical order for bsearch.
// They could also be stored from shortest to longest, we won't miss longer keywords (e.g. "do" and "double") because we know the length of the identifier.
static const CcMapping keywordMap[] = {
	{CC_STRING("_Static_assert"), CC_TOKEN__STATIC_ASSERT},
	{CC_STRING("static_assert"), CC_TOKEN_STATIC_ASSERT},
	{CC_STRING("typeof_unqual"), CC_TOKEN_TYPEOF_UNQUAL},
	{CC_STRING("_Thread_local"), CC_TOKEN__THREAD_LOCAL},
	{CC_STRING("thread_local"), CC_TOKEN_THREAD_LOCAL},
	{CC_STRING("_Decimal128"), CC_TOKEN__DECIMAL_128},
	{CC_STRING("_Decimal32"), CC_TOKEN__DECIMAL_32},
	{CC_STRING("_Decimal64"), CC_TOKEN__DECIMAL_64},
	{CC_STRING("_Imaginary"), CC_TOKEN__IMAGINARY},
	{CC_STRING("constexpr"), CC_TOKEN_CONSTEXPR},
	{CC_STRING("_Alignas"), CC_TOKEN__ALIGNAS},
	{CC_STRING("_Alignof"), CC_TOKEN__ALIGNOF},
	{CC_STRING("_Complex"), CC_TOKEN__COMPLEX},
	{CC_STRING("_Generic"), CC_TOKEN__GENERIC},
	{CC_STRING("continue"), CC_TOKEN_CONTINUE},
	{CC_STRING("register"), CC_TOKEN_REGISTER},
	{CC_STRING("restrict"), CC_TOKEN_RESTRICT},
	{CC_STRING("unsigned"), CC_TOKEN_UNSIGNED},
	{CC_STRING("volatile"), CC_TOKEN_VOLATILE},
	{CC_STRING("_Atomic"), CC_TOKEN__ATOMIC},
	{CC_STRING("_BitInt"), CC_TOKEN__BIT_INT},
	{CC_STRING("alignas"), CC_TOKEN_ALIGNAS},
	{CC_STRING("alignof"), CC_TOKEN_ALIGNOF},
	{CC_STRING("default"), CC_TOKEN_DEFAULT},
	{CC_STRING("typedef"), CC_TOKEN_TYPEDEF},
	{CC_STRING("double"), CC_TOKEN_DOUBLE},
	{CC_STRING("extern"), CC_TOKEN_EXTERN},
	{CC_STRING("return"), CC_TOKEN_RETURN},
	{CC_STRING("signed"), CC_TOKEN_SIGNED},
	{CC_STRING("sizeof"), CC_TOKEN_SIZEOF},
	{CC_STRING("static"), CC_TOKEN_STATIC},
	{CC_STRING("struct"), CC_TOKEN_STRUCT},
	{CC_STRING("switch"), CC_TOKEN_SWITCH},
	{CC_STRING("typeof"), CC_TOKEN_TYPEOF},
	{CC_STRING("_Bool"), CC_TOKEN__BOOL},
	{CC_STRING("break"), CC_TOKEN_BREAK},
	{CC_STRING("const"), CC_TOKEN_CONST},
	{CC_STRING("false"), CC_TOKEN_FALSE},
	{CC_STRING("float"), CC_TOKEN_FLOAT},
	{CC_STRING("short"), CC_TOKEN_SHORT},
	{CC_STRING("union"), CC_TOKEN_UNION},
	{CC_STRING("while"), CC_TOKEN_WHILE},
	{CC_STRING("auto"), CC_TOKEN_AUTO},
	{CC_STRING("bool"), CC_TOKEN_BOOL},
	{CC_STRING("case"), CC_TOKEN_CASE},
	{CC_STRING("char"), CC_TOKEN_CHAR},
	{CC_STRING("else"), CC_TOKEN_ELSE},
	{CC_STRING("enum"), CC_TOKEN_ENUM},
	{CC_STRING("goto"), CC_TOKEN_GOTO},
	{CC_STRING("long"), CC_TOKEN_LONG},
	{CC_STRING("true"), CC_TOKEN_TRUE},
	{CC_STRING("void"), CC_TOKEN_VOID},
	{CC_STRING("for"), CC_TOKEN_FOR},
	{CC_STRING("int"), CC_TOKEN_INT},
	{CC_STRING("do"), CC_TOKEN_DO},
	{CC_STRING("if"), CC_TOKEN_IF}
};
constexpr size_t keywordCount = CC_LEN(keywordMap);

// Tokens must be stored from longest to shortest to avoid missing longer tokens (e.g. "+" and "=" instead of "+=").
static const CcMapping tokens[] = {
	{CC_STRING("<<="), CC_TOKEN_LEFT_SHIFT_EQUAL},
	{CC_STRING(">>="), CC_TOKEN_RIGHT_SHIFT_EQUAL},
	{CC_STRING("+="), CC_TOKEN_PLUS_EQUAL},
	{CC_STRING("-="), CC_TOKEN_MINUS_EQUAL},
	{CC_STRING("*="), CC_TOKEN_STAR_EQUAL},
	{CC_STRING("/="), CC_TOKEN_SLASH_EQUAL},
	{CC_STRING("%="), CC_TOKEN_PERCENT_EQUAL},
	{CC_STRING("&="), CC_TOKEN_AMPERSAND_EQUAL},
	{CC_STRING("|="), CC_TOKEN_BAR_EQUAL},
	{CC_STRING("^="), CC_TOKEN_CARET_EQUAL},
	{CC_STRING("<<"), CC_TOKEN_LEFT_SHIFT},
	{CC_STRING(">>"), CC_TOKEN_RIGHT_SHIFT},
	{CC_STRING("++"), CC_TOKEN_PLUS_PLUS},
	{CC_STRING("--"), CC_TOKEN_MINUS_MINUS},
	{CC_STRING("=="), CC_TOKEN_EQUAL_EQUAL},
	{CC_STRING("!="), CC_TOKEN_NOT_EQUAL},
	{CC_STRING("<="), CC_TOKEN_LESS_EQUAL},
	{CC_STRING(">="), CC_TOKEN_GREATER_EQUAL},
	{CC_STRING("&&"), CC_TOKEN_AMPERSAND_AMPERSAND},
	{CC_STRING("||"), CC_TOKEN_BAR_BAR},
	{CC_STRING("->"), CC_TOKEN_ARROW},
	{CC_STRING("("), CC_TOKEN_OPEN_PARENTHESIS},
	{CC_STRING(")"), CC_TOKEN_CLOSE_PARENTHESIS},
	{CC_STRING("{"), CC_TOKEN_OPEN_BRACE},
	{CC_STRING("}"), CC_TOKEN_CLOSE_BRACE},
	{CC_STRING("["), CC_TOKEN_OPEN_BRACKET},
	{CC_STRING("]"), CC_TOKEN_CLOSE_BRACKET},
	{CC_STRING(";"), CC_TOKEN_SEMICOLON},
	{CC_STRING("+"), CC_TOKEN_PLUS},
	{CC_STRING("-"), CC_TOKEN_MINUS},
	{CC_STRING("*"), CC_TOKEN_STAR},
	{CC_STRING("/"), CC_TOKEN_SLASH},
	{CC_STRING("%"), CC_TOKEN_PERCENT},
	{CC_STRING("&"), CC_TOKEN_AMPERSAND},
	{CC_STRING("|"), CC_TOKEN_BAR},
	{CC_STRING("^"), CC_TOKEN_CARET},
	{CC_STRING("~"), CC_TOKEN_TILDE},
	{CC_STRING("="), CC_TOKEN_EQUAL},
	{CC_STRING("<"), CC_TOKEN_LESS},
	{CC_STRING(">"), CC_TOKEN_GREATER},
	{CC_STRING("!"), CC_TOKEN_EXCLAMATION},
	{CC_STRING("?"), CC_TOKEN_QUESTION},
	{CC_STRING(","), CC_TOKEN_COMMA},
	{CC_STRING("."), CC_TOKEN_DOT},
	{CC_STRING(":"), CC_TOKEN_COLON}
};
constexpr size_t tokenCount = CC_LEN(tokens);
constexpr size_t tokenSize = sizeof(tokens[0]);

#define CC_TOKEN_CASE(name) \
	case CC_TOKEN_##name: \
		return #name;

const char* ccTokenTypeString(const CcTokenType type)
{
	assert(type >= CC_TOKEN_IDENTIFIER && type <= CC_TOKEN_STRING);

	switch(type)
	{
		CC_TOKEN(CC_TOKEN_CASE)

		default:
			return nullptr;
	}
}

bool ccIsKeyword(const CcTokenType type)
{
	assert(type >= CC_TOKEN_IDENTIFIER && type <= CC_TOKEN_STRING);

	return type >= CC_TOKEN_VOID && type <= CC_TOKEN_GOTO;
}

/*
 * Check for token string equality.
 *
 * Parameters:
 * - pStringVoid: A string.
 * - pTokenVoid: A pointer to a CcConstString which is the token string.
 *
 * Returns:
 * - 0 if the strings are equal.
 * - 1 otherwise.
 */
static int ccCompareToken(const void* const stringVoid, const void* const pTokenVoid)
{
	const char* const string = stringVoid;
	const CcConstString* const pToken = pTokenVoid;

	return strncmp(pToken->string, string, pToken->length);
}

/*
 * Check for two strings equality.
 *
 * Parameters:
 * - pStringVoid: A pointer to a CcConstString to test.
 * - pKeywordVoid: A pointer to a keyword CcConstString.
 *
 * Returns:
 * - 0 if the strings are equal.
 * - -1 if the string is before the keyword.
 * - 1 if the string is after the keyword.
 */
static int ccCompareString(const void* const pStringVoid, const void* const pKeywordVoid)
{
	const CcConstString* const pString = pStringVoid;
	const CcConstString* const pKeyword = pKeywordVoid;

	if(pString->length != pKeyword->length)
	{
		return (pString->length < pKeyword->length) - (pString->length > pKeyword->length);
	}

	return strncmp(pString->string, pKeyword->string, pString->length);
}

/*
 * Pop a character from a string.
 *
 * Parameters:
 * - pString: A pointer to a string.
 * - count: The number of characters to pop.
 */
static void ccPop(CcConstString* const pString, const size_t count)
{
	pString->string += count;
	pString->length -= count;
}

/*
 * Skip spaces.
 *
 * Parameters:
 * - pString: A pointer to a string.
 */
static void ccSkipSpaces(CcConstString* const pString)
{
	while(isspace((unsigned char)*pString->string))
	{
		ccPop(pString, 1);
	}
}

bool ccParseString(const char* string, CcToken* const pToken)
{
	assert(string != nullptr);
	assert(pToken != nullptr);

	if(*string != '"')
	{
		return false;
	}

	pToken->type = CC_TOKEN_STRING;
	pToken->string.string = string;

	++string;

	while(*string != '\0' && (*string != '"' || *(string - 1) == '\\'))
	{
		if(!isprint((unsigned char)*string))
		{
			fprintf(stderr, "Invalid character in string literal.\n");
		}

		++string;
	}
	if(*string == '"')
	{
		++string;
	}
	else
	{
		fprintf(stderr, "Unfinished string literal.\n");
	}

	pToken->string.length = string - pToken->string.string;

	return true;
}

bool ccParseCharacter(const char* string, CcToken* const pToken)
{
	assert(string != nullptr);
	assert(pToken != nullptr);

	if(*string != '\'')
	{
		return false;
	}

	pToken->type = CC_TOKEN_CONSTANT;
	pToken->constant.type = CC_CONSTANT_INT;
	pToken->string.string = string;
	pToken->string.length = 3;

	++string;

	if(
		string[0] == '\0' ||
		(string[0] != '\\' && string[1] != '\'') ||
		(string[0] == '\\' && (string[1] == '\0' || string[2] != '\''))
	)
	{
		fprintf(stderr, "Invalid character constant.\n");
		return false;
	}

	if(string[0] == '\\')
	{
		++string;
		++pToken->string.length;
	}

	pToken->constant.value = string[0];

	return true;
}

bool ccParseToken(const char* const string, CcToken* const pToken)
{
	assert(string != nullptr);
	assert(pToken != nullptr);

	const CcMapping* const pMapping = ccFind(string, tokens, tokenCount, tokenSize, ccCompareToken);
	if(!pMapping)
	{
		return false;
	}

	pToken->type = pMapping->token;
	pToken->string.string = string;
	pToken->string.length = pMapping->string.length;

	return true;
}

/*
 * Check if a character is a digit in some base.
 *
 * Parameters:
 * - character: The character to test.
 * - base: The base, should be 2, 8, 10 or 16.
 *
 * Returns:
 * - true if the character is a digit in the given base.
 * - false otherwise.
 */
static bool ccIsDigit(const char character, const unsigned char base)
{
	static constexpr char digits[16] = "0123456789ABCDEF";

	const char upperCharacter = toupper((unsigned char)character);

	for(unsigned char digitIndex = 0; digitIndex < base; ++digitIndex)
	{
		if(upperCharacter == digits[digitIndex])
		{
			return true;
		}
	}

	return false;
}

bool ccParseConstant(const char* string, CcToken* const pToken)
{
	assert(string != nullptr);
	assert(pToken != nullptr);

	if(!isdigit((unsigned char)*string))
	{
		return false;
	}

	pToken->string.string = string;

	pToken->type = CC_TOKEN_CONSTANT;

	unsigned char base = 10;

	if(string[0] == '0')
	{
		if(tolower((unsigned char)string[1]) == 'x')
		{
			base = 16;
			string += 2;
		}
		else if(tolower((unsigned char)string[1]) == 'b')
		{
			base = 2;
			string += 2;
		}
		else if(isdigit((unsigned char)string[1]))
		{
			base = 8;
			++string;
		}
	}

	CcConstantType type = CC_CONSTANT_INT;

	const unsigned char* temp = (const unsigned char*)string;
	while(ccIsDigit(*temp, base))
	{
		++temp;
	}
	if(isalpha(*temp))
	{
		if(
			(
				(tolower(temp[0]) == 'u' && tolower(temp[1]) == 'l' && temp[2] == temp[1]) ||
				(tolower(temp[0]) == 'l' && temp[1] == temp[0] && tolower(temp[2]) == 'u')
			) && !isalnum(temp[3])
		)
		{
			type = CC_CONSTANT_UNSIGNED_LONG_LONG;
		}

		else if(
			(
				(tolower(temp[0]) == 'u' && tolower(temp[1]) == 'l') ||
				(tolower(temp[0]) == 'l' && tolower(temp[1]) == 'u')
			) && !isalnum(temp[2])
		)
		{
			type = CC_CONSTANT_UNSIGNED_LONG;
		}

		else if(
			tolower(temp[0]) == 'u' && !isalnum(temp[1])
		)
		{
			type = CC_CONSTANT_UNSIGNED_INT;
		}

		else if(
			tolower(temp[0]) == 'l' && temp[1] == temp[0] && !isalnum(temp[2])
		)
		{
			type = CC_CONSTANT_LONG_LONG;
		}

		else if(
			tolower(temp[0]) == 'l' && !isalnum(temp[1])
		)
		{
			type = CC_CONSTANT_LONG;
		}

		else
		{
			fprintf(stderr, "Invalid integer literal suffix.\n");
		}
	}

	const bool isUnsigned = type >= CC_CONSTANT_UNSIGNED_INT;

	unsigned long long compare = INT_MAX;
	switch(type)
	{
		case CC_CONSTANT_UNSIGNED_LONG_LONG:
			compare = ULLONG_MAX;
			break;

		case CC_CONSTANT_UNSIGNED_LONG:
			compare = ULONG_MAX;
			break;

		case CC_CONSTANT_UNSIGNED_INT:
			compare = UINT_MAX;
			break;

		case CC_CONSTANT_LONG_LONG:
			compare = LLONG_MAX;
			break;

		case CC_CONSTANT_LONG:
			compare = LONG_MAX;
			break;

		case CC_CONSTANT_INT:
			compare = INT_MAX;
			break;

		default:
			fprintf(stderr, "Unreachable.\n");
			break;
	}

	pToken->constant.value = 0;

	while(ccIsDigit(*string, base))
	{
		const unsigned char c = ('0' <= *string && *string <= '9') ? *string - '0' : (unsigned char)(tolower((unsigned char)*string) - 'a' + 10);
		++string;

		while(pToken->constant.value > (compare - c) / base)
		{
			if(compare == LLONG_MAX && base != 10 && !isUnsigned)
			{
				compare = ULLONG_MAX;
				type = CC_CONSTANT_UNSIGNED_LONG_LONG;
			}
			else if(compare == ULLONG_MAX || compare == LLONG_MAX)
			{
				fprintf(stderr, "Integer literal too large.\n");
				break;
			}
			else if(compare == ULONG_MAX)
			{
				if(base == 10 || isUnsigned)
				{
					compare = ULLONG_MAX;
					type = CC_CONSTANT_UNSIGNED_LONG_LONG;
				}
				else
				{
					compare = LLONG_MAX;
					type = CC_CONSTANT_LONG_LONG;
				}
			}
			else if(compare == LONG_MAX)
			{
				if(base == 10)
				{
					compare = LLONG_MAX;
					type = CC_CONSTANT_LONG_LONG;
				}
				else
				{
					compare = ULONG_MAX;
					type = CC_CONSTANT_UNSIGNED_LONG;
				}
			}
			else if(compare == UINT_MAX)
			{
				if(base == 10 || isUnsigned)
				{
					compare = ULONG_MAX;
					type = CC_CONSTANT_UNSIGNED_LONG;
				}
				else
				{
					compare = LONG_MAX;
					type = CC_CONSTANT_LONG;
				}
			}
			else if(compare == INT_MAX)
			{
				if(base == 10)
				{
					compare = LONG_MAX;
					type = CC_CONSTANT_LONG;
				}
				else
				{
					compare = UINT_MAX;
					type = CC_CONSTANT_UNSIGNED_INT;
				}
			}
			else
			{
				fprintf(stderr, "Should not happen.\n");
			}
		}

		pToken->constant.value *= base;
		pToken->constant.value += c;
	}

	pToken->constant.type = type;

	while(isalnum((unsigned char)*string))
	{
		++string;
	}

	pToken->string.length = string - pToken->string.string;

	return true;
}

bool ccParseIdentifier(const char* string, CcToken* const pToken)
{
	assert(string != nullptr);
	assert(pToken != nullptr);

	if(!isalpha((unsigned char)*string) && *string != '_')
	{
		return false;
	}

	pToken->type = CC_TOKEN_IDENTIFIER;
	pToken->string.string = string;

	++string;
	while(isalnum((unsigned char)*string) || *string == '_')
	{
		++string;
	}

	pToken->string.length = string - pToken->string.string;

	CcMapping* const pKeyword = bsearch(&pToken->string, keywordMap, keywordCount, sizeof(keywordMap[0]), ccCompareString);
	if(pKeyword)
	{
		pToken->type = pKeyword->token;
	}

	return true;
}

CcResult ccLex(CcConstString string, CcTokenList* const pTokenList)
{
	// Validate arguments.
	assert(string.string != nullptr);
	assert(string.length == strlen(string.string));
	assert(pTokenList != nullptr);

	pTokenList->count = 0;

	if(string.length == 0)
	{
		pTokenList->tokens = nullptr;

		return CC_SUCCESS;
	}

	if(ccSizeMax / sizeof(pTokenList->tokens[0]) < string.length)
	{
		return CC_ERROR_OUT_OF_MEMORY;
	}

	pTokenList->tokens = malloc(string.length * sizeof(pTokenList->tokens[0]));
	if(!pTokenList->tokens)
	{
		fprintf(stderr, "Failed to allocate memory.\n");
		return CC_ERROR_OUT_OF_MEMORY;
	}

	while(*string.string)
	{
		ccSkipSpaces(&string);
		if(!*string.string)
		{
			break;
		}

		if(ccParseString(string.string, &pTokenList->tokens[pTokenList->count]))
		{
			goto add;
		}

		if(ccParseCharacter(string.string, &pTokenList->tokens[pTokenList->count]))
		{
			goto add;
		}

		if(ccParseToken(string.string, &pTokenList->tokens[pTokenList->count]))
		{
			goto add;
		}

		if(ccParseConstant(string.string, &pTokenList->tokens[pTokenList->count]))
		{
			goto add;
		}

		if(ccParseIdentifier(string.string, &pTokenList->tokens[pTokenList->count]))
		{
			goto add;
		}

		fprintf(stderr, "Unexpected token.\n");
		ccPop(&string, 1);
		continue;

		add:
		pTokenList->tokens[pTokenList->count].string.string = string.string;
		ccPop(&string, pTokenList->tokens[pTokenList->count].string.length);
		++pTokenList->count;
	}

	if(pTokenList->count == 0)
	{
		CC_FREE(pTokenList->tokens);
	}
	else
	{
		CcToken* const newTokens = realloc(pTokenList->tokens, pTokenList->count * sizeof(pTokenList->tokens[0]));
		if(!newTokens)
		{
			return CC_ERROR_UNKNOWN;
		}
		pTokenList->tokens = newTokens;
	}

	return CC_SUCCESS;
}

void ccFreeTokenList(CcTokenList* const pTokenList)
{
	assert(pTokenList != nullptr);

	CC_FREE(pTokenList->tokens);
	pTokenList->count = 0;
}
