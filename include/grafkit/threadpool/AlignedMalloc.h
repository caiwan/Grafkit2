#pragma once
//#ifndef THREADPOOL_ALIGNED_ALLOC_H
//#define THREADPOOL_ALIGNED_ALLOC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

	void* AlignedMalloc(size_t size, size_t alignment);
	void AlignedFree(void* pointer);

#ifdef __cplusplus
}
#endif

//#endif THREADPOOL_ALIGNED_ALLOC_H
