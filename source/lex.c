#include "cece/lex.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "cece/log.h"
#include "cece/memory.h"

typedef struct CcMapping
{
	CcConstString string;
	CcTokenType token;
} CcMapping;

#define CC_STRING(string) \
{string, sizeof(string) - 1}

static const CcMapping keywordMap[] = {
	{CC_STRING("void"), CC_TOKEN_VOID},
	{CC_STRING("char"), CC_TOKEN_CHAR},
	{CC_STRING("short"), CC_TOKEN_SHORT},
	{CC_STRING("int"), CC_TOKEN_INT},
	{CC_STRING("long"), CC_TOKEN_LONG},
	{CC_STRING("float"), CC_TOKEN_FLOAT},
	{CC_STRING("double"), CC_TOKEN_DOUBLE},
	{CC_STRING("signed"), CC_TOKEN_SIGNED},
	{CC_STRING("unsigned"), CC_TOKEN_UNSIGNED},
	{CC_STRING("const"), CC_TOKEN_CONST},
	{CC_STRING("volatile"), CC_TOKEN_VOLATILE},
	{CC_STRING("static"), CC_TOKEN_STATIC},
	{CC_STRING("extern"), CC_TOKEN_EXTERN},
	{CC_STRING("auto"), CC_TOKEN_AUTO},
	{CC_STRING("register"), CC_TOKEN_REGISTER},
	{CC_STRING("restrict"), CC_TOKEN_RESTRICT},
	{CC_STRING("typedef"), CC_TOKEN_TYPEDEF},
	{CC_STRING("struct"), CC_TOKEN_STRUCT},
	{CC_STRING("union"), CC_TOKEN_UNION},
	{CC_STRING("enum"), CC_TOKEN_ENUM},
	{CC_STRING("return"), CC_TOKEN_RETURN},
	{CC_STRING("sizeof"), CC_TOKEN_SIZEOF},
	{CC_STRING("if"), CC_TOKEN_IF},
	{CC_STRING("else"), CC_TOKEN_ELSE},
	{CC_STRING("switch"), CC_TOKEN_SWITCH},
	{CC_STRING("case"), CC_TOKEN_CASE},
	{CC_STRING("default"), CC_TOKEN_DEFAULT},
	{CC_STRING("while"), CC_TOKEN_WHILE},
	{CC_STRING("do"), CC_TOKEN_DO},
	{CC_STRING("for"), CC_TOKEN_FOR},
	{CC_STRING("break"), CC_TOKEN_BREAK},
	{CC_STRING("continue"), CC_TOKEN_CONTINUE},
	{CC_STRING("goto"), CC_TOKEN_GOTO},
	{CC_STRING("define"), CC_TOKEN_DEFINE},
	{CC_STRING("undef"), CC_TOKEN_UNDEF},
	{CC_STRING("ifdef"), CC_TOKEN_IFDEF},
	{CC_STRING("endif"), CC_TOKEN_ENDIF},
	{CC_STRING("include"), CC_TOKEN_INCLUDE}
};
static const size_t keywordCount = CC_LEN(keywordMap);

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
	{CC_STRING("=="), CC_TOKEN_EQUAL_EQUAL},
	{CC_STRING("!="), CC_TOKEN_NOT_EQUAL},
	{CC_STRING("<="), CC_TOKEN_LESS_EQUAL},
	{CC_STRING(">="), CC_TOKEN_GREATER_EQUAL},
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
	{CC_STRING(","), CC_TOKEN_COMMA},
	{CC_STRING("."), CC_TOKEN_DOT},
	{CC_STRING(":"), CC_TOKEN_COLON},
	{CC_STRING("#"), CC_TOKEN_HASH}
};
static const size_t tokenCount = CC_LEN(tokens);

#define CC_TOKEN_CASE(name) \
	case CC_TOKEN_##name: \
		return #name;

const char* ccTokenTypeString(CcTokenType type)
{
	switch(type)
	{
		CC_TOKEN(CC_TOKEN_CASE)

		default:
			return "?";
	}
}

