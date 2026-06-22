#pragma once
#include "pch.h"
#include "world/Biome.h"
#include "world/Noise.h"

class BiomeMap {
public:
    void generate(int width, int height, unsigned seed);
    BiomeType getBiome(int x, int y) const;
    int getWidth()  const { return m_width; }
    int getHeight() const { return m_height; }

private:
    BiomeType classify(float temp, float moist, float distFromCenter) const;

    std::vector<BiomeType> m_biomes;
    int m_width{ 0 }, m_height{ 0 };

    static constexpr float TEMP_SCALE = 0.004f;
    static constexpr float MOIST_SCALE = 0.004f;
    static constexpr float SNOW_T = 0.35f; // temp < ovo => Snow
    static constexpr float TROPICAL_T = 0.62f; // temp > ovo
    static constexpr float TROPICAL_M = 0.55f; // i moist > ovo => Tropical
    static constexpr float HOME_RADIUS = 40.f;  // tile-ovi, forsira Plains oko spawn-a
};