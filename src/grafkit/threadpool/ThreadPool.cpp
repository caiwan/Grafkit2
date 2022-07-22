#include <grafkit/threadpool/ThreadPool.h>

using namespace JobSystem::Internal;

thread_local Worker * localWorker = nullptr;

ThreadPool::ThreadPool(const size_t numThreads) :
    mNumWorkers(numThreads), mAllocator(numThreads * ThreadPool::maxJobCount, sizeof(Job), 16), mainThreadId(std::this_thread::get_id())
{
	assert(mNumWorkers);

	for (size_t i = 0; i < mNumWorkers; ++i) { mWorkers.push_back(std::make_unique<Worker>()); }

	for (size_t i = 0; i < numThreads; ++i)
	{
		mThreads.emplace_back(
		    [this](Worker * worker) {
			    localWorker = worker;
			    while (!localWorker->mIsTerminated)
			    {
				    Job * job = GetJob();
				    if (job) { Execute(job); }
			    }
		    },
		    mWorkers[i].get());
	}

	mMainWorker = std::make_unique<Worker>();
}

ThreadPool::~ThreadPool()
{
	for (auto & worker : mWorkers) worker->mIsTerminated = true;
	for (auto & thread : mThreads) thread.join();
}

Job * ThreadPool::CreateJob(JobFunction function, void * data)
{
	Job * job = AllocateJob();
	job->function = function;
	job->parent = nullptr;
	job->data = data;
	job->unfinishedJobs = 1;

	return job;
}

Job * ThreadPool::CreateJobAsChild(Job * parent, JobFunction function, void * data)
{
	parent->unfinishedJobs++;

	Job * job = AllocateJob();
	job->function = function;
	job->parent = parent;
	job->data = data;
	job->unfinishedJobs = 1;

	return job;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc30-c"
void ThreadPool::Schedule(Job * job)
{
	assert(job);
	const auto randomIndex = static_cast<size_t>(std::rand()) % (mNumWorkers);
	mWorkers[randomIndex]->mQueue.Push(job);
}

Job * ThreadPool::AllocateJob()
{
	Job * job = reinterpret_cast<Job *>(mAllocator.Allocate());
	assert(job);
	return job;
}

void ThreadPool::Deallocate(Job * job) { mAllocator.Deallocate(job); }

void ThreadPool::Steal(JobQueue *& stolenQueue)
{
	auto randomIndex = static_cast<size_t>(std::rand()) % (mNumWorkers);
	stolenQueue = &mWorkers[randomIndex]->mQueue;
}

#pragma clang diagnostic pop

void ThreadPool::Wait(Job * job)
{
	// wait until the job has completed. in the meantime, work on any other pJob.
	while (!HasJobCompleted(job))
	{
		Job * nextJob = GetJob();
		if (nextJob) { Execute(nextJob); }
	}
}

Worker * ThreadPool::FindWorker()
{
	const auto threadId = std::this_thread::get_id();
	if (threadId == mainThreadId) { return mMainWorker.get(); }

	return localWorker;
}

Job * ThreadPool::GetJob()
{
	Worker * worker = FindWorker();
	if (!worker) return nullptr; // Should not happen

	Job * job = nullptr;
	const bool hasJob = worker->mQueue.Pop(job);
	if (!hasJob)
	{
		// this is not a valid job because our own queue is empty, so try stealing from some other queue

		JobQueue * stolenQueue = nullptr;
		Steal(stolenQueue);

		// avoid steal from ourselves
		if (stolenQueue == &worker->mQueue)
		{
			Yield();
			return nullptr;
		}

		Job * stolenJob = nullptr;
		const bool hasStolenJob = stolenQueue->Pop(stolenJob);
		if (!hasStolenJob)
		{
			// we couldn't steal a job from the other queue either, so we just yield our time slice for now
			Yield();
			return nullptr;
		}

		return stolenJob;
	}

	return job;
}

void ThreadPool::Execute(Job * job)
{
	(job->function)(job, job->data);
	Finish(job);
}

void ThreadPool::Finish(Job * job)
{
	job->unfinishedJobs--;
	auto unfinishedJobs = job->unfinishedJobs.load(std::memory_order_relaxed);
	if (unfinishedJobs == 0)
	{
		if (job->parent) { Finish(job->parent); }
		Deallocate(job);
	}
}

void ThreadPool::Yield() NOEXCEPT { std::this_thread::yield(); }

bool ThreadPool::HasJobCompleted(const Job * job)
{
	const auto unfinishedJobs = job->unfinishedJobs.load(std::memory_order_relaxed);
	return unfinishedJobs == 0;
}

// ------------------------------------------------------------------------------------------------------------------
