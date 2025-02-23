#include "cece/tree.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cece/memory.h"

#ifndef NDEBUG
static bool ccAssertBuilder(const CcTreeBuilder* const pBuilder)
{
	assert(pBuilder != nullptr);

	assert(pBuilder->pTree != nullptr);
	assert(pBuilder->pTree->nodes != nullptr);
	assert(pBuilder->pTree->children != nullptr);

	assert(pBuilder->tokens != nullptr);
	assert(pBuilder->tokens->tokens != nullptr);
	// assert(pBuilder->tokens->count > 0);

	return true;
}
#endif

bool ccSkipParentheses(size_t* const pTokenIndex, const CcConstTokenList* const tokens, const CcDirection direction)
{
	assert(pTokenIndex != nullptr);

	assert(tokens != nullptr);
	assert(tokens->tokens != nullptr);
	assert(tokens->count > 0);

	assert(*pTokenIndex < tokens->count);

	assert(direction == CC_DIRECTION_FORWARD || direction == CC_DIRECTION_BACKWARD);

	const CcTokenType type = tokens->tokens[*pTokenIndex].type;

	if(type != CC_TOKEN_OPEN_PARENTHESIS && type != CC_TOKEN_CLOSE_PARENTHESIS)
	{
		return true;
	}

	if(direction == CC_DIRECTION_FORWARD && type == CC_TOKEN_CLOSE_PARENTHESIS)
	{
		return false;
	}
	if(direction == CC_DIRECTION_BACKWARD && type == CC_TOKEN_OPEN_PARENTHESIS)
	{
		return false;
	}

	const CcTokenType plusType = type;
	const CcTokenType minusType = direction == CC_DIRECTION_FORWARD ? CC_TOKEN_CLOSE_PARENTHESIS : CC_TOKEN_OPEN_PARENTHESIS;

	size_t parenthesisCount = 1;
	while(direction == CC_DIRECTION_FORWARD ? *pTokenIndex < tokens->count - 1 : *pTokenIndex > 0)
	{
		*pTokenIndex += direction;

		const CcTokenType currentType = tokens->tokens[*pTokenIndex].type;

		parenthesisCount += currentType == plusType;
		parenthesisCount -= currentType == minusType;

		if(parenthesisCount == 0)
		{
			return true;
		}
	}

	return false;
}

typedef enum CcParseResult
{
	CC_PARSE_INVALID,
	CC_PARSE_MATCH,
	CC_PARSE_NO_MATCH
} CcParseResult;

static CcParseResult ccParseBinaryOperator(CcTreeBuilder* const pBuilder, const CcTokenType* const tokens, const CcBinOp* const binOps, const size_t count)
{
	assert(ccAssertBuilder(pBuilder));

	size_t tokenIndex = pBuilder->tokens->count - 1;
	while(tokenIndex > 0)
	{
		if(!ccSkipParentheses(&tokenIndex, pBuilder->tokens, CC_DIRECTION_BACKWARD))
		{
			return CC_PARSE_INVALID;
		}
		--tokenIndex;

		size_t matchIndex;
		for(matchIndex = 0; matchIndex < count; ++matchIndex)
		{
			if(pBuilder->tokens->tokens[tokenIndex].type == tokens[matchIndex])
			{
				goto parse;
			}
		}
		continue;

		parse:
		if(tokenIndex == 0 || tokenIndex == pBuilder->tokens->count - 1)
		{
			return false;
		}

		if(!ccParseExpression(&(CcTreeBuilder){
			.pTree = pBuilder->pTree,
			.tokens = &(CcConstTokenList){
				pBuilder->tokens->tokens,
				tokenIndex
			}
		}))
		{
			return CC_PARSE_INVALID;
		}

		const size_t leftNodeIndex = pBuilder->pTree->count - 1;

		if(!ccParseExpression(&(CcTreeBuilder){
			.pTree = pBuilder->pTree,
			.tokens = &(CcConstTokenList){
				pBuilder->tokens->tokens + tokenIndex + 1,
				pBuilder->tokens->count - tokenIndex - 1
			}
		}))
		{
			return CC_PARSE_INVALID;
		}

		pBuilder->pTree->nodes[pBuilder->pTree->count].type = CC_NODE_BIN_OP;
		pBuilder->pTree->nodes[pBuilder->pTree->count].binOpNode.op = binOps[matchIndex];

		pBuilder->pTree->nodes[pBuilder->pTree->count].binOpNode.leftNode = leftNodeIndex;
		pBuilder->pTree->nodes[pBuilder->pTree->count].binOpNode.rightNode = pBuilder->pTree->count - 1;

		++pBuilder->pTree->count;

		return CC_PARSE_MATCH;
	}

	return CC_PARSE_NO_MATCH;
}

