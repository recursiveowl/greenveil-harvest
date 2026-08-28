#include "world/MapSystem.h"
#include "world/TileMap.h"
#include "world/TreeSystem.h"
#include "world/BushSystem.h"
#include "world/LandmarkSystem.h"

const sf::Texture& MapSystem::getTexture() const {
    static sf::Texture empty;
    if (!m_renderTex) return empty;
    return m_renderTex->getTexture();
}

static sf::Color colorForTile(TileType t) {
    switch (t) {
    case TileType::Grass: return sf::Color(74, 138, 62);
    case TileType::Dirt:  return sf::Color(122, 92, 58);
    case TileType::Water: return sf::Color(54, 96, 168);
    case TileType::Sand:  return sf::Color(212, 190, 140);
    case TileType::Path:  return sf::Color(214, 182, 112);
    default:              return sf::Color(80, 80, 80);
    }
}

void MapSystem::bake(const TileMap& map, const TreeSystem& trees,
    const BushSystem& bushes, const LandmarkSystem& landmarks) {
    int w = map.getWidth();
    int h = map.getHeight();

    m_texSize = { static_cast<unsigned>(w * PIXELS_PER_TILE),
                  static_cast<unsigned>(h * PIXELS_PER_TILE) };

    m_renderTex = std::make_unique<sf::RenderTexture>(m_texSize);
    m_renderTex->clear(sf::Color(30, 30, 30));

    // ---- Bazni teren ----
    sf::VertexArray verts(sf::PrimitiveType::Triangles);
    m_roadSet.clear();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            TileType type = map.getTile(x, y).type;
            sf::Color c = colorForTile(type);

            if (type == TileType::Path)
                m_roadSet.insert(y * 10000 + x);

            float px = x * PIXELS_PER_TILE;
            float py = y * PIXELS_PER_TILE;

            sf::Vertex v0({ px,                  py }, c);
            sf::Vertex v1({ px + PIXELS_PER_TILE, py }, c);
            sf::Vertex v2({ px + PIXELS_PER_TILE, py + PIXELS_PER_TILE }, c);
            sf::Vertex v3({ px,                  py + PIXELS_PER_TILE }, c);

            verts.append(v0); verts.append(v1); verts.append(v2);
            verts.append(v0); verts.append(v2); verts.append(v3);
        }
    }
    m_renderTex->draw(verts);

    // ---- Puteljci — drugi, sireniji prolaz da budu jasniji/manje tanki ----
    sf::VertexArray roadOverlay(sf::PrimitiveType::Triangles);
    sf::Color roadCol(230, 200, 130);
    float pad = PIXELS_PER_TILE * 0.5f;
    for (int key : m_roadSet) {
        int rx = key % 10000;
        int ry = key / 10000;
        float px = rx * PIXELS_PER_TILE - pad;
        float py = ry * PIXELS_PER_TILE - pad;
        float sizePx = PIXELS_PER_TILE + pad * 2.f;

        sf::Vertex v0({ px,          py }, roadCol);
        sf::Vertex v1({ px + sizePx, py }, roadCol);
        sf::Vertex v2({ px + sizePx, py + sizePx }, roadCol);
        sf::Vertex v3({ px,          py + sizePx }, roadCol);

        roadOverlay.append(v0); roadOverlay.append(v1); roadOverlay.append(v2);
        roadOverlay.append(v0); roadOverlay.append(v2); roadOverlay.append(v3);
    }
    m_renderTex->draw(roadOverlay);

    // ---- Drveće ----
    sf::Color treeCol(30, 70, 30, 220);
    for (int key : trees.getTileSet()) {
        int tx = key % 10000;
        int ty = key / 10000;
        sf::CircleShape dot(PIXELS_PER_TILE * 0.9f);
        dot.setOrigin({ dot.getRadius(), dot.getRadius() });
        dot.setPosition({ tx * PIXELS_PER_TILE + PIXELS_PER_TILE * 0.5f,
                           ty * PIXELS_PER_TILE + PIXELS_PER_TILE * 0.5f });
        dot.setFillColor(treeCol);
        m_renderTex->draw(dot);
    }

    // ---- Žbunje ----
    sf::Color bushCol(50, 110, 50, 200);
    for (int key : bushes.getTileSet()) {
        int bx = key % 10000;
        int by = key / 10000;
        sf::CircleShape dot(PIXELS_PER_TILE * 0.5f);
        dot.setOrigin({ dot.getRadius(), dot.getRadius() });
        dot.setPosition({ bx * PIXELS_PER_TILE + PIXELS_PER_TILE * 0.5f,
                           by * PIXELS_PER_TILE + PIXELS_PER_TILE * 0.5f });
        dot.setFillColor(bushCol);
        m_renderTex->draw(dot);
    }

    m_renderTex->display();

    // ---- Landmark markeri (Market / ForSale / NPC ranch / Player kuca) ----
    m_markers.clear();

    float ts = static_cast<float>(TileMap::TILE_SIZE);

    sf::Vector2i marketPos = landmarks.getMarketPos();
    sf::Vector2i marketSize = landmarks.getMarketSize();
    sf::Vector2f marketCenter{
        (marketPos.x + marketSize.x * 0.5f) * ts,
        (marketPos.y + marketSize.y * 0.5f) * ts };
    m_markers.push_back({ marketCenter, MapMarkerType::Market });

    for (const auto& slot : landmarks.getSlots()) {
        sf::Vector2f center{
            (slot.tilePos.x + LandmarkSystem::SLOT_WIDTH * 0.5f) * ts,
            (slot.tilePos.y + LandmarkSystem::SLOT_HEIGHT * 0.5f) * ts };

        if (slot.role == LandmarkRole::ForSale)
            m_markers.push_back({ center, MapMarkerType::ForSale });
        else if (slot.role == LandmarkRole::NPC)
            m_markers.push_back({ center, MapMarkerType::NPCRanch });
        else if (slot.role == LandmarkRole::Player)
            m_markers.push_back({ center, MapMarkerType::PlayerHouse });
    }

    m_baked = true;
}