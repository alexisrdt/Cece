#ifndef CECE_CECE_H
#define CECE_CECE_H

#include <stdio.h>

#include "cece/arguments.h"
#include "cece/lex.h"
#include "cece/memory.h"
#include "cece/result.h"
#include "cece/tree.h"

/*
 * Print usage.
 *
 * Parameters:
 * - file: File to output the usage.
 */
void ccPrintUsage(FILE* file);

/*
 * Read all the contents of a file in text mode.
 *
 * Parameters:
 * - path: Path of the file.
 * - pString: A pointer to a string where to store the contents of the file.
 *
 * Returns:
 * - CC_SUCCESS on success.
 * - CC_ERROR_FILE_NOT_FOUND if the file could not be opened.
 * - CC_ERROR_OUT_OF_MEMORY if the file is too large.
 * - CC_ERROR_UNKNOWN otherwise.
 */
CcResult ccReadFile(const char* path, CcString* pString);

/*
 * Compile a source file.
 *
 * Parameters:
 * - pOptions: A pointer to the options to use for compilation.
 *
 * Returns:
 * - CC_SUCCESS on success.
 * - CC_ERROR_FILE_NOT_FOUND if the file could not be opened.
 * - CC_ERROR_OUT_OF_MEMORY if the code is too large.
 * - CC_ERROR_UNKNOWN otherwise.
 */
CcResult ccCompile(const CcOptions* pOptions);

#endif
