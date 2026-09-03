#include "systems/FarmingSystem.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

const sf::Color FarmingSystem::STAGE_COLORS[] = {
    sf::Color(180, 140,  60),  // Seed
    sf::Color( 80, 200,  80),  // Sprout
    sf::Color( 30, 150,  30),  // Grown
    sf::Color(240, 200,   0),  // Ready
};

static sf::Vector2f tileCenter(int x, int y) {
    return {
        static_cast<float>(x * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2),
        static_cast<float>(y * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2)
    };
}

void FarmingSystem::update(float dt, const TileMap& map) {}

bool FarmingSystem::tryTill(sf::Vector2f playerPos, TileMap& map, bool hasHoe) {

    if (!hasHoe) return false;

    int tileX = static_cast<int>(playerPos.x) / TileMap::TILE_SIZE;
    int tileY = static_cast<int>(playerPos.y) / TileMap::TILE_SIZE;

    Tile& t = map.getTileMut(tileX, tileY);
    if (t.solid) return false;

    // Zelena trava -> kafena zemlja
    if (t.type == TileType::Grass) {
        map.setTileType(tileX, tileY, TileType::Dirt);
        t.fertile = map.getTile(tileX, tileY).fertile;
    }

    if (t.type != TileType::Dirt) return false;

    int k = key(tileX, tileY, map.getWidth());
    m_farmTiles[k].tilled = true;
    return true;
}

bool FarmingSystem::tryPlant(sf::Vector2f playerPos, TileMap& map) {
    int tileX = static_cast<int>(playerPos.x) / TileMap::TILE_SIZE;
    int tileY = static_cast<int>(playerPos.y) / TileMap::TILE_SIZE;
    int k     = key(tileX, tileY, map.getWidth());

    auto it = m_farmTiles.find(k);
    if (it == m_farmTiles.end()) return false;
    if (!it->second.tilled)      return false;
    if (it->second.hasCrop)      return false;

    Crop crop;
    crop.tileX   = tileX;
    crop.tileY   = tileY;
    crop.fertile = map.getTile(tileX, tileY).fertile;

    m_crops.push_back(crop);
    it->second.hasCrop = true;
    it->second.cropIdx = static_cast<int>(m_crops.size()) - 1;
    return true;
}

bool FarmingSystem::tryWater(sf::Vector2f playerPos, const TileMap& map) {
    int tileX = static_cast<int>(playerPos.x) / TileMap::TILE_SIZE;
    int tileY = static_cast<int>(playerPos.y) / TileMap::TILE_SIZE;
    int k = key(tileX, tileY, map.getWidth());

    auto it = m_farmTiles.find(k);
    if (it == m_farmTiles.end()) return false;
    if (!it->second.hasCrop) return false;

    Crop& crop = m_crops[it->second.cropIdx];

    if (crop.stage == CropStage::Ready) return false;
    if (crop.wateredToday) return false;

    crop.wateredToday = true;
    return true;
}

void FarmingSystem::advanceDay() {
    for (auto& crop : m_crops) {
        if (crop.stage == CropStage::Ready) continue;

        if (!crop.wateredToday) crop.missedDays++;
        crop.wateredToday = false;
        crop.daysGrоwn++;

        int growTarget = crop.fertile ? 2 : 3;

        if (crop.daysGrоwn >= growTarget) {
            crop.stage = CropStage::Ready;
            float ratio = static_cast<float>(crop.missedDays) / growTarget;
            if (ratio <= 0.f)          crop.quality = CropQuality::Perfect;
            else if (ratio <= 1.f / 3.f) crop.quality = CropQuality::Good;
            else if (ratio <= 2.f / 3.f) crop.quality = CropQuality::Poor;
            else                       crop.quality = CropQuality::Rotten;
        }
        else {
            crop.stage = static_cast<CropStage>(std::min(crop.daysGrоwn, 2));
        }
    }
}

bool FarmingSystem::tryHarvest(sf::Vector2f playerPos, TileMap& map,
    int& seedsOut, ItemType& itemTypeOut) {
    int tileX = static_cast<int>(playerPos.x) / TileMap::TILE_SIZE;
    int tileY = static_cast<int>(playerPos.y) / TileMap::TILE_SIZE;
    int k = key(tileX, tileY, map.getWidth());

    auto it = m_farmTiles.find(k);
    if (it == m_farmTiles.end()) return false;
    if (!it->second.hasCrop)     return false;

    Crop& crop = m_crops[it->second.cropIdx];

    if (crop.stage != CropStage::Ready) return false;

    // Rotten — ukloni, ali nema nagrade
    if (crop.quality == CropQuality::Rotten) {
        it->second.hasCrop = false;
        it->second.tilled = false;
        it->second.cropIdx = -1;
        crop = {};
        seedsOut = 0;
        itemTypeOut = ItemType::None;
        return true;
    }

    // Plodna zemlja daje više
    seedsOut = crop.fertile
        ? 3 + std::rand() % 3   // 3-5
        : 2 + std::rand() % 2;  // 2-3

    itemTypeOut = (crop.quality == CropQuality::Poor)
        ? ItemType::CropPoor
        : ItemType::Crop;

    it->second.hasCrop = false;
    it->second.tilled = false;
    it->second.cropIdx = -1;
    crop = {};
    return true;
}

void FarmingSystem::draw(sf::RenderWindow& window) {
    for (auto& crop : m_crops) {
        if (crop.tileX == 0 && crop.tileY == 0 &&
            crop.stage == CropStage::Seed &&
            crop.daysGrоwn == 0.f) continue;

        sf::CircleShape circle(9.f);
        circle.setFillColor(STAGE_COLORS[static_cast<int>(crop.stage)]);
        circle.setOutlineThickness(1.5f);
        circle.setOutlineColor(sf::Color(0, 0, 0, 120));
        circle.setOrigin({9.f, 9.f});
        circle.setPosition(tileCenter(crop.tileX, crop.tileY));
        window.draw(circle);
    }
}
