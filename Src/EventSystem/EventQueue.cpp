#include "EventQueue.h"

#include "Utils/Logger.h"

std::queue<EventType> EventQueue::s_Queue;
HANDLE EventQueue::s_Mutex = nullptr;

void EventQueue::Init()
{
    s_Mutex = CreateMutex(nullptr, FALSE, nullptr);
}

void EventQueue::Shutdown()
{
    if (s_Mutex)
    {
        CloseHandle(s_Mutex);
        s_Mutex = nullptr;
    }
}

void EventQueue::Push(EventType event)
{
    if (s_Mutex && WaitForSingleObject(s_Mutex, INFINITE) == WAIT_OBJECT_0)
    {
        m_PrevEvent = event;
        s_Queue.push(event);
        LOG_INFO("Event Pushed");
        ReleaseMutex(s_Mutex);
    }
}

bool EventQueue::Pop(EventType& outEvent)
{
    if (s_Mutex && WaitForSingleObject(s_Mutex, INFINITE) == WAIT_OBJECT_0)
    {
        if (!s_Queue.empty())
        {
            outEvent = s_Queue.front();
            s_Queue.pop();
            ReleaseMutex(s_Mutex);
            return true;
        }
        ReleaseMutex(s_Mutex);
    }
    return false;
}

bool EventQueue::IsEmpty()
{
    if (s_Mutex && WaitForSingleObject(s_Mutex, INFINITE) == WAIT_OBJECT_0)
    {
        bool empty = s_Queue.empty();
        ReleaseMutex(s_Mutex);
        return empty;
    }
    return true;
}
