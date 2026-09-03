#pragma once
#include "pch.h"
#include "world/TileMap.h"
#include "Inventory.h"

enum class CropStage : uint8_t {
    Seed = 0,
    Sprout,
    Grown,
    Ready,
    COUNT
};

enum class CropQuality : uint8_t {
    Rotten,
    Poor,
    Good,
    Perfect
};

struct Crop {
    int       tileX{0}, tileY{0};
    CropStage stage{CropStage::Seed};
    CropQuality quality{ CropQuality::Good };
    int       daysGrоwn{0};
    int       missedDays{ 0 };
    bool wateredToday{false};
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
    void advanceDay();
    bool tryWater(sf::Vector2f playerPos, const TileMap& map);
    bool tryTill(sf::Vector2f playerPos, TileMap& map, bool hasHoe);
    bool tryPlant(sf::Vector2f playerPos, TileMap& map);
    bool tryHarvest(sf::Vector2f playerPos, TileMap& map, int& seedsOut, ItemType& itemTypeOut);
    void draw(sf::RenderWindow& window);

private:
    std::unordered_map<int, FarmTile> m_farmTiles;
    std::vector<Crop>                 m_crops;

    static int key(int x, int y, int w) { return y * w + x; }

    static const sf::Color STAGE_COLORS[static_cast<int>(CropStage::COUNT)];
};
