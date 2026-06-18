#include "TreeSystem.h"

void TreeSystem::generate(const TileMap& map, unsigned seed, const std::unordered_set<int>& blocked) {

    m_tex.loadFromFile("assets/textures/pl_tree.png");

    m_trees.clear();
    m_treeSet.clear();
    std::mt19937 rng(seed + 1337);
    std::uniform_int_distribution<int> roll(0, 190);

    std::vector<std::pair<int, int>> occupied;

    for (int y = 1; y < map.getHeight() - 1; ++y)
        for (int x = 1; x < map.getWidth() - 1; ++x) {
            if (map.getTile(x, y).type != TileType::Grass) continue;

            bool ok = true;
            for (int dy = -1; dy <= 1 && ok; ++dy)
                for (int dx = -1; dx <= 1 && ok; ++dx)
                    if (map.getTile(x + dx, y + dy).type != TileType::Grass) ok = false;
            if (!ok) continue;

            for (int dy = -4; dy <= 4 && ok; ++dy)
                for (int dx = -4; dx <= 4 && ok; ++dx) {
                    int key = (y + dy) * 10000 + (x + dx);
                    if (blocked.count(key)) ok = false;
                    for (auto& o : occupied)
                        if (o.first == x + dx && o.second == y + dy) ok = false;
                }
            if (!ok) continue;
            if (roll(rng) != 0) continue;

            m_trees.push_back({ {x,y} });
            m_treeSet.insert(y * 10000 + x);
            occupied.push_back({ x,y });
        }
}

bool TreeSystem::isSolid(sf::Vector2f pos) const {
    for (auto& t : m_trees) {
        float tx = t.tilePos.x * 32.f + 8.f;
        float ty = t.tilePos.y * 32.f - 64.f;
        float w = 16.f, h = 76.f;
        // Krošnja — oval
        float cx = t.tilePos.x * 32.f + 16.f;  // centar krošnje X
        float cy = t.tilePos.y * 32.f - 80.f;  // centar krošnje Y
        float rx = 42.f, ry = 48.f;
        float dx = (pos.x - cx) / rx;
        float dy = (pos.y - cy) / ry;
        if (dx * dx + dy * dy <= 1.f) return true;
        if (pos.x >= tx && pos.x <= tx + w &&
            pos.y >= ty && pos.y <= ty + h)
            return true;
    }
    return false;
}

void TreeSystem::draw(sf::RenderWindow& window) {
    sf::Sprite sprite(m_tex);
    float ts = (float)TileMap::TILE_SIZE;
    for (auto& t : m_trees) {
        sprite.setScale({ 3.f, 3.f });
        sprite.setOrigin({ 16.f, 48.f }); 
        sprite.setPosition({ t.tilePos.x * ts + 16.f, t.tilePos.y * ts });
        window.draw(sprite);
    }
}