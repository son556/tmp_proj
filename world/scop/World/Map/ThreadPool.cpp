#include "pch.h"
#include "ThreadPool.h"

ThreadPool::ThreadPool()
{
}

ThreadPool::~ThreadPool()
{
	Wait();
	m_stop = true;
	m_notifyTask.notify_all();
	for (int i = 0; i < m_threadCnt; i++)
		m_threads[i].join();
}



void ThreadPool::Wait()
{
	unique_lock lock(m_taskMutex);
	m_notifyTask.wait(lock, [this]() { return m_tasks.empty(); });
}

void ThreadPool::SetTask(function<void()>&& taskFunc)
{
	if (m_threadCnt == -1) // debug용
	{
		cerr << "no thread" << endl;
		throw;
	}
	unique_lock lock(m_taskMutex);
	m_tasks.emplace(forward<function<void()>>(taskFunc));
	m_notifyTask.notify_one();
}

void ThreadPool::CreateThread()
{
	m_threadCnt = thread::hardware_concurrency();
	if (m_threadCnt >= 1) // main thread 제외
		m_threadCnt -= 1;
	m_threads.reserve(m_threadCnt);
	for (int i = 0; i < m_threadCnt; i++)
	{
		m_threads.emplace_back(thread(
			[this, i]() 
			{
				worker(i);
			}
		));
	}
}

void ThreadPool::worker(int threadIndex)
{
	function<void()> task;
	while (true)
	{
		{
			unique_lock taskLock(m_taskMutex);
			m_notifyTask.wait(taskLock, 
				[this]() 
				{
					return m_tasks.empty() == false || m_stop; // 참일 때 까지 기다림
				}
			);
			if (m_stop && m_tasks.empty())
				break;
			task = std::move(m_tasks.front());
			m_tasks.pop();
		}
		task();
	}
}
