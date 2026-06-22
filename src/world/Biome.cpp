#include "world/Biome.h"

static const std::array<BiomeDef, static_cast<int>(BiomeType::COUNT)> kBiomeDefs = { {
    { BiomeType::Plains, "Plains", sf::Color::White, 1.0f, 1.0f, {
        "assets/textures/plains_grass.png",
        "assets/textures/plains_dirt.png",
        "assets/textures/water.png",
        "assets/textures/plains_sand.png" } },

    { BiomeType::Tropical, "Tropical", sf::Color(255,245,225), 1.2f, 1.4f, {
        "assets/textures/tropical_grass.png",
        "assets/textures/tropical_dirt.png",
        "assets/textures/water.png",
        "assets/textures/tropical_sand.png" } },

    { BiomeType::Snow, "Snow", sf::Color(225,235,255), 0.6f, 0.4f, {
        "assets/textures/snow.png",
        "assets/textures/plains_dirt.png",
        "assets/textures/water_frozen.png",
        "assets/textures/plains_sand.png" } }
} };

const BiomeDef& getBiomeDef(BiomeType type) {
    return kBiomeDefs[static_cast<int>(type)];
}