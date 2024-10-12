#ifndef CECE_MEMORY_H
#define CECE_MEMORY_H

#include <stdlib.h>

/*
 * Get the number of elements in an array.
 *
 * Parameters:
 * - array: An array.
 * 
 * Returns:
 * The number of elements in the array.
 */
#define CC_LEN(array) \
(sizeof(array) / sizeof((array)[0]))

/*
 * Free a pointer and set it to NULL.
 * Cece only uses this macro to free pointers and avoid dangling pointers.
 * Therefore, any pointer returned by a Cece function is either NULL or a valid pointer.
 *
 * Parameters:
 * - pointer: A valid pointer to dynamically allocated memory.
 */
#define CC_FREE(pointer) \
free(pointer); \
pointer = NULL;

/*
 * Comparison function.
 *
 * Parameters:
 * - pFirstVoid: The first value to compare.
 * - pSecondVoid: The second value to compare.
 * 
 * Returns:
 * - 0 if the values are equal.
 * - A negative value if the first value is less than the second value.
 * - A positive value if the first value is greater than the second value.
 */
typedef int (*CcCompare)(const void* pFirstVoid, const void* pSecondVoid);

/*
 * Find the index of a value in an array.
 * If the array is sorted, prefer using bsearch.
 *
 * Parameters:
 * - arrayVoid: The array to search.
 * - count: The number of elements in the array.
 * - size: The size of each element in the array.
 * - pValueVoid: A pointer to the value to find.
 * - compare: The comparison function.
 * 
 * Returns:
 * - The index of the value in the array.
 * - count if the value is not found.
 */
size_t ccFind(const void* arrayVoid, size_t count, size_t size, const void* pValueVoid, CcCompare compare);

#endif
