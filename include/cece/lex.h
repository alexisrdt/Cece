#ifndef CECE_LEX_H
#define CECE_LEX_H

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
 * A string view.
 * Alias of CcConstString, not null-terminated.
 */
typedef CcConstString CcStringView;

#define CC_TOKEN(F) \
	F(IDENTIFIER) \
	/* Keywords */ \
	F(VOID) \
	F(_BOOL) \
	F(BOOL) \
	F(FALSE) \
	F(TRUE) \
	F(CHAR) \
	F(SHORT) \
	F(INT) \
	F(LONG) \
	F(FLOAT) \
	F(DOUBLE) \
	F(_DECIMAL_32) \
	F(_DECIMAL_64) \
	F(_DECIMAL_128) \
	F(_COMPLEX) \
	F(_IMAGINARY) \
	F(SIGNED) \
	F(UNSIGNED) \
	F(CONST) \
	F(CONSTEXPR) \
	F(VOLATILE) \
	F(STATIC) \
	F(EXTERN) \
	F(AUTO) \
	F(REGISTER) \
	F(RESTRICT) \
	F(TYPEDEF) \
	F(TYPEOF) \
	F(TYPEOF_UNQUAL) \
	F(STRUCT) \
	F(UNION) \
	F(ENUM) \
	F(_ATOMIC) \
	F(_GENERIC) \
	F(_BIT_INT) \
	F(_THREAD_LOCAL) \
	F(THREAD_LOCAL) \
	F(RETURN) \
	F(_STATIC_ASSERT) \
	F(STATIC_ASSERT) \
	F(SIZEOF) \
	F(_ALIGNAS) \
	F(ALIGNAS) \
	F(_ALIGNOF) \
	F(ALIGNOF) \
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
	/* End of keywords */ \
	F(CONSTANT) \
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
	F(PLUS_PLUS) \
	F(MINUS_MINUS) \
	F(EQUAL_EQUAL) \
	F(NOT_EQUAL) \
	F(LESS_EQUAL) \
	F(GREATER_EQUAL) \
	F(AMPERSAND_AMPERSAND) \
	F(BAR_BAR) \
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
	F(QUESTION) \
	F(COMMA) \
	F(DOT) \
	F(ARROW) \
	F(COLON) \
	F(STRING)

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

bool ccIsKeyword(CcTokenType type);

typedef enum CcConstantType
{
	CC_CONSTANT_INT,
	CC_CONSTANT_LONG,
	CC_CONSTANT_LONG_LONG,
	CC_CONSTANT_UNSIGNED_INT,
	CC_CONSTANT_UNSIGNED_LONG,
	CC_CONSTANT_UNSIGNED_LONG_LONG
} CcConstantType;

typedef struct CcConstant
{
	CcConstantType type;
	unsigned long long value;
} CcConstant;

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

	CcStringView string;

	CcConstant constant;
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
	CcToken* tokens;
	size_t count;
} CcTokenList;

typedef struct CcConstTokenList
{
	const CcToken* tokens;
	size_t count;
} CcConstTokenList;

/*
 * Parse a string literal.
 *
 * Parameters:
 * - string: A string.
 * - pToken: A pointer to a token to store the result.
 *
 * Returns:
 * - true if a string literal was found.
 * - false otherwise.
 */
bool ccParseString(const char* string, CcToken* pToken);

/*
 * Parse a character constant.
 *
 * Parameters:
 * - string: A string.
 * - pToken: A pointer to a token to store the result.
 *
 * Returns:
 * - true if a character constant was found.
 * - false otherwise.
 */
bool ccParseCharacter(const char* string, CcToken* const pToken);

/*
 * Parse a token that does not fit in another category (operators, punctuation).
 *
 * Parameters:
 * - string: A string.
 * - pToken: A pointer to a token to store the result.
 *
 * Returns:
 * - true if a token was found.
 * - false otherwise.
 */
bool ccParseToken(const char* string, CcToken* pToken);

/*
 * Parse an integer constant.
 *
 * Parameters:
 * - string: A string.
 * - pToken: A pointer to a token to store the result.
 *
 * Returns:
 * - true if an integer constant was found.
 * - false otherwise.
 */
bool ccParseConstant(const char* string, CcToken* pToken);

/*
 * Parse an identifier or keyword.
 *
 * Parameters:
 * - string: A string.
 * - pToken: A pointer to a token to store the result.
 *
 * Returns:
 * - true if an identifier or keyword was found.
 * - false otherwise.
 */
bool ccParseIdentifier(const char* string, CcToken* pToken);

/*
 * Lex a string into a list of tokens.
 *
 * Parameters:
 * - string: A string.
 * - pTokenList: A pointer to a list of tokens.
 *
 * Returns:
 * - CC_SUCCESS if the string is successfully lexed.
 * - CC_ERROR_OUT_OF_MEMORY if memory allocation fails.
 */
CcResult ccLex(CcConstString string, CcTokenList* pTokenList);

/*
 * Free a token list.
 *
 * Parameters:
 * - pTokenList: A pointer to a token list.
 */
void ccFreeTokenList(CcTokenList* pTokenList);

#endif
