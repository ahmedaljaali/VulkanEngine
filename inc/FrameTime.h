#pragma once

// std
#include <chrono>

namespace VE
{
    class FrameTime final
    {
    private:  // Private variables
        std::chrono::system_clock::time_point m_startTime;
        std::chrono::system_clock::time_point m_gameLoopStartingTime;
        float m_frameTime;

    public:  // Public variables

    private:  // Private methods

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                  Don't copy or move my class!!!                  */

        FrameTime(const FrameTime& copy) = delete;
        FrameTime& operator=(const FrameTime& copy) = delete;
        FrameTime(FrameTime&& move) = delete;
        FrameTime& operator=(FrameTime&& move) = delete;
        /*------------------------------------------------------------------*/

        // Constructor
        FrameTime(void);

        // Destructor
        ~FrameTime(void);

        void gameLoopStarted(void);
        float getFrameTime(void);
    };
}
