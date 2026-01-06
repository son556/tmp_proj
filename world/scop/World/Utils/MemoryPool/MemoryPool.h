#pragma once

#include <mutex>

class LockFreeQueue;

struct Node
{
	int lightIndex = -1;
	int lightValue = -1;
	atomic<Node*> next = nullptr;
};

class MemoryPool
{
public:
	MemoryPool(int sizeHeight, int sizeWidth, vector<LockFreeQueue>& lockFreeQueueList);
	~MemoryPool();
	void ReturnToPool(Node* nodeHead, Node* nodeTail, int listSize);
	Node* GetNode();

private:
	MemoryPool() = delete;
	MemoryPool(const MemoryPool&) = delete;
	MemoryPool& operator=(const MemoryPool&) = delete;

private:
	void FindFreeMemoryList(Node* head, Node* tail);

private:
	vector<LockFreeQueue>& _queueList;
	Node* _memoryArr;
	alignas(64) atomic<Node*> _freeTail;
	alignas(64) atomic<Node*> _freeHead;
	alignas(64) atomic<int> _freeCnt;
};

