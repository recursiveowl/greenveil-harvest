#pragma once
#include <algorithm>

namespace UI {
    static float scaleX = 1.f;
    static float scaleY = 1.f;
    static float scale = 1.f;

    static void init(unsigned winW, unsigned winH) {
        scaleX = (float)winW / 1280.f;
        scaleY = (float)winH / 720.f;
        scale = std::min(scaleX, scaleY); // uniformni scale
    }

    static float X(float x) { return x * scale; } // ← scale umjesto scaleX
    static float Y(float y) { return y * scale; } // ← scale umjesto scaleY
    static float S(float v) { return v * scale; }
}