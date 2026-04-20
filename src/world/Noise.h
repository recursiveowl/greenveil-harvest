#pragma once
#include <cmath>
#include <array>
#include <numeric>
#include <algorithm>
#include <random>

class PerlinNoise {
public:
    explicit PerlinNoise(unsigned seed = 42) {
        std::iota(m_perm.begin(), m_perm.begin() + 256, 0);
        std::default_random_engine engine(seed);
        std::shuffle(m_perm.begin(), m_perm.begin() + 256, engine);
        for (int i = 0; i < 256; ++i)
            m_perm[256 + i] = m_perm[i];
    }

    // Vraca vrijednost izmedu 0.0 i 1.0
    float noise(float x, float y) const {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        x -= std::floor(x);
        y -= std::floor(y);
        float u = fade(x), v = fade(y);
        int a = m_perm[X] + Y,     b = m_perm[X+1] + Y;
        float res = lerp(v,
            lerp(u, grad(m_perm[a],   x,   y),
                    grad(m_perm[b],   x-1, y)),
            lerp(u, grad(m_perm[a+1], x,   y-1),
                    grad(m_perm[b+1], x-1, y-1)));
        return (res + 1.f) * 0.5f;
    }

    // Octave noise za prirodniji izgled
    float octave(float x, float y, int octaves, float persistence = 0.5f) const {
        float val = 0.f, amp = 1.f, freq = 1.f, max = 0.f;
        for (int i = 0; i < octaves; ++i) {
            val  += noise(x * freq, y * freq) * amp;
            max  += amp;
            amp  *= persistence;
            freq *= 2.f;
        }
        return val / max;
    }

private:
    std::array<int, 512> m_perm{};

    static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    static float lerp(float t, float a, float b) { return a + t * (b - a); }
    static float grad(int hash, float x, float y) {
        switch (hash & 3) {
            case 0: return  x + y;
            case 1: return -x + y;
            case 2: return  x - y;
            default:return -x - y;
        }
    }
};
