#pragma once
#include "TileMap.h"
#include <unordered_set>

class BushSystem {
public:
    void generate(const TileMap& map, unsigned seed, const std::unordered_set<int>& blocked);
    bool isSolid(sf::Vector2f pos) const;
    bool isInBush(sf::Vector2f pos) const;
    void draw(sf::RenderWindow& window);

    const std::unordered_set<int>& getTileSet() const { return m_bushSet; }

private:
    struct Bush {
        sf::Vector2i tilePos;
        float        animOffset{ 0.f };
    };
    std::vector<Bush> m_bushes;
    sf::Texture       m_tex;
    sf::Clock         m_clock;
    std::unordered_set<int> m_bushSet;
};