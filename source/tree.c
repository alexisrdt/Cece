#include "cece/tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cece/log.h"
#include "cece/memory.h"

CcResult ccParse(const CcConstTokenList* pTokenList, CcTree* pTree)
{
	pTree->buffer = pTokenList->buffer;

	pTree->nodes = malloc((pTokenList->count + 1) * sizeof(pTree->nodes[0]));
	if(!pTree->nodes)
	{
		CC_LOG("Failed to allocate memory.");
		return CC_ERROR_OUT_OF_MEMORY;
	}

	pTree->indices = malloc(pTokenList->count * sizeof(pTree->indices[0]));
	if(!pTree->indices)
	{
		CC_LOG("Failed to allocate memory.");
		CC_FREE(pTree->nodes)
		return CC_ERROR_OUT_OF_MEMORY;
	}

	pTree->count = 0;
	size_t indexCount = 0;

	for(size_t i = 0; i < pTokenList->count; ++i)
	{
		if(
			pTokenList->count >= 7 &&
			pTokenList->count - i > 5 &&
			(CC_TOKEN_VOID <= pTokenList->tokens[i].type && pTokenList->tokens[i].type <= CC_TOKEN_DOUBLE) &&
			pTokenList->tokens[i + 1].type == CC_TOKEN_IDENTIFIER &&
			pTokenList->tokens[i + 2].type == CC_TOKEN_OPEN_PARENTHESIS &&
			pTokenList->tokens[i + 3].type == CC_TOKEN_VOID &&
			pTokenList->tokens[i + 4].type == CC_TOKEN_CLOSE_PARENTHESIS &&
			pTokenList->tokens[i + 5].type == CC_TOKEN_OPEN_BRACE
		)
		{
			const size_t nameIndex = i + 1;

			i += 6;

			size_t returnNode = SIZE_MAX;

			if(pTokenList->tokens[i].type == CC_TOKEN_RETURN && pTokenList->tokens[i + 1].type == CC_TOKEN_LITERAL && pTokenList->tokens[i + 2].type == CC_TOKEN_SEMICOLON)
			{

				pTree->nodes[pTree->count].type = CC_NODE_CONSTANT;
				pTree->nodes[pTree->count].constant.value = pTokenList->tokens[i + 1].intLiteral;
				++pTree->count;

				pTree->nodes[pTree->count].type = CC_NODE_RETURN;
				pTree->nodes[pTree->count].returnNode.valueNode = pTree->count - 1;
				returnNode = pTree->count;
				++pTree->count;

				i += 3;
			}

			while(i < pTokenList->count && pTokenList->tokens[i].type != CC_TOKEN_CLOSE_BRACE)
			{
				++i;
			}

			pTree->nodes[pTree->count].type = CC_NODE_FUNCTION;

			pTree->nodes[pTree->count].function.nameStart = pTokenList->tokens[nameIndex].offset;
			pTree->nodes[pTree->count].function.nameLength = pTokenList->tokens[nameIndex].length;

			pTree->nodes[pTree->count].function.returnNode = returnNode;

			pTree->indices[pTokenList->count - 1 - indexCount] = pTree->count;
			++indexCount;
			++pTree->count;
		}
	}

	pTree->nodes[pTree->count].type = CC_NODE_PROGRAM;

	memmove(pTree->indices, pTree->indices + pTokenList->count - indexCount, indexCount * sizeof(pTree->indices[0]));
	pTree->nodes[pTree->count].program.childrenStart = indexCount - 1;
	pTree->nodes[pTree->count].program.childrenCount = indexCount;

	++pTree->count;

	CcNode* const newNodes = realloc(pTree->nodes, pTree->count * sizeof(pTree->nodes[0]));
	if(!newNodes)
	{
		CC_LOG("Failed to allocate memory.");
		CC_FREE(pTree->indices)
		CC_FREE(pTree->nodes)
		return CC_ERROR_OUT_OF_MEMORY;
	}
	pTree->nodes = newNodes;

	if(indexCount == 0)
	{
		CC_FREE(pTree->indices)
		pTree->indices = NULL;
	}
	else
	{
		size_t* const newIndices = realloc(pTree->indices, indexCount * sizeof(pTree->indices[0]));
		if(!newIndices)
		{
			CC_LOG("Failed to allocate memory.");
			CC_FREE(pTree->nodes)
			return CC_ERROR_OUT_OF_MEMORY;
		}
		pTree->indices = newIndices;
	}

	return CC_SUCCESS;
}

static void ccPrintNode(const CcConstTree* pTree, size_t index, size_t depth)
{
	for(size_t i = 0; i < depth; ++i)
	{
		printf("  ");
	}
	printf("(%zu) ", index);

	switch(pTree->nodes[index].type)
	{
		case CC_NODE_PROGRAM:
			printf("Program\n");

			for(size_t i = 0; i < pTree->nodes[index].program.childrenCount; ++i)
			{
				ccPrintNode(pTree, pTree->indices[pTree->nodes[index].program.childrenStart - i], depth + 1);
			}

			break;

		case CC_NODE_FUNCTION:
			printf("Function %.*s\n", (int)pTree->nodes[index].function.nameLength, pTree->buffer + pTree->nodes[index].function.nameStart);

			if(pTree->nodes[index].function.returnNode != SIZE_MAX)
			{
				ccPrintNode(pTree, pTree->nodes[index].function.returnNode, depth + 1);
			}

			break;

		case CC_NODE_RETURN:
			printf("Return\n");
			ccPrintNode(pTree, pTree->nodes[index].returnNode.valueNode, depth + 1);
			break;

		case CC_NODE_CONSTANT:
			printf("Constant %d\n", pTree->nodes[index].constant.value);
			break;
	}
}

void ccPrintTree(const CcConstTree* pTree)
{
	ccPrintNode(pTree, pTree->count - 1, 0);
}
