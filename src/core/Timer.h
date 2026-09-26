#pragma once
#include "Types.h"
#include <chrono>

class Timer {
public:
    void start() { m_start = now(); }
    void reset() { m_start = now(); }
    f64 elapsed() const { return now() - m_start; }
    f64 restart() {
        f64 e = elapsed();
        reset();
        return e;
    }

    static f64 now() {
        return std::chrono::duration<f64>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
    }

private:
    f64 m_start = now();
};

class FPSCounter {
public:
    void tick() {
        m_frameCount++;
        f64 t = Timer::now();
        if (t - m_lastTime >= 1.0) {
            m_fps = m_frameCount;
            m_frameCount = 0;
            m_lastTime = t;
        }
    }

    u32 fps() const { return m_fps; }

private:
    u32 m_fps = 0;
    u32 m_frameCount = 0;
    f64 m_lastTime = 0;
};
