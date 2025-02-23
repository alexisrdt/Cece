#ifndef CECE_MEMORY_H
#define CECE_MEMORY_H

#include <stdint.h>
#include <stdlib.h>

/*
 * Get the minimum of two values.
 *
 * This is implemented as a macro to be able to use it for constant expressions (and possibly preprocessor).
 *
 * Parameters:
 * - first: The first value.
 * - second: The second value.
 *
 * Returns:
 * The minimum of the two values.
 */
#define CC_MIN(first, second) \
((first) <= (second) ? (first) : (second))

// Maximum size of an object.
constexpr size_t ccSizeMax = CC_MIN(PTRDIFF_MAX, SIZE_MAX);

/*
 * Get the number of elements in an array.
 *
 * This is implemented as a macro since arrays decay to pointers when passed to functions.
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
 * Free a pointer and set it to nullptr.
 * Cece only uses this macro to free pointers and avoid dangling pointers.
 * Therefore, any pointer returned by a Cece function is either nullptr or a valid pointer.
 *
 * Parameters:
 * - pointer: A valid pointer to dynamically allocated memory.
 */
#define CC_FREE(pointer) \
do \
{ \
	free(pointer); \
	(pointer) = nullptr; \
} while(0)

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
 * Find a value in an array by exploring it linearly.
 *
 * This generic function preserves const-qualification of array elements.
 * It also does not modify the array elements, even if they are not constant.
 *
 * The behavior is undefined if the passed array is not of pointer or array type or if it is nullptr.
 *
 * Parameters:
 * - pValueVoid: A pointer to the value to find.
 * - arrayVoid: The array to search.
 * - count: The number of elements in the array.
 * - size: The size of each element in the array.
 * - compare: The comparison function.
 *
 * Returns:
 * - A pointer to the value in the array.
 * - nullptr if the value is not found.
 */
void* ccFind(const void* pValueVoid, const void* arrayVoid, size_t count, size_t size, CcCompare compare);

#define ccFind(pValueVoid, arrayVoid, count, size, compare) _Generic( \
	true ? (arrayVoid) : (void*)nullptr, \
	const void*: (const void*)ccFind((pValueVoid), (arrayVoid), (count), (size), (compare)), \
	default: ccFind((pValueVoid), (arrayVoid), (count), (size), (compare)) \
)

#endif
