#ifndef CECE_TREE_H
#define CECE_TREE_H

#include <stddef.h>

#include "cece/lex.h"
#include "cece/result.h"

typedef enum CcNodeType
{
	CC_NODE_PROGRAM,
	CC_NODE_FUNCTION,
	CC_NODE_RETURN,
	CC_NODE_BIN_OP,
	CC_NODE_CONSTANT
} CcNodeType;

typedef struct CcNode CcNode;

typedef struct CcProgramNode
{
	size_t childrenStart;
	size_t childrenCount;
} CcProgramNode;

typedef struct CcFunctionNode
{
	CcStringView name;

	size_t statementsStart;
	size_t statementsCount;
} CcFunctionNode;

typedef size_t CcReturnNode;

typedef CcConstant CcConstantNode;

typedef enum CcBinOp
{
	CC_BIN_OP_SUM,
	CC_BIN_OP_DIF,
	CC_BIN_OP_MUL,
	CC_BIN_OP_DIV,
	CC_BIN_OP_MOD,
	CC_BIN_OP_LS,
	CC_BIN_OP_RS,
	CC_BIN_OP_LE,
	CC_BIN_OP_LEQ,
	CC_BIN_OP_GE,
	CC_BIN_OP_GEQ,
	CC_BIN_OP_EQ,
	CC_BIN_OP_NEQ,
	CC_BIN_OP_AND,
	CC_BIN_OP_XOR,
	CC_BIN_OP_OR,
	CC_BIN_OP_LAND,
	CC_BIN_OP_LOR
} CcBinOp;

typedef struct CcBinOpNode
{
	CcBinOp op;

	size_t leftNode;
	size_t rightNode;
} CcBinOpNode;

typedef struct CcNode
{
	CcNodeType type;

	union
	{
		CcProgramNode program;
		CcFunctionNode function;
		CcReturnNode returnNode;
		CcBinOpNode binOpNode;
		CcConstantNode constant;
	};
} CcNode;

/*
 * An abstract syntax tree.
 *
 * Fields:
 * - nodes: The nodes of the tree.
 * - children: Children nodes.
 * - count: Number of nodes.
 */
typedef struct CcTree
{
	CcNode* nodes;
	size_t* children;
	size_t count;
} CcTree;

/*
 * A temporary structure holding objects necessary to build a tree but useless to use it.
 *
 * Fields:
 * - pTree: A pointer to the tree to build.
 * - pTokenList: A pointer to the token list to parse.
 * - indexCount: The number of indices in the tree's indices buffer.
 * - lastIndex: The last index stored at the end of the indices buffer.
 */
typedef struct CcTreeBuilder
{
	CcTree* pTree;
	CcConstTokenList* tokens;

	size_t childCount;
	size_t lastIndex;
} CcTreeBuilder;

typedef enum CcDirection
{
	CC_DIRECTION_FORWARD = 1,
	CC_DIRECTION_BACKWARD = -1
} CcDirection;

bool ccSkipParentheses(size_t* pTokenIndex, const CcConstTokenList* tokens, CcDirection direction);

bool ccParseExpression(CcTreeBuilder* pBuilder);

bool ccParseStatement(CcTreeBuilder* pBuilder);

bool ccParseFunction(CcTreeBuilder* pBuilder);

bool ccParseProgram(CcTreeBuilder* pBuilder);

CcResult ccParse(const CcConstTokenList* tokens, CcTree* pTree);

void ccFreeTree(CcTree* pTree);

#endif
