#include "world/TileMap.h"
#include "core/PlaceholderAssets.h"
#include <algorithm>

TileMap::TileMap(AssetManager& assets) : m_assets(assets) {
    static const char* kTileTextures[NUM_TILE_TYPES] = {
        "assets/textures/plains_grass.png",
        "assets/textures/plains_dirt.png",
        "assets/textures/water.png",
        "assets/textures/plains_sand.png",
        "assets/textures/plains_path.png"
    };

    for (int t = 0; t < NUM_TILE_TYPES; ++t) {
        TileLayer& layer = m_layers[layerIndex(static_cast<TileType>(t))];
        if (!layer.texture.loadFromFile(kTileTextures[t])) {
            switch (static_cast<TileType>(t)) {
            case TileType::Grass: layer.texture = PlaceholderAssets::makeGrass(); break;
            case TileType::Dirt:  layer.texture = PlaceholderAssets::makeDirt();  break;
            case TileType::Water: layer.texture = PlaceholderAssets::makeWater(); break;
            case TileType::Sand:  layer.texture = PlaceholderAssets::makeSand();  break;
            case TileType::Path:  layer.texture = PlaceholderAssets::makePath();  break;
            default: break;
            }
        }
        layer.texture.setRepeated(true);
        layer.texture.setSmooth(false);
    }
}

void TileMap::generate(int width, int height, unsigned seed) {
    m_width = width;
    m_height = height;
    m_tiles.resize(width * height);

    (void)seed; // seed se trenutno ne koristi - teren je fiksno Grass

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Tile& t = m_tiles[y * width + x];
            t.type = TileType::Grass;
            t.solid = false;
            t.fertile = false;
        }
    }
}

void TileMap::connectLandmarks(const std::vector<sf::Vector2i>& entryPoints,
    sf::Vector2i marketCenter, unsigned seed, int armWidth) {

    (void)seed; (void)armWidth; // vise se ne koriste

    for (const auto& entry : entryPoints) {
        // Pomjeri pocetnu tacku 4 tila dalje od slota (prema marketu)
        sf::Vector2i current = entry;
        sf::Vector2i target = marketCenter;

        int dx = target.x - current.x;
        int dy = target.y - current.y;
        float len = std::sqrt(static_cast<float>(dx * dx + dy * dy));
        if (len > 0.f) {
            current.x += static_cast<int>(dx / len * 4.f);
            current.y += static_cast<int>(dy / len * 4.f);
        }

        // Ravna linija od current do target, 1 tile sirina
        constexpr int MAX_STEPS = 2048;
        int steps = 0;

        while (steps < MAX_STEPS) {
            ++steps;

            dx = target.x - current.x;
            dy = target.y - current.y;
            if (std::abs(dx) <= 1 && std::abs(dy) <= 1) break;

            if (current.x >= 0 && current.y >= 0 &&
                current.x < m_width && current.y < m_height) {
                Tile& t = m_tiles[current.y * m_width + current.x];
                t.type = TileType::Path;
                t.solid = false;
            }

            if (std::abs(dy) >= std::abs(dx))
                current.y += (dy > 0) ? 1 : -1;
            else
                current.x += (dx > 0) ? 1 : -1;
        }

        // Garantuj spajanje sa market centrom
        if (target.x >= 0 && target.y >= 0 &&
            target.x < m_width && target.y < m_height) {
            Tile& t = m_tiles[target.y * m_width + target.x];
            t.type = TileType::Path;
            t.solid = false;
        }
    }
}

void TileMap::setTileType(int x, int y, TileType type) {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) return;
    Tile& t = m_tiles[y * m_width + x];
    t.type = type;
    t.solid = (type == TileType::Water);
    t.fertile = false;
}

void TileMap::draw(sf::RenderWindow& window) {
    sf::View view = window.getView();
    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    int startX = std::max(0, (int)((center.x - size.x * 0.5f) / TILE_SIZE) - 1);
    int startY = std::max(0, (int)((center.y - size.y * 0.5f) / TILE_SIZE) - 1);
    int endX = std::min(m_width, (int)((center.x + size.x * 0.5f) / TILE_SIZE) + 2);
    int endY = std::min(m_height, (int)((center.y + size.y * 0.5f) / TILE_SIZE) + 2);

    for (auto& layer : m_layers) {
        layer.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
        layer.vertices.clear();
    }

    float ts = static_cast<float>(TILE_SIZE);

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const Tile& t = m_tiles[y * m_width + x];
            auto& va = m_layers[layerIndex(t.type)].vertices;

            float px = static_cast<float>(x * TILE_SIZE);
            float py = static_cast<float>(y * TILE_SIZE);

            sf::Color col = sf::Color::White;
            if (t.type == TileType::Dirt && t.fertile)
                col = sf::Color(180, 255, 180);

            sf::Vector2f uv0{ 0.f, 0.f };
            sf::Vector2f uv1{ ts,  0.f };
            sf::Vector2f uv2{ 0.f, ts };
            sf::Vector2f uv3{ ts,  ts };

            va.append({ {px,    py   }, col, uv0 });
            va.append({ {px + ts, py   }, col, uv1 });
            va.append({ {px,    py + ts}, col, uv2 });
            va.append({ {px + ts, py   }, col, uv1 });
            va.append({ {px + ts, py + ts}, col, uv3 });
            va.append({ {px,    py + ts}, col, uv2 });
        }
    }

    for (auto& layer : m_layers) {
        if (layer.vertices.getVertexCount() == 0) continue;
        sf::RenderStates states;
        states.texture = &layer.texture;
        window.draw(layer.vertices, states);
    }
}

const Tile& TileMap::getTile(int x, int y) const {
    static Tile dummy;
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) return dummy;
    return m_tiles[y * m_width + x];
}

Tile& TileMap::getTileMut(int x, int y) {
    static Tile dummy;
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) return dummy;
    return m_tiles[y * m_width + x];
}