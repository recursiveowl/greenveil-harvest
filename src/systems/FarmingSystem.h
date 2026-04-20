#pragma once
#include "pch.h"
#include "world/TileMap.h"

enum class CropStage : uint8_t {
    Seed = 0,
    Sprout,
    Grown,
    Ready,
    Withered, // uvenulo — daleko od vode
    COUNT
};

struct Crop {
    int       tileX{0}, tileY{0};
    CropStage stage{CropStage::Seed};
    float     growTimer{0.f};
    float     growTime{3.f}; // 3 sekunde po fazi (brže!)
    bool      fertile{false};
};

struct FarmTile {
    bool tilled  = false;
    bool hasCrop = false;
    int  cropIdx = -1;
};

class FarmingSystem {
public:
    void update(float dt, const TileMap& map);
    bool tryTill(sf::Vector2f playerPos, TileMap& map);
    bool tryPlant(sf::Vector2f playerPos, TileMap& map);
    bool tryHarvest(sf::Vector2f playerPos, TileMap& map, int& seedsOut);
    void draw(sf::RenderWindow& window);

private:
    std::unordered_map<int, FarmTile> m_farmTiles;
    std::vector<Crop>                 m_crops;

    static int key(int x, int y, int w) { return y * w + x; }

    static const sf::Color STAGE_COLORS[static_cast<int>(CropStage::COUNT)];
};