/*
 * Check for token string equality.
 *
 * Parameters:
 * - pFirstVoid: A pointer to a CcConstString which is the token string.
 * - pSecondVoid: A pointer to a CcConstString which starts at the current index in the string stream.
 * 
 * Returns:
 * - 0 if the strings are equal.
 * - 1 otherwise.
 */
static int ccCompareToken(const void* pFirstVoid, const void* pSecondVoid)
{
	const CcConstString* pTokenString = pFirstVoid;
	const CcConstString* pString = pSecondVoid;

	if(pString->length < pTokenString->length)
	{
		return 1;
	}

	return strncmp(pTokenString->string, pString->string, pTokenString->length);
}

/*
 * Check for two strings equality.
 *
 * Parameters:
 * - pFirstVoid: A pointer to a CcConstString.
 * - pSecondVoid: A pointer to a CcConstString.
 * 
 * Returns:
 * - 0 if the strings are equal.
 * - 1 otherwise.
 */
static int ccCompareString(const void* pFirstVoid, const void* pSecondVoid)
{
	const CcConstString* pFirst = pFirstVoid;
	const CcConstString* pSecond = pSecondVoid;

	if(pFirst->length != pSecond->length)
	{
		return 1;
	}

	return strncmp(pFirst->string, pSecond->string, pFirst->length);
}

