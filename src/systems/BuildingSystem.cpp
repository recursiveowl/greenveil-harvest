#include "systems/BuildingSystem.h"
#include <cmath>
#include <algorithm>

BuildingSystem::BuildingSystem() {
    // ---- Wood buildings ----
    m_defs.push_back({
        BuildingType::WoodWall, "Wood Wall",
        "Basic wall. Stops enemies and wind.",
        ItemType::Wood, 2, {1, 1},
        sf::Color(160, 100, 45), true, 80.f,
        50.f,   // 50 kg
        2000.f  // 2 kN
        });
    m_defs.push_back({
        BuildingType::WoodFloor, "Wood Floor",
        "Flat surface. Can be walked on.",
        ItemType::Wood, 1, {1, 1},
        sf::Color(180, 130, 70), false, 60.f,
        30.f, 1500.f
        });
    m_defs.push_back({
        BuildingType::WoodDoor, "Wood Door",
        "Passable opening in a wall.",
        ItemType::Wood, 3, {1, 2},
        sf::Color(140, 85, 30), false, 50.f,
        40.f, 800.f
        });
    m_defs.push_back({
        BuildingType::WoodFence, "Wood Fence",
        "Low barrier. Keeps animals in.",
        ItemType::Wood, 1, {1, 1},
        sf::Color(190, 150, 80), true, 30.f,
        20.f, 500.f
        });
    m_defs.push_back({
        BuildingType::WoodRoof, "Wood Roof",
        "Provides shelter from rain.",
        ItemType::Wood, 4, {2, 1},
        sf::Color(120, 70, 25), false, 70.f,
        80.f, 3000.f
        });
    m_defs.push_back({
        BuildingType::WoodTower, "Wood Tower",
        "Tall structure. Good vantage point.",
        ItemType::Wood, 8, {2, 3},
        sf::Color(150, 95, 40), true, 200.f,
        200.f, 8000.f
        });

    // ---- Stone buildings ----
    m_defs.push_back({
        BuildingType::StoneWall, "Stone Wall",
        "Heavy wall. Very durable.",
        ItemType::Stone, 3, {1, 1},
        sf::Color(130, 130, 140), true, 300.f,
        500.f,  // 500 kg
        15000.f // 15 kN
        });
    m_defs.push_back({
        BuildingType::StoneFloor, "Stone Floor",
        "Solid ground. Very stable.",
        ItemType::Stone, 2, {1, 1},
        sf::Color(150, 150, 160), false, 250.f,
        400.f, 20000.f
        });
    m_defs.push_back({
        BuildingType::StoneTower, "Stone Tower",
        "Fortress tower. Near indestructible.",
        ItemType::Stone, 15, {2, 3},
        sf::Color(110, 110, 120), true, 800.f,
        2000.f, 50000.f
        });
    m_defs.push_back({
        BuildingType::StoneFoundation, "Stone Foundation",
        "Base for any structure. Spreads load.",
        ItemType::Stone, 4, {2, 2},
        sf::Color(120, 120, 130), true, 400.f,
        800.f, 30000.f
        });
}

const BuildingDef& BuildingSystem::getDef(BuildingType t) const {
    for (auto& d : m_defs)
        if (d.type == t) return d;
    return m_defs[0];
}

bool BuildingSystem::overlapsExisting(sf::Vector2i tilePos,
    sf::Vector2i size) const {
    for (auto& b : m_buildings) {
        if (b.destroyed) continue;
        // AABB provjera
        bool overlapX = tilePos.x < b.tilePos.x + b.size.x &&
            tilePos.x + size.x > b.tilePos.x;
        bool overlapY = tilePos.y < b.tilePos.y + b.size.y &&
            tilePos.y + size.y > b.tilePos.y;
        if (overlapX && overlapY) return true;
    }
    return false;
}

bool BuildingSystem::canPlace(BuildingType type, sf::Vector2i tilePos,
    const Inventory& inv,
    const TileMap& map) const {
    const BuildingDef& def = getDef(type);

    // Provjeri materijal
    if (inv.countOf(def.material) < def.cost) return false;

    // Provjeri da nije na vodi ili kamenu
    for (int dy = 0; dy < def.size.y; ++dy)
        for (int dx = 0; dx < def.size.x; ++dx) {
            const Tile& t = map.getTile(tilePos.x + dx, tilePos.y + dy);
            if (t.type == TileType::Water) return false;
        }

    // Provjeri preklapanje
    if (overlapsExisting(tilePos, def.size)) return false;

    return true;
}

bool BuildingSystem::tryPlace(BuildingType type, sf::Vector2i tilePos,
    Inventory& inv, const TileMap& map) {
    if (!canPlace(type, tilePos, inv, map)) return false;

    const BuildingDef& def = getDef(type);
    inv.removeItem(def.material, def.cost);

    PlacedBuilding b;
    b.type = type;
    b.tilePos = tilePos;
    b.size = def.size;
    b.color = def.color;
    b.solid = def.solid;
    b.hp = def.hp;
    b.maxHp = def.hp;
    m_buildings.push_back(b);
    return true;
}

