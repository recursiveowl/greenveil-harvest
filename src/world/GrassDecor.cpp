#include "GrassDecor.h"

void GrassDecor::generate(const TileMap& map, unsigned seed, const std::unordered_set<int>& blockedTrees) {
    if (!m_tex.loadFromFile("assets/textures/grass_plants.png")) {
        printf("Failed to load grass_plants.png\n");
        return;
    }

    m_sprites.clear();
    m_verts.clear();
    m_verts.setPrimitiveType(sf::PrimitiveType::Triangles);

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> spawnChance(0.f, 1.f);
    constexpr float BASE_CHANCE = 0.2f;
    std::uniform_int_distribution<int> frameDist(0, 2);
    std::uniform_real_distribution<float> scaleDist(1.f, 1.8f);
    std::uniform_real_distribution<float> cellOffset(1.f, 8.f);

    auto allNeighboursGrass = [&](int x, int y) {
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx) {
                if (blockedTrees.count(y * 10000 + x)) continue;
                if (dx == 0 && dy == 0) continue;
                int nx = x + dx, ny = y + dy;
                if (nx < 0 || ny < 0 || nx >= map.getWidth() || ny >= map.getHeight())
                    return false;
                if (map.getTile(nx, ny).type != TileType::Grass)
                    return false;
            }
        return true;
        };

    constexpr float CELL = 16.f; // pola tile-a

    for (int y = 0; y < map.getHeight(); ++y)
        for (int x = 0; x < map.getWidth(); ++x) {
            if (map.getTile(x, y).type != TileType::Grass) continue;
            if (!allNeighboursGrass(x, y)) continue;
            float density = getBiomeDef(map.getBiome(x, y)).floraDensity;
            for (int cy = 0; cy < 2; ++cy)
                for (int cx = 0; cx < 2; ++cx) {
                    if (spawnChance(rng) >= BASE_CHANCE * density) continue;
                        m_sprites.push_back({
                            {
                                x * 32.f + cx * CELL + cellOffset(rng),
                                y * 32.f + cy * CELL + cellOffset(rng)
                            },
                            frameDist(rng), scaleDist(rng)
                            });
                }
        }

    constexpr float TS = 32.f, FS = 16.f;

    std::sort(m_sprites.begin(), m_sprites.end(),
        [](const GrassSprite& a, const GrassSprite& b) {
            return a.pos.y < b.pos.y;
        });

    for (auto& g : m_sprites) {
        float w = FS * g.scale, h = FS * g.scale;
        float ty = g.frame * FS;
        sf::Vertex v0({ g.pos.x,     g.pos.y }, sf::Color::White, { 0.f, ty });
        sf::Vertex v1({ g.pos.x + w, g.pos.y }, sf::Color::White, { FS,  ty });
        sf::Vertex v2({ g.pos.x + w, g.pos.y + h }, sf::Color::White, { FS,  ty + FS });
        sf::Vertex v3({ g.pos.x,     g.pos.y + h }, sf::Color::White, { 0.f, ty + FS });
        m_verts.append(v0); m_verts.append(v1); m_verts.append(v2);
        m_verts.append(v0); m_verts.append(v2); m_verts.append(v3);
    }

    printf("Generated grass sprites: %zu\n", m_sprites.size());
}

void GrassDecor::draw(sf::RenderWindow& window) {

    sf::RenderStates states;
    states.texture = &m_tex;

    window.draw(m_verts, states);
}