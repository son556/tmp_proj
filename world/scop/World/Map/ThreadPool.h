#pragma once

#include <thread>
#include <condition_variable>

class [[nodiscard]] ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();
	void Wait();
	void SetTask(function<void()>&& taskFunc);

private:
	void CreateThread();
	void worker(int threadIndex);

private:
	vector<thread> m_threads;
	condition_variable m_notifyTask;
	queue<function<void()>> m_tasks;
	mutex m_taskMutex;
	size_t m_threadCnt = -1;
	bool m_stop = false;

private:
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
};

