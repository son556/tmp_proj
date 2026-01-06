#include "pch.h"
#include "ThreadPool/ThreadPool.h"

ThreadPool::ThreadPool()
{
	CreateThread();
}

ThreadPool::~ThreadPool()
{
	Wait();
	_stop = true;
	_notifyTask.notify_all();
	for (int i = 0; i < _threadCnt; i++)
		_threads[i].join();
}



void ThreadPool::Wait()
{
	unique_lock lock(_taskMutex);
	_notifyMain.wait(lock, [this]() { return _tasks.empty() && _workingTrheadCnt == 0; });
}

void ThreadPool::SetTask(function<void(int)>&& taskFunc)
{
	if (_threadCnt == -1) // debug용
	{
		cerr << "no thread" << endl;
		throw;
	}
	unique_lock lock(_taskMutex);
	_tasks.emplace(forward<function<void(int)>>(taskFunc));
	_notifyTask.notify_one();
}

int ThreadPool::GetThreadCount() const
{
	return _threadCnt;
}

void ThreadPool::CreateThread()
{
	_threadCnt = thread::hardware_concurrency();
	if (_threadCnt >= 1) // main thread 제외
		_threadCnt -= 1;
	_threads.reserve(_threadCnt);
	for (int i = 0; i < _threadCnt; i++)
	{
		_threads.emplace_back(thread(
			[this, i]() 
			{
				worker(i);
			}
		));
	}
}

void ThreadPool::worker(int threadIndex)
{
	function<void(int)> task;
	while (true)
	{
		{
			unique_lock taskLock(_taskMutex);
			_notifyTask.wait(taskLock, 
				[this]() 
				{
					return _tasks.empty() == false || _stop; // 참일 때 까지 기다림
				}
			);
			if (_stop && _tasks.empty())
				break;
			task = std::move(_tasks.front());
			_tasks.pop();
		}
		++_workingTrheadCnt;
		task(threadIndex);
		--_workingTrheadCnt;
		lock_guard local_lock(_taskMutex);
		if (_tasks.empty() && _workingTrheadCnt == 0)
			_notifyMain.notify_one();
	}
}
