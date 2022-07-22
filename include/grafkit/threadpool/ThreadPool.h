#pragma once

#include "BoundedMpmcQueue.h"
#include "MemoryPoolAllocator.h"
#include "ThreadPlatform.h"

#include <cstddef>
#include <memory>
#include <thread>
#include <vector>

namespace JobSystem
{
	namespace Internal
	{
		struct Job;
		struct Worker;

		typedef void (*JobFunction)(Job *, void *);

		typedef BoundedMpmcQueue<Job *> JobQueue;

		constexpr size_t cachelineSize = 64;
		typedef char CachelinePadType[cachelineSize];

		/**
		 * Threadpool implementation with job stealing
		 * Impl. Based on Molecoolar Matters
		 */
		class ThreadPool
		{
			friend struct Worker;

		public:
			static const size_t maxJobCount = 4096;

			explicit ThreadPool(size_t numThreads);
			virtual ~ThreadPool();

			ThreadPool(const ThreadPool &) = delete;
			ThreadPool & operator=(const ThreadPool &) = delete;

			size_t NumWorkers() const { return mNumWorkers; }

			Job * CreateJob(JobFunction function, void * data);
			Job * CreateJobAsChild(Job * parent, JobFunction function, void * data);

			void Schedule(Job * job);
			void Wait(Job * job);

		protected:
			Job * AllocateJob();
			void Deallocate(Job * job);
			void Steal(JobQueue *& stolenQueue);

			Worker * FindWorker();

			Job * GetJob();

			void Execute(Job * job);
			void Finish(Job * job);

			static void Yield() NOEXCEPT;

			bool HasJobCompleted(const Job * job);

		private:
			size_t mNumWorkers;
			std::vector<std::unique_ptr<Worker>> mWorkers;
			std::unique_ptr<Worker> mMainWorker;

			MemoryPoolAllocator mAllocator;

			std::thread::id mainThreadId;

			std::vector<std::thread> mThreads;
		};

		struct Worker
		{
			JobQueue mQueue = { ThreadPool::maxJobCount };
			std::atomic_bool mIsTerminated = false;
		};

		struct Job
		{
			JobFunction function;
			Job * parent;
			void * data;
			std::atomic_char32_t unfinishedJobs;
			CachelinePadType padding;
		};
	} // namespace Internal

} // namespace JobSystem
