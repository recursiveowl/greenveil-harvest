#include "world/BiomeMap.h"

void BiomeMap::generate(int width, int height, unsigned seed) {
    m_width = width;
    m_height = height;
    m_biomes.assign(static_cast<size_t>(width) * height, BiomeType::Plains);

    PerlinNoise tempNoise(seed + 7000);
    PerlinNoise moistNoise(seed + 13000);

    sf::Vector2f center{ width * 0.5f, height * 0.5f };

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float temp = tempNoise.octave(x * TEMP_SCALE, y * TEMP_SCALE, 4);
            float moist = moistNoise.octave(x * MOIST_SCALE, y * MOIST_SCALE, 4);
            float dist = std::hypot(x - center.x, y - center.y);
            m_biomes[y * width + x] = classify(temp, moist, dist);
        }
    }
    int counts[static_cast<int>(BiomeType::COUNT)] = {};
    sf::Vector2i firstTropical{ -1, -1 }, firstSnow{ -1, -1 };
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            BiomeType b = m_biomes[y * width + x];
            ++counts[static_cast<int>(b)];
            if (b == BiomeType::Tropical && firstTropical.x < 0) firstTropical = { x, y };
            if (b == BiomeType::Snow && firstSnow.x < 0) firstSnow = { x, y };
        }
    printf("Biome counts -> Plains=%d Tropical=%d Snow=%d\n", counts[0], counts[1], counts[2]);
    printf("Tropical tile (%d,%d) offset (%d,%d)\n", firstTropical.x, firstTropical.y,
        firstTropical.x - width / 2, firstTropical.y - height / 2);
    printf("Snow tile (%d,%d) offset (%d,%d)\n", firstSnow.x, firstSnow.y,
        firstSnow.x - width / 2, firstSnow.y - height / 2);
}

BiomeType BiomeMap::classify(float temp, float moist, float distFromCenter) const {
    if (distFromCenter < HOME_RADIUS) return BiomeType::Plains;
    if (temp < SNOW_T) return BiomeType::Snow;
    if (temp > TROPICAL_T && moist > TROPICAL_M) return BiomeType::Tropical;
    return BiomeType::Plains;
}

BiomeType BiomeMap::getBiome(int x, int y) const {
    x = std::clamp(x, 0, m_width - 1);
    y = std::clamp(y, 0, m_height - 1);
    return m_biomes[y * m_width + x];
}