bool ccParseExpression(CcTreeBuilder* const pBuilder)
{
	assert(ccAssertBuilder(pBuilder));

	// Remove potential outer parentheses.
	size_t tokenIndex = 0;
	if(!ccSkipParentheses(&tokenIndex, pBuilder->tokens, CC_DIRECTION_FORWARD))
	{
		return false;
	}
	if(pBuilder->tokens->tokens[tokenIndex].type == CC_TOKEN_CLOSE_PARENTHESIS && tokenIndex == pBuilder->tokens->count - 1)
	{
		if(pBuilder->tokens->count < 3)
		{
			return false;
		}

		return ccParseExpression(&(CcTreeBuilder){
			.pTree = pBuilder->pTree,
			.tokens = &(CcConstTokenList){
				.tokens = pBuilder->tokens->tokens + 1,
				.count = pBuilder->tokens->count - 2
			}
		});
	}

	{
		constexpr CcTokenType tokens[] = {CC_TOKEN_BAR_BAR};
		constexpr CcBinOp binOps[] = {CC_BIN_OP_LOR};
		constexpr size_t count = CC_LEN(tokens);
		static_assert(CC_LEN(binOps) == count);

		const CcParseResult result = ccParseBinaryOperator(pBuilder, tokens, binOps, count);
		if(result == CC_PARSE_INVALID)
		{
			return false;
		}
		if(result == CC_PARSE_MATCH)
		{
			return true;
		}
	}

	{
		constexpr CcTokenType tokens[] = {CC_TOKEN_AMPERSAND_AMPERSAND};
		constexpr CcBinOp binOps[] = {CC_BIN_OP_LAND};
		constexpr size_t count = CC_LEN(tokens);
		static_assert(CC_LEN(binOps) == count);

		const CcParseResult result = ccParseBinaryOperator(pBuilder, tokens, binOps, count);
		if(result == CC_PARSE_INVALID)
		{
			return false;
		}
		if(result == CC_PARSE_MATCH)
		{
			return true;
		}
	}

	{
		constexpr CcTokenType tokens[] = {CC_TOKEN_BAR};
		constexpr CcBinOp binOps[] = {CC_BIN_OP_OR};
		constexpr size_t count = CC_LEN(tokens);
		static_assert(CC_LEN(binOps) == count);

		const CcParseResult result = ccParseBinaryOperator(pBuilder, tokens, binOps, count);
		if(result == CC_PARSE_INVALID)
		{
			return false;
		}
		if(result == CC_PARSE_MATCH)
		{
			return true;
		}
	}

	{
		constexpr CcTokenType tokens[] = {CC_TOKEN_CARET};
		constexpr CcBinOp binOps[] = {CC_BIN_OP_XOR};
		constexpr size_t count = CC_LEN(tokens);
		static_assert(CC_LEN(binOps) == count);

		const CcParseResult result = ccParseBinaryOperator(pBuilder, tokens, binOps, count);
		if(result == CC_PARSE_INVALID)
		{
			return false;
		}
		if(result == CC_PARSE_MATCH)
		{
			return true;
		}
	}

	{
		constexpr CcTokenType tokens[] = {CC_TOKEN_AMPERSAND};
		constexpr CcBinOp binOps[] = {CC_BIN_OP_AND};
		constexpr size_t count = CC_LEN(tokens);
		static_assert(CC_LEN(binOps) == count);

		const CcParseResult result = ccParseBinaryOperator(pBuilder, tokens, binOps, count);
		if(result == CC_PARSE_INVALID)
		{
			return false;
		}
		if(result == CC_PARSE_MATCH)
		{
			return true;
		}
	}

	{
		constexpr CcTokenType tokens[] = {CC_TOKEN_EQUAL_EQUAL, CC_TOKEN_NOT_EQUAL};
		constexpr CcBinOp binOps[] = {CC_BIN_OP_EQ, CC_BIN_OP_NEQ};
		constexpr size_t count = CC_LEN(tokens);
		static_assert(CC_LEN(binOps) == count);

		const CcParseResult result = ccParseBinaryOperator(pBuilder, tokens, binOps, count);
		if(result == CC_PARSE_INVALID)
		{
			return false;
		}
		if(result == CC_PARSE_MATCH)
		{
			return true;
		}
	}

	{
		constexpr CcTokenType tokens[] = {CC_TOKEN_LESS, CC_TOKEN_LESS_EQUAL, CC_TOKEN_GREATER, CC_TOKEN_GREATER_EQUAL};
		constexpr CcBinOp binOps[] = {CC_BIN_OP_LE, CC_BIN_OP_LEQ, CC_BIN_OP_GE, CC_BIN_OP_GEQ};
		constexpr size_t count = CC_LEN(tokens);
		static_assert(CC_LEN(binOps) == count);

		const CcParseResult result = ccParseBinaryOperator(pBuilder, tokens, binOps, count);
		if(result == CC_PARSE_INVALID)
		{
			return false;
		}
		if(result == CC_PARSE_MATCH)
		{
			return true;
		}
	}

	{
		constexpr CcTokenType tokens[] = {CC_TOKEN_LEFT_SHIFT, CC_TOKEN_RIGHT_SHIFT};
		constexpr CcBinOp binOps[] = {CC_BIN_OP_LS, CC_BIN_OP_RS};
		constexpr size_t count = CC_LEN(tokens);
		static_assert(CC_LEN(binOps) == count);

		const CcParseResult result = ccParseBinaryOperator(pBuilder, tokens, binOps, count);
		if(result == CC_PARSE_INVALID)
		{
			return false;
		}
		if(result == CC_PARSE_MATCH)
		{
			return true;
		}
	}

	{
		constexpr CcTokenType tokens[] = {CC_TOKEN_PLUS, CC_TOKEN_MINUS};
		constexpr CcBinOp binOps[] = {CC_BIN_OP_SUM, CC_BIN_OP_DIF};
		constexpr size_t count = CC_LEN(tokens);
		static_assert(CC_LEN(binOps) == count);

		const CcParseResult result = ccParseBinaryOperator(pBuilder, tokens, binOps, count);
		if(result == CC_PARSE_INVALID)
		{
			return false;
		}
		if(result == CC_PARSE_MATCH)
		{
			return true;
		}
	}

	{
		constexpr CcTokenType tokens[] = {CC_TOKEN_STAR, CC_TOKEN_SLASH, CC_TOKEN_PERCENT};
		constexpr CcBinOp binOps[] = {CC_BIN_OP_MUL, CC_BIN_OP_DIV, CC_BIN_OP_MOD};
		constexpr size_t count = CC_LEN(tokens);
		static_assert(CC_LEN(binOps) == count);

		const CcParseResult result = ccParseBinaryOperator(pBuilder, tokens, binOps, count);
		if(result == CC_PARSE_INVALID)
		{
			return false;
		}
		if(result == CC_PARSE_MATCH)
		{
			return true;
		}
	}

	if(pBuilder->tokens->count == 1 && pBuilder->tokens->tokens[0].type == CC_TOKEN_CONSTANT)
	{
		pBuilder->pTree->nodes[pBuilder->pTree->count].type = CC_NODE_CONSTANT;
		pBuilder->pTree->nodes[pBuilder->pTree->count].constant = pBuilder->tokens->tokens[0].constant;

		++pBuilder->pTree->count;

		return true;
	}

	return false;
}

