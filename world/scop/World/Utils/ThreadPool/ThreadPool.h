#pragma once

#include <thread>
#include <condition_variable>

class [[nodiscard]] ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();
	void Wait();
	void SetTask(function<void(int)>&& taskFunc);
	int GetThreadCount() const;

private:
	void CreateThread();
	void worker(int threadIndex);

private:
	vector<thread> _threads;
	condition_variable _notifyTask;
	condition_variable _notifyMain;
	queue<function<void(int)>> _tasks;
	std::mutex _taskMutex;
	size_t _threadCnt = -1;
	atomic<int> _workingTrheadCnt = 0;
	bool _stop = false;

private:
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
};

