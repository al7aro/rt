#pragma once

#include <chrono>

namespace rt {

    class Timer
    {
    private:
        std::chrono::high_resolution_clock::time_point _start;
    public:
        Timer()
        {
            _start = std::chrono::high_resolution_clock::now();
        }

        void Restart()
        {
            _start = std::chrono::high_resolution_clock::now();
        }

        double EllapsedMiliseconds() const
        {
            std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
            std::chrono::high_resolution_clock::duration ellapsed = now - _start;
            return (std::chrono::duration<double, std::milli>(ellapsed).count());
        }

        double EllapsedSeconds() const
        {
            return (EllapsedMiliseconds() / 1000.0);
        }

    };

}