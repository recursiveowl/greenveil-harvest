#pragma once
#include "TileMap.h"
#include <unordered_set>

struct Tree { sf::Vector2i tilePos; };

class TreeSystem {
public:
    void generate(const TileMap& map, unsigned seed, const std::unordered_set<int>& blocked);
    bool isSolid(sf::Vector2f pos) const;
    const std::unordered_set<int>& getTileSet() const { return m_treeSet; }
    void draw(sf::RenderWindow& window);
private:
    std::vector<Tree>       m_trees;
    sf::Texture             m_tex;
    std::unordered_set<int> m_treeSet;
    sf::Clock               m_clock;
};