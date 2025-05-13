#include "SystemClock.h"

void SystemClock::Start()
{
    m_StartTime = Clock::now();
    m_LastSimTime = m_StartTime;
    m_LastRealTime = m_StartTime;
    m_AccumulatedTime = 0.0;
    m_Paused = false;
}

void SystemClock::Pause()
{
    if (!m_Paused)
    {
        m_PauseTime = Clock::now();
        m_Paused = true;
    }
}

void SystemClock::Resume()
{
    if (m_Paused)
    {
        auto now = Clock::now();
        double pausedDuration = std::chrono::duration<double>(now - m_PauseTime).count();
        m_AccumulatedTime += pausedDuration;

        m_LastSimTime += std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(pausedDuration));
        m_Paused = false;
    }
}

void SystemClock::Reset()
{
    m_StartTime = Clock::now();
    m_LastSimTime = m_StartTime;
    m_LastRealTime = m_StartTime;
    m_AccumulatedTime = 0.0;
    m_Paused = false;
}

bool SystemClock::IsPaused()
{
    bool result = m_Paused;
    return result;
}

double SystemClock::GetElapsedTime()
{
    double time;
    if (m_Paused)
    {
        time = std::chrono::duration<double>(m_PauseTime - m_StartTime).count() - m_AccumulatedTime;
    }
    else
    {
        time = std::chrono::duration<double>(Clock::now() - m_StartTime).count() - m_AccumulatedTime;
    }
    return time;
}

double SystemClock::GetRunningTime()
{
    double time = std::chrono::duration<double>(Clock::now() - m_StartTime).count();
    return time;
}

double SystemClock::GetDeltaTime()
{
    if (m_Paused)
    {
        return 0.0;
    }

    auto now = Clock::now();
    double delta = std::chrono::duration<double>(now - m_LastSimTime).count();
    m_LastSimTime = now;
    return delta;
}

double SystemClock::GetRunningDeltaTime()
{
    auto now = Clock::now();
    double delta = std::chrono::duration<double>(now - m_LastRealTime).count();
    m_LastRealTime = now;
    return delta;
}
