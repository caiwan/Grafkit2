#include <atomic>
#include <thread>
//
#include <grafkit/ThreadPool/MemoryPoolAllocator.h>
//
#include <gtest/gtest.h>
class MemoryPool : public testing::Test
{
};

TestF(MemoryPool, allocate)
{
	JobSystem::MemoryPoolAllocator allocator(256, 16, 16);
	void* memory = allocator.Allocate();
	ASSERT_TRUE(memory);
	allocator.Deallocate(memory);
}

TestF(MemoryPool, overflow)
{
	JobSystem::MemoryPoolAllocator allocator(256, 16, 16);

	for (size_t i = 0; i < 256; ++i)
	{
		void* memory = allocator.Allocate();
		ASSERT_TRUE(memory);
	}

	void* memory = allocator.Allocate();
	ASSERT_FALSE(memory);
}

TestF(MemoryPool, raceConditions)
{

	// given
	unsigned numThreads = std::thread::hardware_concurrency();
	numThreads = numThreads ? numThreads : 2;

	std::vector<std::thread> threads;
	threads.reserve(numThreads);

	JobSystem::MemoryPoolAllocator allocator(256, 16, 16);

	// when
	for (size_t i = 0; i < numThreads; i++)
	{
		threads.emplace_back([&]() {
			for (int i = 0; i < 100000; ++i)
			{
				void* memory = allocator.Allocate();
				ASSERT_TRUE(memory);
				allocator.Deallocate(memory);
				// TODO this particulalry does nothing relevant
			}
		});
	}

	for (auto& thread : threads)
	{
		thread.join();
	}
}
