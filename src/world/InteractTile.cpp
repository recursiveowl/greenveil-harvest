#include "pch.h"
#include "world/InteractTile.h"
#include "world/TileMap.h"
#include <cmath>

static float dist(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

void InteractTileManager::add(InteractTile tile) {
    m_tiles.push_back(tile);
}

void InteractTileManager::update(sf::Vector2f playerPos) {
    m_nearest = nullptr;
    float bestDist = RANGE;
    for (auto& t : m_tiles) {
        if (t.discovered && t.oneShot) continue;
        sf::Vector2f wp{
            t.tilePos.x * static_cast<float>(TileMap::TILE_SIZE) + TileMap::TILE_SIZE * 0.5f,
            t.tilePos.y * static_cast<float>(TileMap::TILE_SIZE) + TileMap::TILE_SIZE * 0.5f
        };
        float d = dist(playerPos, wp);
        if (d < bestDist) { bestDist = d; m_nearest = &t; }
    }
}

bool InteractTileManager::tryInteract(sf::Vector2f playerPos,
    std::string& outTitle,
    std::string& outBody) {
    if (!m_nearest) return false;
    outTitle = m_nearest->title;
    outBody = m_nearest->body;
    if (m_nearest->oneShot) m_nearest->discovered = true;
    return true;
}

void InteractTileManager::draw(sf::RenderWindow& w) {
    float ts = static_cast<float>(TileMap::TILE_SIZE);

    for (auto& t : m_tiles) {
        if (t.discovered && t.oneShot) continue;
        sf::RectangleShape hl({ ts, ts });
        hl.setPosition({ t.tilePos.x * ts, t.tilePos.y * ts });
        hl.setFillColor(sf::Color(60, 140, 255, 60));
        hl.setOutlineThickness(2.f);
        hl.setOutlineColor(sf::Color(60, 140, 255, 200));
        w.draw(hl);
    }
}