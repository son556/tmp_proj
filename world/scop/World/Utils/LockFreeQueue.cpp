#include "pch.h"
#include "LockFreeQueue.h"
#include "MemoryPool/MemoryPool.h"

constexpr int MAX_FREE_CNT = 16 * 16 * 256 * 20;

LockFreeQueue::LockFreeQueue(MemoryPool* const memoryPool) : _memoryPool(memoryPool)
{
    _dummyNode = new Node(-1, -1, nullptr);
    _head = _dummyNode;
    _tail = _dummyNode;
    _freeHead = nullptr;
    _freeTail = nullptr;
    _notUseCnt = 0;
}

LockFreeQueue::~LockFreeQueue()
{
    delete _dummyNode;
}

void LockFreeQueue::EnQueue(int index, int lightValue)
{
    Node* head;
    Node* tail;
    Node* next;
    Node* newNode;
    if (_freeHead == nullptr)
    {
        lock_guard lock(_freeListMutex);
        _freeHead = _memoryPool->GetNode();
        _notUseCnt = 16 * 16 * 256;
    }
    else
    {
        lock_guard lock(_freeListMutex);
        newNode = _freeHead;
        _freeHead = newNode->next;
        _notUseCnt--;
    }
    while (true)
    {
        head = _head.load();
        tail = _tail.load();
        next = tail->next;
        if (tail == _tail.load())
        {
            tail->next = newNode;
            tail->next.compare_exchange_week(tail->next, newNode);
            if (_tail.compare_exchange_weak(tail, newNode))
            {
                return;
            }
        }
    }
}

void LockFreeQueue::DeQueue(Index2& res)
{
    Node* head;
    Node* tail;
    Node* next;

    while (true) 
    {
        head = _head.load();
        tail = _tail.load();
        next = tail->next;

        if (head == _head.load()) 
        {
            if (head == tail)
            {
                if (next == nullptr) 
                {
                    res.flag = false;
                    return;
                }
                _tail.compare_exchange_weak(tail, next);
            }
            else 
            {
                if (_head.compare_exchange_weak(head, next)) 
                {
                    res.x = head->lightIndex;
                    res.y = head->lightValue;
                    res.flag = true;

                    lock_guard lock(_freeListMutex);
                    _freeTail->next = head;
                    ++_notUseCnt;
                    if (_notUseCnt == MAX_FREE_CNT)
                    {
                        _memoryPool->ReturnToPool(_freeHead);
                        _notUseCnt = 0;
                        _freeTail = nullptr;
                        _freeHead = nullptr;
                    }
                    return;
                }
            }
        }
    }
}

void LockFreeQueue::ReQuestReturnToPoolFreeNodeList()
{
    lock_guard lock(_freeListMutex);
    _memoryPool->ReturnToPool(_freeHead, _freeTail, _notUseCnt);
    _freeHead = nullptr;
    _freeTail = nullptr;
    _notUseCnt = 0;
}

int LockFreeQueue::GetFreeNodeCnt() const
{
    return _notUseCnt;
}
