#ifndef CECE_TREE_H
#define CECE_TREE_H

#include <stddef.h>

#include "cece/lex.h"
#include "cece/result.h"

typedef enum CcNodeType
{
	CC_NODE_PROGRAM,
	CC_NODE_FUNCTION,
	CC_NODE_CONSTANT,
	CC_NODE_RETURN
} CcNodeType;

typedef struct CcNode CcNode;

typedef struct CcProgramNode
{
	size_t childrenStart;
	size_t childrenCount;
} CcProgramNode;

typedef struct CcFunctionNode
{
	size_t nameStart;
	size_t nameLength;

	size_t returnNode;
} CcFunctionNode;

typedef struct CcReturnNode
{
	size_t valueNode;
} CcReturnNode;

typedef struct CcConstantNode
{
	int value;
} CcConstantNode;

typedef struct CcNode
{
	CcNodeType type;

	union
	{
		CcProgramNode program;
		CcFunctionNode function;
		CcReturnNode returnNode;
		CcConstantNode constant;
	};
} CcNode;

typedef struct CcTree
{
	const char* buffer;
	CcNode* nodes;
	size_t* indices;
	size_t count;
} CcTree;

CcResult ccParse(const CcConstTokenList* pTokenList, CcTree* pTree);

typedef struct CcConstTree
{
	const char* buffer;
	const CcNode* nodes;
	const size_t* indices;
	size_t count;
} CcConstTree;

void ccPrintTree(const CcConstTree* pTree);

#endif
