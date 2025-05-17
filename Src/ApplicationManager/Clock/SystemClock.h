#pragma once
#include <chrono>
#include <Windows.h>

class SystemClock
{
public:
    // Delete instancing
    SystemClock() = delete;
    SystemClock(const SystemClock&) = delete;
    SystemClock(SystemClock&&) = delete;
    SystemClock& operator=(const SystemClock&) = delete;
    SystemClock& operator=(SystemClock&&) = delete;

    // Static interface
    static void Start();                    // Starts or restarts the clock
    static void Pause();                    // Pauses the simulation clock
    static void Resume();                   // Resumes simulation clock
    static void Reset();                    // Resets everything
    static bool IsPaused();                 // Returns true if simulation is paused

    static double GetElapsedTime();         // Simulation time (pausable)
    static double GetRunningTime();         // Absolute running time (non-pausable)
    static double GetDeltaTime();           // Delta of simulation time (pausable)
    static double GetRunningDeltaTime();    // Delta of running time (non-pausable)

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    inline static TimePoint m_StartTime;
    inline static TimePoint m_PauseTime;
    inline static TimePoint m_LastSimTime;
    inline static TimePoint m_LastRealTime;

    inline static double m_AccumulatedTime = 0.0;
    inline static bool m_Paused = false;
};
