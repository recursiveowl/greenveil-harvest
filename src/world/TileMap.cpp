#include "world/TileMap.h"
#include "world/Noise.h"
#include "core/PlaceholderAssets.h"
#include <algorithm>

TileMap::TileMap(AssetManager& assets) : m_assets(assets) {
    // Grass — prava tekstura
    if (!m_layers[0].texture.loadFromFile("assets/textures/grass.png"))
        m_layers[0].texture = PlaceholderAssets::makeGrass();
    m_layers[0].texture.setRepeated(true);
    m_layers[0].texture.setSmooth(false);

    // Ostale — placeholder za sada
    m_layers[1].texture = PlaceholderAssets::makeDirt();
    m_layers[2].texture = PlaceholderAssets::makeWater();
    m_layers[3].texture = PlaceholderAssets::makeStone();
}

bool TileMap::isNearWater(int x, int y, int radius) const {
    for (int dy = -radius; dy <= radius; ++dy)
        for (int dx = -radius; dx <= radius; ++dx) {
            int nx = x + dx, ny = y + dy;
            if (nx < 0 || ny < 0 || nx >= m_width || ny >= m_height) continue;
            if (m_tiles[ny * m_width + nx].type == TileType::Water) return true;
        }
    return false;
}

void TileMap::generate(int width, int height, unsigned seed) {
    m_width = width;
    m_height = height;
    m_tiles.resize(width * height);

    PerlinNoise noise(seed);
    PerlinNoise moisture(seed + 100);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Tile& t = m_tiles[y * width + x];
            float elevation = noise.octave(x * 0.03f, y * 0.03f, 6, 0.5f);
            float wet = moisture.octave(x * 0.05f, y * 0.05f, 4, 0.6f);

            if (elevation < 0.35f) { t.type = TileType::Water; t.solid = true; }
            else if (elevation < 0.45f || wet > 0.7f) { t.type = TileType::Dirt;  t.solid = false; }
            else if (elevation > 0.75f) { t.type = TileType::Stone; t.solid = true; }
            else { t.type = TileType::Grass; t.solid = false; }
        }
    }

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            Tile& t = m_tiles[y * width + x];
            if (t.type == TileType::Dirt && isNearWater(x, y, 4))
                t.fertile = true;
        }
}

void TileMap::setTileType(int x, int y, TileType type) {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) return;
    Tile& t = m_tiles[y * m_width + x];
    t.type = type;
    t.solid = (type == TileType::Water || type == TileType::Stone);
    t.fertile = (type == TileType::Dirt && isNearWater(x, y, 4));
}

void TileMap::draw(sf::RenderWindow& window) {
    sf::View view = window.getView();
    sf::Vector2f center = view.getCenter();
    sf::Vector2f size = view.getSize();

    int padding = 1;
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
            auto& va = m_layers[static_cast<int>(t.type)].vertices;

            float px = static_cast<float>(x * TILE_SIZE);
            float py = static_cast<float>(y * TILE_SIZE);

            sf::Color col = sf::Color::White;
            if (t.type == TileType::Dirt && t.fertile)
                col = sf::Color(180, 255, 180);

            // UV koordinate — tile texture je 32x32, tile je 32x32
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