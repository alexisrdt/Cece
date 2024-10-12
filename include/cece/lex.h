#ifndef CECE_LEX_H
#define CECE_LEX_H

#include <stddef.h>

#include "cece/result.h"

#define CC_TOKEN(F) \
	F(IDENTIFIER) \
	F(VOID) \
	F(CHAR) \
	F(SHORT) \
	F(INT) \
	F(LONG) \
	F(FLOAT) \
	F(DOUBLE) \
	F(SIGNED) \
	F(UNSIGNED) \
	F(CONST) \
	F(VOLATILE) \
	F(STATIC) \
	F(EXTERN) \
	F(AUTO) \
	F(REGISTER) \
	F(RESTRICT) \
	F(TYPEDEF) \
	F(STRUCT) \
	F(UNION) \
	F(ENUM) \
	F(RETURN) \
	F(SIZEOF) \
	F(IF) \
	F(ELSE) \
	F(SWITCH) \
	F(CASE) \
	F(DEFAULT) \
	F(WHILE) \
	F(DO) \
	F(FOR) \
	F(BREAK) \
	F(CONTINUE) \
	F(GOTO) \
	F(DEFINE) \
	F(UNDEF) \
	F(IFDEF) \
	F(ENDIF) \
	F(INCLUDE) \
	F(LITERAL) \
	F(OPEN_PARENTHESIS) \
	F(CLOSE_PARENTHESIS) \
	F(OPEN_BRACE) \
	F(CLOSE_BRACE) \
	F(OPEN_BRACKET) \
	F(CLOSE_BRACKET) \
	F(SEMICOLON) \
	F(PLUS_EQUAL) \
	F(MINUS_EQUAL) \
	F(STAR_EQUAL) \
	F(SLASH_EQUAL) \
	F(PERCENT_EQUAL) \
	F(AMPERSAND_EQUAL) \
	F(BAR_EQUAL) \
	F(CARET_EQUAL) \
	F(LEFT_SHIFT_EQUAL) \
	F(RIGHT_SHIFT_EQUAL) \
	F(LEFT_SHIFT) \
	F(RIGHT_SHIFT) \
	F(EQUAL_EQUAL) \
	F(NOT_EQUAL) \
	F(LESS_EQUAL) \
	F(GREATER_EQUAL) \
	F(PLUS) \
	F(MINUS) \
	F(STAR) \
	F(SLASH) \
	F(PERCENT) \
	F(AMPERSAND) \
	F(BAR) \
	F(CARET) \
	F(TILDE) \
	F(EQUAL) \
	F(LESS) \
	F(GREATER) \
	F(EXCLAMATION) \
	F(COMMA) \
	F(DOT) \
	F(COLON) \
	F(STRING) \
	F(HASH)

#define CC_TOKEN_ENUM(name) \
	CC_TOKEN_##name,

/*
 * A token type.
 */
typedef enum CcTokenType
{
	CC_TOKEN(CC_TOKEN_ENUM)
} CcTokenType;

/*
 * Get string representation of a token type.
 *
 * Parameters:
 * - type: A token type.
 * 
 * Returns:
 * A string representation of the token type.
 */
const char* ccTokenTypeString(CcTokenType type);

/*
 * A token.
 *
 * Fields:
 * - type: The token type.
 * If the token is a literal:
 * - intLiteral: The integer literal value.
 * If the token is a string or identifier:
 * - offset: The offset of the token in the buffer.
 * - length: The length of the token.
 */
typedef struct CcToken
{
	CcTokenType type;

	union
	{
		int intLiteral;
		struct
		{
			size_t offset;
			size_t length;
		};
	};
} CcToken;

/*
 * A list of tokens.
 *
 * Fields:
 * - buffer: A character buffer for strings and identifiers.
 * - tokens: An array of tokens.
 * - count: The number of tokens.
 */
typedef struct CcTokenList
{
	char* buffer;

	CcToken* tokens;
	size_t count;
} CcTokenList;

typedef struct CcConstTokenList
{
	const char* buffer;

	const CcToken* tokens;
	size_t count;
} CcConstTokenList;

/*
 * A constant string.
 * Should almost always be used as a constant.
 *
 * Fields:
 * - string: A string. Cece always makes it length + 1-wide and null-terminated.
 * - length: The length of the string.
 */
typedef struct CcConstString
{
	const char* string;
	size_t length;
} CcConstString;

/*
 * Lex a string into a list of tokens.
 *
 * Parameters:
 * - pString: A string.
 * - pTokenList: A list of tokens.
 * 
 * Returns:
 * - CC_SUCCESS if the string is successfully lexed.
 * - CC_ERROR_OUT_OF_MEMORY if memory allocation fails.
 */
CcResult ccLex(const CcConstString* pString, CcTokenList* pTokenList);

#endif
