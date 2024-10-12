#ifndef CECE_CECE_H
#define CECE_CECE_H

#include "cece/arguments.h"
#include "cece/lex.h"
#include "cece/log.h"
#include "cece/memory.h"
#include "cece/pre.h"
#include "cece/result.h"
#include "cece/tree.h"

CcResult ccReadFile(const char* filePath, CcString* pString);

#endif
