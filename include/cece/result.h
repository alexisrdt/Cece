#ifndef CECE_RESULT_H
#define CECE_RESULT_H

// Result type returned by Cece functions.
typedef enum CcResult
{
	CC_SUCCESS,
	CC_ERROR_INVALID_ARGUMENT,
	CC_ERROR_FILE_NOT_FOUND,
	CC_ERROR_OUT_OF_MEMORY,
	CC_ERROR_UNKNOWN
} CcResult;

#endif
