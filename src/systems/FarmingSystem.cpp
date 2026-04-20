#include "systems/FarmingSystem.h"
#include <cmath>
#include <cstdlib>

const sf::Color FarmingSystem::STAGE_COLORS[] = {
    sf::Color(180, 140,  60),  // Seed
    sf::Color( 80, 200,  80),  // Sprout
    sf::Color( 30, 150,  30),  // Grown
    sf::Color(240, 200,   0),  // Ready
    sf::Color(100,  80,  40),  // Withered
};

static sf::Vector2f tileCenter(int x, int y) {
    return {
        static_cast<float>(x * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2),
        static_cast<float>(y * TileMap::TILE_SIZE + TileMap::TILE_SIZE / 2)
    };
}

void FarmingSystem::update(float dt, const TileMap& map) {
    for (auto& crop : m_crops) {
        if (crop.stage == CropStage::Ready ||
            crop.stage == CropStage::Withered) continue;

        // Provjeri da li je tile još uvijek blizu vode
        const Tile& t = map.getTile(crop.tileX, crop.tileY);
        if (!t.fertile && crop.stage != CropStage::Seed) {
            crop.stage = CropStage::Withered;
            continue;
        }

        crop.growTimer += dt;
        if (crop.growTimer >= crop.growTime) {
            crop.growTimer = 0.f;
            crop.stage = static_cast<CropStage>(
                static_cast<int>(crop.stage) + 1);
        }
    }
}

bool FarmingSystem::tryTill(sf::Vector2f playerPos, TileMap& map) {
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
    // Plodna zemlja raste 2x brže
    crop.growTime = crop.fertile ? 2.f : 3.f;

    m_crops.push_back(crop);
    it->second.hasCrop = true;
    it->second.cropIdx = static_cast<int>(m_crops.size()) - 1;
    return true;
}

bool FarmingSystem::tryHarvest(sf::Vector2f playerPos, TileMap& map, int& seedsOut) {
    int tileX = static_cast<int>(playerPos.x) / TileMap::TILE_SIZE;
    int tileY = static_cast<int>(playerPos.y) / TileMap::TILE_SIZE;
    int k     = key(tileX, tileY, map.getWidth());

    auto it = m_farmTiles.find(k);
    if (it == m_farmTiles.end()) return false;
    if (!it->second.hasCrop)     return false;

    Crop& crop = m_crops[it->second.cropIdx];

    // Uvenulo — možeš ukloniti ali nema nagrade
    if (crop.stage == CropStage::Withered) {
        it->second.hasCrop = false;
        it->second.tilled  = false;
        it->second.cropIdx = -1;
        crop = {};
        seedsOut = 0;
        return true;
    }

    if (crop.stage != CropStage::Ready) return false;

    // Plodna zemlja daje više
    seedsOut = crop.fertile
        ? 3 + std::rand() % 3   // 3-5
        : 2 + std::rand() % 2;  // 2-3

    it->second.hasCrop = false;
    it->second.tilled  = false;
    it->second.cropIdx = -1;
    crop = {};
    return true;
}

void FarmingSystem::draw(sf::RenderWindow& window) {
    for (auto& crop : m_crops) {
        if (crop.tileX == 0 && crop.tileY == 0 &&
            crop.stage == CropStage::Seed &&
            crop.growTimer == 0.f) continue;

        sf::CircleShape circle(9.f);
        circle.setFillColor(STAGE_COLORS[static_cast<int>(crop.stage)]);
        circle.setOutlineThickness(1.5f);
        circle.setOutlineColor(sf::Color(0, 0, 0, 120));
        circle.setOrigin({9.f, 9.f});
        circle.setPosition(tileCenter(crop.tileX, crop.tileY));
        window.draw(circle);
    }
}
