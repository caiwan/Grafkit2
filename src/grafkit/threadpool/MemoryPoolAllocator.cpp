#include <cassert>
#include <cstdlib>
#include <exception>
//
#include <grafkit/threadpool/AlignedMalloc.h>
#include <grafkit/threadpool/MemoryPoolAllocator.h>

// Thread-safeness
// https://stackoverflow.com/questions/12467514/simple-thread-safe-and-fast-memory-pool-implementation
// https://codereview.stackexchange.com/questions/216762/creating-a-lock-free-memory-pool-using-c11-featuresz

JobSystem::MemoryPoolAllocator::MemoryPoolAllocator(size_t numElements, size_t elementSize, size_t alignment)
{
	mHead.store(nullptr);
	const bool result = AllocatePool(numElements, elementSize, alignment);
	if (!result)
		throw std::bad_alloc();
}

JobSystem::MemoryPoolAllocator::~MemoryPoolAllocator() { ReleasePool(); }

bool JobSystem::MemoryPoolAllocator::AllocatePool(const size_t numElements, const size_t elementSize, const size_t alignment)
{
	// guarantees that this pool was not allocated before
	assert(mPool == nullptr);

	mElementSize = elementSize;
	mAlignment = alignment;

	// element size must be at least the same size of void*
	assert(mElementSize >= sizeof(void*));
	// element size must be a multiple of the alignment requirement
	assert(mElementSize % mAlignment == 0);
	// alignment must be a power of two
	assert((mAlignment & (mAlignment - 1)) == 0);

	// --- allocate
	mPoolSize = (mElementSize * numElements) /*+ alignment*/;
	mPool = AlignedMalloc(mElementSize * numElements, mAlignment);
	if (mPool == nullptr)
		return false;

	// ---
	// cast the void* to void**, so the memory is 'interpret' as a buffer of void*
	void** freeMemoryList = static_cast<void**>(mPool);
	mHead.store(freeMemoryList);
	// get the address of the end of the memory block
	const auto endAddress = reinterpret_cast<uintptr_t>(freeMemoryList) + (elementSize * numElements);

	for (size_t element = 0; element < numElements; ++element)
	{
		// get the addresses of the current chunck and the next one
		const auto currentAddress = reinterpret_cast<uintptr_t>(freeMemoryList) + element * mElementSize;
		//    const auto currAddress = static_cast<char*>(freeMemoryList) + element * mElementSize;
		const auto nextAddress = currentAddress + mElementSize;
		// cast the address of the current chunk to a void**
		void** currentMemory = reinterpret_cast<void**>(currentAddress);
		if (nextAddress >= endAddress)
		{ // last chunk
			*currentMemory = nullptr;
		}
		else
		{
			*currentMemory = reinterpret_cast<void*>(nextAddress); // NOLINT
		}
	}
	return true;
}

void* JobSystem::MemoryPoolAllocator::Allocate() noexcept
{
	assert(mPool);

	void** head = nullptr;
	void** next = nullptr;
	void* block = nullptr;

	do
	{
		head = mHead.load(std::memory_order_relaxed);
		// Pool is full
		if (!head)
			return nullptr;
		// Take a block out and tyr to move the head
		block = reinterpret_cast<void*>(head);
		next = static_cast<void**>(*head);
		// If the head was changed by another thread, do it again
	} while (!mHead.compare_exchange_weak(head, next, std::memory_order_relaxed));

	return block;
}

void JobSystem::MemoryPoolAllocator::Deallocate(void* block) noexcept
{
	if (block == nullptr)
	{
		return;
	}

	// TODO disallow taking pointed rom outside the valid address space

	assert(mPool);

	void** head = nullptr;
	void** prev = nullptr;
	void** returningBlock = nullptr; // where the returning block will point at as a next block (to restore freelist)

	while (true)
	{
		head = mHead.load(std::memory_order_relaxed);

		if (head == nullptr)
		{
			returningBlock = nullptr;
			// the free list was empty set the head to the returning block then mark as the last one
			prev = reinterpret_cast<void**>(block); // NOLINT
		}
		else
		{
			// otherwise put back to the queue as is
			returningBlock = head;
			prev = reinterpret_cast<void**>(block); // NOLINT
		}

		// Only mark when change could be committed, otherwise try it again
		if (mHead.compare_exchange_weak(head, prev, std::memory_order_relaxed))
		{
			*prev = returningBlock;
			return;
		}
	}
}

void JobSystem::MemoryPoolAllocator::ReleasePool()
{
	AlignedFree(mPool);
	mPool = mHead = nullptr;
}
