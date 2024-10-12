#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cece/cece.h"

int main(int argumentCount, char** arguments)
{
	// Yes, this is just to be able to pass constant arguments to ccParseArguments.
	// Yes, it's ugly.
	// But it feels better to have a clean interface for ccParseArguments and keep main prototype like this.
	const char** argumentsConst = NULL;
	if(argumentCount > 0)
	{
		const size_t argumentsSize = argumentCount * sizeof(argumentsConst[0]);
		argumentsConst = malloc(argumentsSize);
		if(!argumentsConst)
		{
			return EXIT_FAILURE;
		}
		memcpy(argumentsConst, arguments, argumentsSize);
	}

	CcOptions options;
	if(ccParseArguments(argumentCount, argumentsConst, &options) != CC_SUCCESS)
	{
		CC_FREE(argumentsConst);
		return EXIT_FAILURE;
	}
	CC_FREE(argumentsConst);

	CcString string;
	if(ccReadFile(options.input, &string) != CC_SUCCESS)
	{
		CC_FREE(options.input)
		CC_FREE(options.output)

		return EXIT_FAILURE;
	}

	CC_FREE(options.input)

	CcTokenList tokenList;
	if(ccLex(&(const CcConstString){.string = string.string, .length = string.length}, &tokenList) != CC_SUCCESS)
	{
		CC_FREE(options.output)
		CC_FREE(string.string)

		return EXIT_FAILURE;
	}

	CC_FREE(string.string)

	for(size_t tokenIndex = 0; tokenIndex < tokenList.count; ++tokenIndex)
	{
		printf("Token %zu: %s", tokenIndex, ccTokenTypeString(tokenList.tokens[tokenIndex].type));

		if(tokenList.tokens[tokenIndex].type == CC_TOKEN_LITERAL)
		{
			printf(" %d", tokenList.tokens[tokenIndex].intLiteral);
		}

		else if(tokenList.tokens[tokenIndex].type == CC_TOKEN_IDENTIFIER)
		{
			printf(" %.*s", (int)tokenList.tokens[tokenIndex].length, tokenList.buffer + tokenList.tokens[tokenIndex].offset);
		}

		else if(tokenList.tokens[tokenIndex].type == CC_TOKEN_STRING)
		{
			printf(" %.*s", (int)tokenList.tokens[tokenIndex].length, tokenList.buffer + tokenList.tokens[tokenIndex].offset);
		}

		printf("\n");
	}

	CcTree tree;
	if(ccParse(&(const CcConstTokenList){.buffer = tokenList.buffer, .count = tokenList.count, .tokens = tokenList.tokens}, &tree) != CC_SUCCESS)
	{
		CC_FREE(options.output)
		CC_FREE(tokenList.buffer)
		CC_FREE(tokenList.tokens)

		return EXIT_FAILURE;
	}

	CC_FREE(tokenList.tokens)

	printf("\n");
	ccPrintTree(&(const CcConstTree){
		.buffer = tree.buffer,
		.nodes = tree.nodes,
		.indices = tree.indices,
		.count = tree.count
	});

	FILE* const file = fopen(options.output, "w");
	if(!file)
	{
		CC_FREE(options.output)
		CC_FREE(tokenList.buffer)
		CC_FREE(tree.nodes)
		CC_FREE(tree.indices)

		return EXIT_FAILURE;
	}
	CC_FREE(options.output)

	for(size_t i = 0; i < tree.nodes[tree.count - 1].program.childrenCount; ++i)
	{
		const size_t index = tree.indices[tree.nodes[tree.count - 1].program.childrenStart - i];
		if(tree.nodes[index].type == CC_NODE_FUNCTION)
		{
			if(tree.nodes[index].function.nameLength == 4 && strncmp(tree.buffer + tree.nodes[index].function.nameStart, "main" , 4) == 0)
			{
				fprintf(
					file,
					"global %.*s\n",
					(int)tree.nodes[index].function.nameLength,
					tree.buffer + tree.nodes[index].function.nameStart
				);
			}

			fprintf(
				file,
				"%.*s:\n"
				"\tmov eax, %d\n"
				"\tret\n",
				(int)tree.nodes[index].function.nameLength,
				tree.buffer + tree.nodes[index].function.nameStart,
				tree.nodes[tree.nodes[index].function.returnNode].constant.value
			);
		}
	}

	fclose(file);

	CC_FREE(tokenList.buffer)

	CC_FREE(tree.nodes)
	CC_FREE(tree.indices)

	return EXIT_SUCCESS;
}
