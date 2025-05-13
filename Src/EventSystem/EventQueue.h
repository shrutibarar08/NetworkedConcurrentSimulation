#pragma once
#include <queue>
#include <Windows.h>

enum class EventType : unsigned int
{
    WINDOW_EVENT_WINDOWED,
    WINDOW_EVENT_FULLSCREEN,
    WINDOW_EVENT_SCREEN_TOGGLE,
    WINDOW_EVENT_RESIZE,

    EVENT_DEFAULT
};

class EventQueue
{
public:
    static void Init();
    static void Shutdown();

    static void Push(EventType event);
    static bool Pop(EventType& outEvent);
    static bool IsEmpty();

private:
    static std::queue<EventType> s_Queue;
    static HANDLE s_Mutex;
    inline static EventType m_PrevEvent{ EventType::EVENT_DEFAULT };
};
