#pragma once
#include "pch.h"

enum class BiomeType : uint8_t { Plains = 0, Tropical, Snow, COUNT };

struct BiomeDef {
    BiomeType   type;
    std::string name;
    sf::Color   ambientTint;
    float       fertileMult;
    float       floraDensity;
    std::string tileTexture[4]; // indeks = TileType ordinal
};

const BiomeDef& getBiomeDef(BiomeType type);