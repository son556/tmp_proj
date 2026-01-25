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
	Node* GetNode(int& nodeCnt);

private:
	MemoryPool() = delete;
	MemoryPool(const MemoryPool&) = delete;
	MemoryPool& operator=(const MemoryPool&) = delete;

private:
	void FindFreeMemoryList(Node* &head, Node* &tail, int& memoryCnt);

private:
	vector<LockFreeQueue>& _queueList;
	Node* _memoryArr;
	alignas(std::hardware_destructive_interference_size) atomic<Node*> _freeTail;
	alignas(std::hardware_destructive_interference_size) atomic<Node*> _freeHead;
	alignas(std::hardware_destructive_interference_size) atomic<int> _freeCnt;
};

