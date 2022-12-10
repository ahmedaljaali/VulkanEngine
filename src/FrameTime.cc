#include "FrameTime.h"

namespace VE
{
    // Constructor
    FrameTime::FrameTime(void) : m_startTime{std::chrono::high_resolution_clock::now()}, m_frameTime{} {}

    // Destructor
    FrameTime::~FrameTime(void) = default;

    void FrameTime::gameLoopStarted(void)
    {
        m_gameLoopStartingTime = std::chrono::high_resolution_clock::now();

        m_frameTime =
              std::chrono::duration<float, std::chrono::seconds::period>(m_gameLoopStartingTime - m_startTime).count();

        m_startTime = m_gameLoopStartingTime;
    }

    float FrameTime::getFrameTime(void) { return m_frameTime; }
}
