#pragma once

#include <atomic>
#include <cstddef>

namespace JobSystem
{
	/**
	 * Bounded pool allocator implementation
	 */
	class MemoryPoolAllocator
	{
	public:
		MemoryPoolAllocator(size_t numElements, size_t elementSize, size_t alignment = 16);

		MemoryPoolAllocator(const MemoryPoolAllocator& alloc) = delete;
		MemoryPoolAllocator& operator=(const MemoryPoolAllocator& rhs) = delete;
		MemoryPoolAllocator(MemoryPoolAllocator&& alloc) = delete;
		MemoryPoolAllocator& operator=(MemoryPoolAllocator&& rhs) = delete;

		~MemoryPoolAllocator();

		void* Allocate() noexcept;
		void Deallocate(void* block) noexcept;
		size_t ElementSize() const { return mElementSize; }
		size_t Capacity() const { return mPoolSize; }

	private:
		bool AllocatePool(size_t numElements, size_t elementSize, size_t alignment);
		void ReleasePool();

		size_t mPoolSize = 0;
		size_t mElementSize = 0;
		size_t mAlignment = 0;

		void* mPool = nullptr;
		std::atomic<void**> mHead;
	};

} // namespace JobSystem
