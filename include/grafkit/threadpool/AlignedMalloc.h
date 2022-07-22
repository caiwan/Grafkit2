#pragma once
//#ifndef THREADPOOL_ALIGNED_ALLOC_H
//#define THREADPOOL_ALIGNED_ALLOC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

	void * aligned_malloc(size_t size, size_t alignment);
	void aligned_free(void * pointer);

#ifdef __cplusplus
}
#endif

//#endif THREADPOOL_ALIGNED_ALLOC_H
