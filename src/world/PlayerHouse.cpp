#include "world/PlayerHouse.h"
#include <random>
#include <cmath>

static constexpr float PI = 3.14159265f;

void PlayerHouseSystem::spawnNear(sf::Vector2f playerPos, unsigned seed, const TileMap& map) {
    std::mt19937 rng(seed + 777); // posebna so da se ne poklapa sa drugim seed-based generacijama

    // Ugao ograničen na uski raspon oko "gore" (-90°), da kuća uvijek
    // bude sjeverno od igrača i da igrač gleda u prednju fasadu na spawn-u.
    constexpr float UP_ANGLE = -PI * 0.5f;
    constexpr float SPREAD = PI * 0.12f; // ±~21° oko "gore"
    std::uniform_real_distribution<float> angleDist(UP_ANGLE - SPREAD, UP_ANGLE + SPREAD);
    std::uniform_real_distribution<float> radiusDist(9.f, 13.f);

    int playerTX = static_cast<int>(playerPos.x) / TileMap::TILE_SIZE;
    int playerTY = static_cast<int>(playerPos.y) / TileMap::TILE_SIZE;

    constexpr int MAX_ATTEMPTS = 30;
    bool found = false;
    sf::Vector2i bestPos{ playerTX, playerTY - 6 }; // fallback ako sve probe padnu

    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        float angle = angleDist(rng);
        float radius = radiusDist(rng);

        int candX = playerTX + static_cast<int>(std::cos(angle) * radius);
        int candY = playerTY + static_cast<int>(std::sin(angle) * radius);

        bool ok = true;
        for (int dy = 0; dy < m_house.size.y && ok; ++dy)
            for (int dx = 0; dx < m_house.size.x && ok; ++dx) {
                const Tile& t = map.getTile(candX + dx, candY + dy);
                if (t.solid || t.type == TileType::Water) ok = false;
            }

        if (ok) {
            bestPos = { candX, candY };
            found = true;
            break;
        }
    }

    m_house.tilePos = bestPos;
    m_house.placed = true;
    (void)found;
}

void PlayerHouseSystem::setTilePos(sf::Vector2i tilePos) {
    m_house.tilePos = tilePos;
    m_house.placed = true;
}

bool PlayerHouseSystem::isSolid(sf::Vector2f worldPos) const {
    if (!m_house.placed) return false;

    int tx = static_cast<int>(worldPos.x) / TileMap::TILE_SIZE;
    int ty = static_cast<int>(worldPos.y) / TileMap::TILE_SIZE;

    return tx >= m_house.tilePos.x && tx < m_house.tilePos.x + m_house.size.x &&
        ty >= m_house.tilePos.y + 1 && ty < m_house.tilePos.y + m_house.size.y;
}

void PlayerHouseSystem::draw(sf::RenderWindow& window) {
    if (!m_house.placed) return;

    if (m_tex.getSize().x == 0)
        (void)m_tex.loadFromFile("assets/textures/house.png");

    float ts = static_cast<float>(TileMap::TILE_SIZE);
    float wx = m_house.tilePos.x * ts;
    float wy = m_house.tilePos.y * ts;
    float ww = m_house.size.x * ts;
    float wh = m_house.size.y * ts;

    sf::Sprite sprite(m_tex);
    sf::Vector2u texSize = m_tex.getSize();
    sprite.setScale({ ww / static_cast<float>(texSize.x), wh / static_cast<float>(texSize.y) });
    sprite.setPosition({ wx, wy });
    window.draw(sprite);
}

bool PlayerHouseSystem::isNearDoor(sf::Vector2f playerPos) const {
    if (!m_house.placed) return false;
    sf::Vector2f doorPos = getFrontOfDoorPos();
    float dx = playerPos.x - doorPos.x, dy = playerPos.y - doorPos.y;
    return std::sqrt(dx * dx + dy * dy) < 48.f;
}

sf::Vector2f PlayerHouseSystem::getFrontOfDoorPos() const {
    float ts = static_cast<float>(TileMap::TILE_SIZE);
    float doorX = (m_house.tilePos.x + m_house.size.x * 0.5f) * ts;
    float doorY = (m_house.tilePos.y + m_house.size.y) * ts + 16.f;
    return { doorX, doorY };
}

bool PlayerHouseSystem::isAtDoorTrigger(sf::Vector2f playerPos) const {
    if (!m_house.placed) return false;

    float ts = static_cast<float>(TileMap::TILE_SIZE);
    float doorCenterX = (m_house.tilePos.x + m_house.size.x * 0.5f) * ts;
    float doorTopY = (m_house.tilePos.y + m_house.size.y - 1) * ts; // gornja ivica prolaza (unutar footprinta)
    float doorBottomY = (m_house.tilePos.y + m_house.size.y) * ts + 12.f; // malo ispod kuće

    return playerPos.x >= doorCenterX - 16.f && playerPos.x <= doorCenterX + 16.f &&
        playerPos.y >= doorTopY && playerPos.y <= doorBottomY;
}

static void ensureInteriorLoaded(sf::Texture& tex) {
    if (tex.getSize().x == 0)
        (void)tex.loadFromFile("assets/textures/house_int.png");
}

sf::Vector2f PlayerHouseSystem::getInteriorSize() const {
    ensureInteriorLoaded(const_cast<sf::Texture&>(m_texInt));
    sf::Vector2u sz = m_texInt.getSize();
    return { static_cast<float>(sz.x) * INTERIOR_SCALE, static_cast<float>(sz.y) * INTERIOR_SCALE };
}

sf::Vector2f PlayerHouseSystem::getInteriorSpawnPos() const {
    sf::Vector2f size = getInteriorSize();
    return { size.x * 0.5f, size.y * 0.75f };
}

bool PlayerHouseSystem::isWithinInteriorBounds(sf::Vector2f localPos) const {
    sf::Vector2f size = getInteriorSize();
    constexpr float MARGIN = 24.f;
    return localPos.x >= MARGIN && localPos.y >= MARGIN &&
        localPos.x <= size.x - MARGIN && localPos.y <= size.y - MARGIN;
}

void PlayerHouseSystem::drawInterior(sf::RenderWindow& window) {
    ensureInteriorLoaded(m_texInt);
    sf::Sprite sprite(m_texInt);
    sprite.setScale({ INTERIOR_SCALE, INTERIOR_SCALE });
    window.draw(sprite);
}

std::unordered_set<int> PlayerHouseSystem::getOccupiedTileSet() const {
    std::unordered_set<int> set;
    if (!m_house.placed) return set;
    for (int dy = 0; dy < m_house.size.y; ++dy)
        for (int dx = 0; dx < m_house.size.x; ++dx) {
            int x = m_house.tilePos.x + dx;
            int y = m_house.tilePos.y + dy;
            set.insert(y * 10000 + x); // isti key-encoding kao TreeSystem/BushSystem
        }
    return set;
}