bool ccParseStatement(CcTreeBuilder* const pBuilder)
{
	assert(ccAssertBuilder(pBuilder));

	if(pBuilder->tokens->tokens[0].type != CC_TOKEN_RETURN)
	{
		return false;
	}

	const CcToken* pToken = pBuilder->tokens->tokens + 1;
	while(pToken < pBuilder->tokens->tokens + pBuilder->tokens->count)
	{
		if(pToken->type == CC_TOKEN_SEMICOLON)
		{
			const bool empty = pToken == pBuilder->tokens->tokens + 1;

			if(!empty)
			{
				if(!ccParseExpression(&(CcTreeBuilder){
					.pTree = pBuilder->pTree,
					.tokens = &(CcConstTokenList){
						pBuilder->tokens->tokens + 1,
						pToken - pBuilder->tokens->tokens - 1
					}
				}))
				{
					return false;
				}
			}

			pBuilder->pTree->nodes[pBuilder->pTree->count] = (CcNode){.type = CC_NODE_RETURN, .returnNode = empty ? SIZE_MAX : pBuilder->pTree->count - 1};
			++pBuilder->pTree->count;

			pBuilder->tokens->count -= pToken - pBuilder->tokens->tokens + 1;
			pBuilder->tokens->tokens = pToken + 1;

			return true;
		}

		++pToken;
	}

	return false;
}

