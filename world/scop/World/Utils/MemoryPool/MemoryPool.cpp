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
					_freeCnt.fetch_add(listSize);
				}
			}
			else
				_freeTail.compare_exchange_weak(tail, tail->next);
		}
	}
	_freeCnt.fetch_add(listSize);
}

Node* MemoryPool::GetNode()
{
	Node* nodeTail;
	Node* nodeHead;
	
	FindFreeMemoryList(nodeHead, nodeTail);


	//todo node list를 만들어서 반환

	return nodeHead;
}

// TODO memory order, memory barrier 공부
// TODO 여기 완성시킬것 -> 완성 후 GetNode에 옮기기
void MemoryPool::FindFreeMemoryList(Node* head, Node* tail)
{
	Node* next;
	int maxCnt = -1;
	int nodeCnt;
	int idx = -1;
	int qIdx = 0;

	tail = _freeHead.load();
	tail = tail->next.load();

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
			{
				_queueList[idx].ReQuestReturnToPoolFreeNodeList();
				_freeCnt.fetch_add(-maxCnt);
			}
			else
				break;
		}
		if (tail->next != nullptr)
			tail = tail->next;
		--_freeCnt;
	}

	// head 설정, _freeHead, _freeTail 변경
	while (true)
	{
		head = _freeHead.load();
		next = head->next.load();
		if (head->next.compare_exchange_weak(next, tail->next))
		{

		}
	}
}
