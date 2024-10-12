#include "cece/memory.h"

size_t ccFind(const void* arrayVoid, size_t count, size_t size, const void* pValueVoid, CcCompare compare)
{
	const char* const array = arrayVoid;

	size_t index;
	for(index = 0; index < count; ++index)
	{
		if(compare(array + index * size, pValueVoid) == 0)
		{
			break;
		}
	}

	return index;
}
