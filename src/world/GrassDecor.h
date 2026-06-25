#pragma once
#include "TileMap.h"
#include <unordered_set>

struct GrassSprite {
    sf::Vector2f pos;
    int          frame;
    float        scale;
};

class GrassDecor {
public:
    void generate(const TileMap& map, unsigned seed, const std::unordered_set<int>& blockedTrees);
    void draw(sf::RenderWindow& window);
private:
    std::vector<GrassSprite> m_sprites;
    sf::Texture              m_tex;
    sf::VertexArray m_verts;
};