bool BuildingSystem::tryDestroy(sf::Vector2f worldPos, Inventory& inv) {
    int tx = static_cast<int>(worldPos.x) / TileMap::TILE_SIZE;
    int ty = static_cast<int>(worldPos.y) / TileMap::TILE_SIZE;

    for (auto& b : m_buildings) {
        if (b.destroyed) continue;
        if (tx >= b.tilePos.x && tx < b.tilePos.x + b.size.x &&
            ty >= b.tilePos.y && ty < b.tilePos.y + b.size.y) {
            b.destroyed = true;
            // Vrati polovinu materijala
            const BuildingDef& def = getDef(b.type);
            int refund = std::max(1, def.cost / 2);
            inv.addItem(def.material, refund);
            return true;
        }
    }
    return false;
}

bool BuildingSystem::isSolid(sf::Vector2f worldPos) const {
    return isSolid(
        static_cast<int>(worldPos.x) / TileMap::TILE_SIZE,
        static_cast<int>(worldPos.y) / TileMap::TILE_SIZE);
}

bool BuildingSystem::isSolid(int tileX, int tileY) const {
    for (auto& b : m_buildings) {
        if (b.destroyed || !b.solid) continue;
        if (tileX >= b.tilePos.x && tileX < b.tilePos.x + b.size.x &&
            tileY >= b.tilePos.y && tileY < b.tilePos.y + b.size.y)
            return true;
    }
    return false;
}

void BuildingSystem::update(float dt) {
    // Ukloni unistene zgrade
    m_buildings.erase(
        std::remove_if(m_buildings.begin(), m_buildings.end(),
            [](const PlacedBuilding& b) { return b.destroyed; }),
        m_buildings.end());
}

void BuildingSystem::drawBuilding(sf::RenderWindow& window, const PlacedBuilding& b) const {
    float ts = static_cast<float>(TileMap::TILE_SIZE);
    float wx = b.tilePos.x * ts;
    float wy = b.tilePos.y * ts;
    float ww = b.size.x * ts;
    float wh = b.size.y * ts;

    // Tijelo
    sf::RectangleShape body({ ww, wh });
    body.setPosition({ wx, wy });
    body.setFillColor(b.color);
    body.setOutlineThickness(2.f);
    body.setOutlineColor(sf::Color(
        static_cast<uint8_t>(b.color.r * 0.6f),
        static_cast<uint8_t>(b.color.g * 0.6f),
        static_cast<uint8_t>(b.color.b * 0.6f)));
    window.draw(body);

    // HP bar ako je ostecena
    if (b.hp < b.maxHp) {
        float ratio = b.hp / b.maxHp;
        sf::RectangleShape barBg({ ww, 4.f });
        barBg.setPosition({ wx, wy - 6.f });
        barBg.setFillColor(sf::Color(60, 0, 0));
        window.draw(barBg);
        sf::RectangleShape barFg({ ww * ratio, 4.f });
        barFg.setPosition({ wx, wy - 6.f });
        barFg.setFillColor(sf::Color(200, 100, 50));
        window.draw(barFg);
    }
}

void BuildingSystem::collectRenderItems(std::vector<RenderItem>& out) const {
    float ts = static_cast<float>(TileMap::TILE_SIZE);
    for (const auto& b : m_buildings) {
        if (b.destroyed) continue;
        float sortY = (b.tilePos.y + b.size.y) * ts; // donja ivica zgrade = "baza"
        out.push_back({ sortY, [this, &b](sf::RenderWindow& w) { drawBuilding(w, b); } });
    }
}

void BuildingSystem::drawPreview(sf::RenderWindow& window,
    BuildingType type,
    sf::Vector2i tilePos,
    bool valid) {
    const BuildingDef& def = getDef(type);
    float ts = static_cast<float>(TileMap::TILE_SIZE);
    float wx = tilePos.x * ts;
    float wy = tilePos.y * ts;
    float ww = def.size.x * ts;
    float wh = def.size.y * ts;

    sf::Color previewCol = valid
        ? sf::Color(def.color.r, def.color.g, def.color.b, 120)
        : sf::Color(200, 50, 50, 100);
    sf::Color outlineCol = valid
        ? sf::Color(150, 255, 150, 200)
        : sf::Color(255, 80, 80, 200);

    sf::RectangleShape preview({ ww, wh });
    preview.setPosition({ wx, wy });
    preview.setFillColor(previewCol);
    preview.setOutlineThickness(2.f);
    preview.setOutlineColor(outlineCol);
    window.draw(preview);

    // Grid linije unutar previewua
    for (int dy = 0; dy < def.size.y; ++dy)
        for (int dx = 0; dx < def.size.x; ++dx) {
            sf::RectangleShape grid({ ts - 2.f, ts - 2.f });
            grid.setPosition({ wx + dx * ts + 1.f, wy + dy * ts + 1.f });
            grid.setFillColor(sf::Color::Transparent);
            grid.setOutlineThickness(1.f);
            grid.setOutlineColor(sf::Color(255, 255, 255, 40));
            window.draw(grid);
        }
}