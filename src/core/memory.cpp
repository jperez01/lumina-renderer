#include "memory.h"
#include <stdlib.h>
#include <new>

LUMINA_NAMESPACE_BEGIN

void* AllocAligned(size_t size) {
#if defined(LUMINA_HAVE_ALIGNED_MALLOC)
	return _aligned_malloc(size, LUMINA_L1_CACHE_SIZE);
#elif defined(LUMINA_HAVE_POSIX_MEMALIGN)
	void* ptr;
	if (posix_memalign(&ptr, LUMINA_L1_CACHE_SIZE, size) != 0)
		ptr = nullptr;
	return ptr;
#else
	return malloc(size);
#endif
}

void FreeAligned(void* ptr)
{
	if (!ptr) return;

#if defined(LUMINA_HAVE_ALIGNED_MALLOC)
	_aligned_free(ptr);
#else
	operator delete(ptr, std::align_val_t(LUMINA_L1_CACHE_SIZE));
#endif
}

LUMINA_NAMESPACE_END