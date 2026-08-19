#pragma once
#include "TileMap.h"
#include "core/RenderQueue.h"
#include <unordered_set>

struct Tree { sf::Vector2i tilePos; };

class TreeSystem {
public:
    void generate(const TileMap& map, unsigned seed, const std::unordered_set<int>& blocked);
    bool isSolid(sf::Vector2f pos) const;
    const std::unordered_set<int>& getTileSet() const { return m_treeSet; }
    void collectRenderItems(std::vector<RenderItem>& out) const;
private:
    void drawTree(sf::RenderWindow& window, const Tree& t) const;

    std::vector<Tree>       m_trees;
    sf::Texture             m_tex;
    std::unordered_set<int> m_treeSet;
    sf::Clock               m_clock;
};