static void ccStoreChild(CcTreeBuilder* const pBuilder)
{
	assert(ccAssertBuilder(pBuilder));
	assert(pBuilder->lastIndex > 0);

	--pBuilder->lastIndex;
	pBuilder->pTree->children[pBuilder->lastIndex] = pBuilder->pTree->count - 1;
}

static void ccCommitChildren(CcTreeBuilder* const pBuilder, const size_t childCount)
{
	assert(ccAssertBuilder(pBuilder));
	assert(childCount > 0);

	memmove(pBuilder->pTree->children + pBuilder->childCount, pBuilder->pTree->children + pBuilder->lastIndex, childCount * sizeof(pBuilder->pTree->children));
	pBuilder->childCount += childCount;
	pBuilder->lastIndex += childCount;
}

bool ccParseFunction(CcTreeBuilder* const pBuilder)
{
	if(pBuilder->tokens->tokens[0].type != CC_TOKEN_INT)
	{
		return false;
	}

	++pBuilder->tokens->tokens;
	--pBuilder->tokens->count;
	if(pBuilder->tokens->count == 0)
	{
		return false;
	}

	if(pBuilder->tokens->tokens[0].type != CC_TOKEN_IDENTIFIER)
	{
		return false;
	}

	const CcToken* const pNameToken = pBuilder->tokens->tokens;

	++pBuilder->tokens->tokens;
	--pBuilder->tokens->count;
	if(pBuilder->tokens->count == 0)
	{
		return false;
	}

	if(pBuilder->tokens->tokens[0].type != CC_TOKEN_OPEN_PARENTHESIS)
	{
		return false;
	}

	++pBuilder->tokens->tokens;
	--pBuilder->tokens->count;
	if(pBuilder->tokens->count == 0)
	{
		return false;
	}

	if(pBuilder->tokens->tokens[0].type == CC_TOKEN_VOID)
	{
		++pBuilder->tokens->tokens;
		--pBuilder->tokens->count;
		if(pBuilder->tokens->count == 0)
		{
			return false;
		}
	}

	if(pBuilder->tokens->tokens[0].type != CC_TOKEN_CLOSE_PARENTHESIS)
	{
		return false;
	}

	++pBuilder->tokens->tokens;
	--pBuilder->tokens->count;
	if(pBuilder->tokens->count == 0)
	{
		return false;
	}

	if(pBuilder->tokens->tokens[0].type != CC_TOKEN_OPEN_BRACE)
	{
		return false;
	}

	++pBuilder->tokens->tokens;
	--pBuilder->tokens->count;
	if(pBuilder->tokens->count == 0)
	{
		return false;
	}

	size_t braceCount = 1;
	const CcToken* pToken = pBuilder->tokens->tokens;
	while(pToken < pBuilder->tokens->tokens + pBuilder->tokens->count)
	{
		braceCount += pToken->type == CC_TOKEN_OPEN_BRACE;
		braceCount -= pToken->type == CC_TOKEN_CLOSE_BRACE;

		if(braceCount == 0)
		{
			if(pToken == pBuilder->tokens->tokens)
			{
				pBuilder->pTree->nodes[pBuilder->pTree->count].type = CC_NODE_FUNCTION;
				pBuilder->pTree->nodes[pBuilder->pTree->count].function.name = pNameToken->string;
				pBuilder->pTree->nodes[pBuilder->pTree->count].function.statementsCount = 0;

				++pBuilder->pTree->count;

				++pBuilder->tokens->tokens;
				--pBuilder->tokens->count;

				return true;
			}

			const size_t tokenCount = pToken - pBuilder->tokens->tokens;

			CcTreeBuilder builder = {
				.pTree = pBuilder->pTree,
				.tokens = &(CcConstTokenList){
					pBuilder->tokens->tokens,
					tokenCount
				}
			};

			size_t statementCount = 0;
			while(builder.tokens->count > 0)
			{
				if(!ccParseStatement(&builder))
				{
					return false;
				}

				ccStoreChild(pBuilder);

				++statementCount;
			}

			if(statementCount > 0)
			{
				ccCommitChildren(pBuilder, statementCount);
			}

			pBuilder->pTree->nodes[pBuilder->pTree->count].type = CC_NODE_FUNCTION;
			pBuilder->pTree->nodes[pBuilder->pTree->count].function.name = pNameToken->string;
			pBuilder->pTree->nodes[pBuilder->pTree->count].function.statementsStart = pBuilder->childCount - 1;
			pBuilder->pTree->nodes[pBuilder->pTree->count].function.statementsCount = statementCount;

			++pBuilder->pTree->count;

			pBuilder->tokens->count -= tokenCount + 1;
			pBuilder->tokens->tokens = pToken + 1;

			return true;
		}

		++pToken;
	}

	return false;
}

