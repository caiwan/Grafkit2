#pragma once

#include "ThreadPool.h"

namespace JobSystem
{

	class TaskScheduler
	{
	public:
		TaskScheduler();

		//    template<typename FunctionType, typename... Args> std::future<typename std::result_of<FunctionType(Args...)>::type> Async(FunctionType && job,
		//    Args &&... args);

		// ------------------------------------------------------------------------------------------------------------------

		//  template<typename FunctionType, typename... Args> std::future<typename std::result_of<FunctionType(Args...)>::type> ThreadPool::Async(FunctionType
		//  && job, Args &&... args)
		//  {
		//    using ReturnType = typename std::result_of<FunctionType(Args...)>::type;
		//    auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<FunctionType>(job), std::forward<Args>(args)...));
		//    std::future<ReturnType> res = task->get_future();

		// TODO:
		//    if (mIsStop) throw std::runtime_error("Cannot add new tasks to a stopped ThreadPool");
		//    const Job taskWrapper({ [task]() { (*task)(); } });
		//    if (!mQueue.Push(taskWrapper)) throw std::runtime_error("bounded task queue is full you cannot add more.");
		//    return res;
		//  }

	private:
		Internal::ThreadPool mThreadPool;
	};

} // namespace JobSystem
