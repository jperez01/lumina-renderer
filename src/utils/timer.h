//
// Created by juperez on 5/25/23.
//

#pragma once

#include "core/common.h"

LUMINA_NAMESPACE_BEGIN

class Timer {
public:
    /// Create a new timer and reset it
    Timer() { reset(); }

    /// Reset the timer to the current time
    void reset() { start = std::chrono::system_clock::now(); }

    /// Return the number of milliseconds elapsed since the timer was last reset
    double elapsed() const {
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        return (double) duration.count();
    }

    /// Like \ref elapsed(), but return a human-readable string
    std::string elapsedString(bool precise = false) const {
        return timeString(elapsed(), precise);
    }

    /// Return the number of milliseconds elapsed since the timer was last reset and then reset it
    double lap() {
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        start = now;
        return (double) duration.count();
    }

    /// Like \ref lap(), but return a human-readable string
    std::string lapString(bool precise = false) {
        return timeString(lap(), precise);
    }
private:
    std::chrono::system_clock::time_point start;
};

LUMINA_NAMESPACE_END