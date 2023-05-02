#include <grafkit/ThreadPool/BoundedMpmcQueue.h>
//
#include <gtest/gtest.h>

TEST(StealingBoundedQueue, Overflow)
{
	JobSystem::BoundedMpmcQueue<int> queue(16);
	for (int i = 0; i < static_cast<int>(queue.Capacity()); ++i)
	{
		ASSERT_TRUE(queue.Push(i));
	}

	ASSERT_FALSE(queue.Push(-1));
}

TEST(StealingBoundedQueue, Undeflow)
{
	JobSystem::BoundedMpmcQueue<int> queue(16);
	for (int i = 0; i < static_cast<int>(queue.Capacity()); ++i)
	{
		ASSERT_TRUE(queue.Push(i)) << i;
	}

	int dummy = 0;
	for (int i = 0; i < static_cast<int>(queue.Capacity()); ++i)
	{
		ASSERT_TRUE(queue.Pop(dummy)) << i;
	}

	ASSERT_FALSE(queue.Pop(dummy));
}