bool ccParseProgram(CcTreeBuilder* const pBuilder)
{
	assert(ccAssertBuilder(pBuilder));

	size_t childCount = 0;
	while(pBuilder->tokens->count > 0)
	{
		if(!ccParseFunction(pBuilder))
		{
			return false;
		}

		ccStoreChild(pBuilder);

		++childCount;
	}

	if(childCount > 0)
	{
		ccCommitChildren(pBuilder, childCount);
	}

	pBuilder->pTree->nodes[pBuilder->pTree->count].type = CC_NODE_PROGRAM;
	pBuilder->pTree->nodes[pBuilder->pTree->count].program.childrenStart = pBuilder->childCount - 1;
	pBuilder->pTree->nodes[pBuilder->pTree->count].program.childrenCount = childCount;

	++pBuilder->pTree->count;
	
	return pBuilder->tokens->count == 0;
}

CcResult ccParse(const CcConstTokenList* const tokens, CcTree* const pTree)
{
	// Validate arguments.
	assert(tokens != nullptr);
	assert(tokens->count > 0);
	assert(tokens->tokens != nullptr);

	assert(pTree != nullptr);

	CcResult result = CC_SUCCESS;

	pTree->nodes = nullptr;
	pTree->children = nullptr;

	pTree->nodes = malloc((tokens->count + 1) * sizeof(pTree->nodes[0]));
	if(!pTree->nodes)
	{
		result = CC_ERROR_OUT_OF_MEMORY;
		goto error;
	}

	pTree->children = malloc(tokens->count * sizeof(pTree->children[0]));
	if(!pTree->children)
	{
		result = CC_ERROR_OUT_OF_MEMORY;
		goto error;
	}

	CcTreeBuilder builder = {.pTree = pTree, .tokens = &(CcConstTokenList){tokens->tokens, tokens->count}, .lastIndex = tokens->count};
	if(!ccParseProgram(&builder))
	{
		result = CC_ERROR_INVALID_ARGUMENT;
		goto error;
	}

	if(pTree->count == 0)
	{
		CC_FREE(pTree->nodes);
	}

	if(builder.childCount == 0)
	{
		CC_FREE(pTree->children);
	}

	goto end;

	error:
	CC_FREE(pTree->nodes);
	CC_FREE(pTree->children);

	end:
	return result;
}

void ccFreeTree(CcTree* const pTree)
{
	assert(pTree != nullptr);

	CC_FREE(pTree->nodes);
	CC_FREE(pTree->children);
	pTree->count = 0;
}
