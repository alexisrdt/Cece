#include "cece/memory.h"

#include <assert.h>

#undef ccFind

void* ccFind(const void* const pValueVoid, const void* const arrayVoid, const size_t count, const size_t size, const CcCompare compare)
{
	assert(pValueVoid != nullptr);
	assert(arrayVoid != nullptr);
	assert(size > 0);
	assert(count <= ccSizeMax / size);
	assert(compare != nullptr);

	for(const char* pElement = arrayVoid, * const end = pElement + count * size; pElement < end; pElement += size)
	{
		if(compare(pValueVoid, pElement) == 0)
		{
			return (void*)pElement;
		}
	}

	return nullptr;
}