CcResult ccLex(const CcConstString* pString, CcTokenList* pTokenList)
{
	CcResult result = CC_SUCCESS;

	pTokenList->buffer = NULL;

	pTokenList->tokens = malloc(pString->length * sizeof(pTokenList->tokens[0]));
	if(!pTokenList->tokens)
	{
		CC_LOG("Failed to allocate memory.");
		result = CC_ERROR_OUT_OF_MEMORY;
		goto error;
	}

	pTokenList->buffer = malloc(pString->length);
	if(!pTokenList->buffer)
	{
		CC_LOG("Failed to allocate memory.");
		result = CC_ERROR_OUT_OF_MEMORY;
		goto error;
	}

	pTokenList->count = 0;

	size_t bufferLength = 0;

	for(size_t i = 0; i < pString->length; ++i, ++pTokenList->count)
	{
		while(isspace(pString->string[i]))
		{
			++i;
		}

		if(i >= pString->length)
		{
			break;
		}

		if(i < pString->length - 1 && pString->string[i] == '/' && pString->string[i + 1] == '/')
		{
			++i;
			while(i < pString->length && pString->string[i] != '\n')
			{
				++i;
			}

			--i;
			--pTokenList->count;

			continue;
		}

		if(i < pString->length - 1 && pString->string[i] == '/' && pString->string[i + 1] == '*')
		{
			++i;
			while(i < pString->length - 1 && (pString->string[i] != '*' || pString->string[i + 1] != '/'))
			{
				++i;
			}

			if(i >= pString->length - 1)
			{
				CC_LOG("Unterminated block comment.");
				result = CC_ERROR_UNKNOWN;
				goto error;
			}

			++i;

			--pTokenList->count;

			continue;
		}

		if(pString->string[i] == '<' && pTokenList->count >= 2 && pTokenList->tokens[pTokenList->count - 1].type == CC_TOKEN_INCLUDE && pTokenList->tokens[pTokenList->count - 2].type == CC_TOKEN_HASH)
		{
			pTokenList->tokens[pTokenList->count].type = CC_TOKEN_STRING;

			pTokenList->tokens[pTokenList->count].offset = bufferLength;

			const size_t start = i;
			while(i < pString->length && pString->string[i] != '>')
			{
				++i;
			}

			if(i >= pString->length)
			{
				CC_LOG("Unterminated include directive.");
				result = CC_ERROR_UNKNOWN;
				goto error;
			}

			pTokenList->tokens[pTokenList->count].length = i - start + 1;

			strncpy(pTokenList->buffer + bufferLength, pString->string + start, pTokenList->tokens[pTokenList->count].length);
			bufferLength += pTokenList->tokens[pTokenList->count].length;

			continue;
		}

		const size_t tokenIndex = ccFind(tokens, tokenCount, sizeof(tokens[0]), &(CcConstString){.string = pString->string + i, .length = pString->length - i}, ccCompareToken);
		if(tokenIndex < tokenCount)
		{
			pTokenList->tokens[pTokenList->count].type = tokens[tokenIndex].token;

			i += tokens[tokenIndex].string.length - 1;

			continue;
		}

		if(pString->string[i] == '\'')
		{
			pTokenList->tokens[pTokenList->count].type = CC_TOKEN_LITERAL;
			pTokenList->tokens[pTokenList->count].offset = bufferLength;

			const size_t start = i;

			++i;
			while(i < pString->length && (pString->string[i] != '\'' || (pString->string[i - 1] == '\\' && pString->string[i - 2] != '\\')))
			{
				++i;
			}

			if(i >= pString->length)
			{
				CC_LOG("Unterminated character literal.");
				result = CC_ERROR_UNKNOWN;
				goto error;
			}

			pTokenList->tokens[pTokenList->count].length = i - start + 1;

			strncpy(pTokenList->buffer + bufferLength, pString->string + start, pTokenList->tokens[pTokenList->count].length);
			bufferLength += pTokenList->tokens[pTokenList->count].length;

			continue;
		}

		if(pString->string[i] == '"')
		{
			pTokenList->tokens[pTokenList->count].type = CC_TOKEN_STRING;
			pTokenList->tokens[pTokenList->count].offset = bufferLength;

			const size_t start = i;

			++i;
			while(i < pString->length && (pString->string[i] != '"' || (pString->string[i - 1] == '\\' && pString->string[i - 2] != '\\')))
			{
				++i;
			}

			if(i >= pString->length)
			{
				CC_LOG("Unterminated string literal.");
				result = CC_ERROR_UNKNOWN;
				goto error;
			}

			pTokenList->tokens[pTokenList->count].length = i - start + 1;

			strncpy(pTokenList->buffer + bufferLength, pString->string + start, pTokenList->tokens[pTokenList->count].length);
			bufferLength += pTokenList->tokens[pTokenList->count].length;

			continue;
		}

		if(isdigit(pString->string[i]))
		{
			pTokenList->tokens[pTokenList->count].type = CC_TOKEN_LITERAL;
			pTokenList->tokens[pTokenList->count].intLiteral = 0;

			while(isalnum(pString->string[i]))
			{
				const char c = pString->string[i] - '0';
				if(pTokenList->tokens[pTokenList->count].intLiteral > (INT_MAX - c) / 10)
				{
					CC_LOG("Integer literal too large.");
					result = CC_ERROR_UNKNOWN;
					goto error;
				}

				pTokenList->tokens[pTokenList->count].intLiteral *= 10;
				pTokenList->tokens[pTokenList->count].intLiteral += c;

				++i;
			}

			--i;

			continue;
		}

		if(isalpha(pString->string[i]) || pString->string[i] == '_')
		{
			pTokenList->tokens[pTokenList->count].type = CC_TOKEN_IDENTIFIER;
			pTokenList->tokens[pTokenList->count].offset = bufferLength;

			const size_t start = i;

			while(isalnum(pString->string[i]) || pString->string[i] == '_')
			{
				++i;
			}
			pTokenList->tokens[pTokenList->count].length = i - start;

			const size_t keywordIndex = ccFind(keywordMap, keywordCount, sizeof(keywordMap[0]), &(CcConstString){.string = pString->string + start, .length = pTokenList->tokens[pTokenList->count].length}, ccCompareString);
			if(keywordIndex < keywordCount)
			{
				pTokenList->tokens[pTokenList->count].type = keywordMap[keywordIndex].token;
			}
			else
			{
				strncpy(pTokenList->buffer + bufferLength, pString->string + start, pTokenList->tokens[pTokenList->count].length);
				bufferLength += pTokenList->tokens[pTokenList->count].length;
			}

			--i;

			continue;
		}
	}

	char* const newBuffer = realloc(pTokenList->buffer, bufferLength);
	if(!newBuffer)
	{
		CC_LOG("Failed to allocate memory.");
		result = CC_ERROR_OUT_OF_MEMORY;
		goto error;
	}
	pTokenList->buffer = newBuffer;

	return result;

	error:
	CC_FREE(pTokenList->buffer)
	CC_FREE(pTokenList->tokens)
	return result;
}
