#include "world/BushSystem.h"
#include <random>

void BushSystem::generate(const TileMap& map, unsigned seed) {
    if (!m_tex.loadFromFile("assets/textures/pl_bush.png"))
        return;

    m_bushes.clear();
    std::mt19937 rng(seed + 999);
    std::uniform_real_distribution<float> spawnChance(0.f, 1.f);
    constexpr float BASE_CHANCE = 1.f / 180.f;
    std::uniform_real_distribution<float> animOff(0.f, 6.28f);

    // Set zauzetih tile-ova za spacing provjeru
    std::vector<std::pair<int, int>> occupied;

    auto isOccupied = [&](int x, int y) {
        for (auto& o : occupied)
            if (o.first == x && o.second == y) return true;
        return false;
        };

    for (int y = 1; y < map.getHeight() - 1; ++y)
        for (int x = 1; x < map.getWidth() - 1; ++x) {
            if (map.getTile(x, y).type != TileType::Grass) continue;

            bool ok = true;
            // Svi susjedi moraju biti Grass
            for (int dy = -1
                ; dy <= 1 && ok; ++dy)
                for (int dx = -1; dx <= 1 && ok; ++dx)
                    if (map.getTile(x + dx, y + dy).type != TileType::Grass)
                        ok = false;
            if (!ok) continue;

            // Spacing
            for (int dy = -4; dy <= 4 && ok; ++dy)
                for (int dx = -4; dx <= 4 && ok; ++dx)
                    if (isOccupied(x + dx, y + dy)) ok = false;
            if (!ok) continue;

            float density = getBiomeDef(map.getBiome(x, y)).floraDensity;
            if (spawnChance(rng) >= BASE_CHANCE * density) continue;

            m_bushes.push_back({ {x, y}, animOff(rng) });
            m_bushSet.insert(y * 10000 + x);
            occupied.push_back({ x, y });
        }
}

bool BushSystem::isInBush(sf::Vector2f pos) const {
    int tx = static_cast<int>(pos.x) / TileMap::TILE_SIZE;
    int ty = static_cast<int>(pos.y) / TileMap::TILE_SIZE;
    if (m_bushSet.count(ty * 10000 + tx) > 0) return true;
    if (m_bushSet.count(ty * 10000 + (tx + 1)) > 0) return true;
    if (m_bushSet.count((ty + 1) * 10000 + tx) > 0) return true;
    if (m_bushSet.count((ty + 1) * 10000 + (tx + 1)) > 0) return true;
    return false;
}

bool BushSystem::isSolid(sf::Vector2f pos) const {
    return isInBush(pos);
}

void BushSystem::draw(sf::RenderWindow& window) {
    float ts = static_cast<float>(TileMap::TILE_SIZE);
    float t = m_clock.getElapsedTime().asSeconds();
    sf::Sprite sprite(m_tex);
    sprite.setScale({ 2.8f, 2.8f });
    for (auto& b : m_bushes) {
        float wobble = std::sin(t * 1.5f + b.animOffset) * 2.f;
        sprite.setPosition({
            b.tilePos.x * ts - ts * 0.5f,
            b.tilePos.y * ts - ts * 0.9f
            });
        window.draw(sprite);
    }
}