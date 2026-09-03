#include "world/WellSystem.h"
#include "world/TileMap.h"
#include <cmath>

void WellSystem::place(sf::Vector2i marketPos, sf::Vector2i marketSize) {
    m_tilePos = { marketPos.x - 3, marketPos.y + marketSize.y / 2 };
}

sf::Vector2f WellSystem::getWorldPos() const {
    float ts = static_cast<float>(TileMap::TILE_SIZE);
    return {
        m_tilePos.x * ts + ts * 0.5f,
        m_tilePos.y * ts + ts * 0.5f
    };
}

bool WellSystem::isNear(sf::Vector2f playerPos, float range) const {
    sf::Vector2f wp = getWorldPos();
    float dx = playerPos.x - wp.x;
    float dy = playerPos.y - wp.y;
    return std::sqrt(dx * dx + dy * dy) <= range;
}

void WellSystem::draw(sf::RenderWindow& window) const {
    float ts = static_cast<float>(TileMap::TILE_SIZE);
    float size = ts * 1.5f;

    sf::CircleShape well(size * 0.5f);
    well.setOrigin({ size * 0.5f, size * 0.5f });
    well.setPosition(getWorldPos());
    well.setFillColor(sf::Color(90, 130, 170, 220));
    well.setOutlineThickness(2.f);
    well.setOutlineColor(sf::Color(50, 80, 120));
    window.draw(well);
}