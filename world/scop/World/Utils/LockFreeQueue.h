#pragma once

#include "WorldUtils.h"
#include <mutex>

class MemoryPool;
struct Node;

class LockFreeQueue
{
public:
	LockFreeQueue(MemoryPool* const memoryPool);
	~LockFreeQueue();
	void EnQueue(int index, int lightValue);
	void DeQueue(Index2& res);
	void ReQuestReturnToPoolFreeNodeList();
	int GetFreeNodeCnt() const;

private:
	LockFreeQueue() = delete;
	LockFreeQueue(const LockFreeQueue&) = delete;
	LockFreeQueue& operator=(const LockFreeQueue&) = delete;

private:
	mutex _freeListMutex;
	MemoryPool* const _memoryPool;
	Node* _dummyNode;
	Node* _freeHead;
	Node* _freeTail;
	atomic<int> _notUseCnt;

	alignas(64) atomic<Node*> _head;
	alignas(64) atomic<Node*> _tail;
};
