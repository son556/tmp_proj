#include "pch.h"
#include "MemoryPool/MemoryPool.h"
#include "LockFreeQueue.h"

MemoryPool::MemoryPool(int sizeHeight, int sizeWidth, vector<LockFreeQueue>& lockFreeQueeueList) : _queueList(lockFreeQueeueList)
{
	_freeCnt = 16 * 16 * sizeHeight * sizeWidth * 256 * 4;
	_memoryArr = new Node[_freeCnt];
	_freeHead = &(_memoryArr[0]);
	_freeTail.exchange(_freeHead);

	Node* connect = _freeHead;
	for (int i = 1; i < _freeCnt - 1; i++)
	{
		connect->next = &(_memoryArr[i]);
		connect = connect->next;
	}
	--_freeCnt;
}

MemoryPool::~MemoryPool()
{
	delete[] _memoryArr;
}

void MemoryPool::ReturnToPool(Node* nodeHead, Node* nodeTail, int listSize)
{
	Node* tail;
	Node* next;
	while (true)
	{
		tail = _freeTail.load();
		next = tail->next.load();
		if (tail == _freeTail.load())
		{
			if (next == nullptr)
			{
				if (tail->next.compare_exchange_weak(next, nodeHead))
				{
					_freeTail.compare_exchange_weak(tail, nodeTail);
					nodeTail->next.store(nullptr, memory_order_relaxed);
					_freeCnt.fetch_add(listSize);
					return;
				}
			}
			else
				_freeTail.compare_exchange_weak(tail, next);
		}
	}
}

Node* MemoryPool::GetNode(int& nodeCnt)
{
	Node* nodeTail = nullptr;
	Node* nodeHead = nullptr;

	FindFreeMemoryList(nodeHead, nodeTail, nodeCnt);

	return nodeHead;
}

// TODO memory order, memory barrier 공부
void MemoryPool::FindFreeMemoryList(Node* &head, Node* &tail, int& memoryCnt)
{
	Node* next;
	int maxCnt = -1;
	int nodeCnt = 0;
	int idx = -1;
	int qIdx = 0;

	tail = _freeHead.load();
	tail = tail->next.load();

	// 반환 할 list 만들기
	for (int i = 0; i < TOTAL_NODE_COUNT; i++)
	{
		if (_freeCnt == 0)
		{
			maxCnt = 0;
			idx = -1;
			qIdx = 0;
			for (auto& queue : _queueList)
			{
				nodeCnt = queue.GetFreeNodeCnt();
				if (maxCnt < nodeCnt)
				{
					maxCnt = nodeCnt;
					idx = qIdx;
				}
				++qIdx;
			}
			if (idx > -1)
				_queueList[idx].ReQuestReturnToPoolFreeNodeList();
			else // 모자란 경우 그대로 반환
				break;
		}
		if (tail->next != nullptr)
			tail = tail->next;
		--_freeCnt;
	}

	Node* step = head;

	while (true)
	{
		head = _freeHead.load();
		next = head->next.load();
		if (head->next.compare_exchange_weak(next, tail->next))
		{
			if (_freeHead.compare_exchange_weak(head, tail->next))
			{
				tail->next = nullptr;
				memoryCnt = 0;
				while (step != nullptr)
				{
					++memoryCnt;
					step = step->next;
				}
				return;
			}
		}
	}
}
