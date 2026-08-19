#pragma once
#include "pch.h"
#include "systems/Inventory.h"
#include "world/TileMap.h"
#include "core/RenderQueue.h"

// ========================= BUILDING TYPES =========================

enum class BuildingType : uint8_t {
    // Wood
    WoodWall = 0,
    WoodFloor,
    WoodDoor,
    WoodFence,
    WoodRoof,
    WoodTower,
    // Stone
    StoneWall,
    StoneFloor,
    StoneTower,
    StoneFoundation,
    COUNT
};

// ========================= BUILDING DEF =========================

struct BuildingDef {
    BuildingType type;
    std::string  name;
    std::string  description;
    ItemType     material;
    int          cost;         // koliko materijala treba
    sf::Vector2i size;         // u tilesima (w, h)
    sf::Color    color;
    bool         solid;        // blokira prolaz
    float        hp;           // max HP
    // Fizika / edukacija
    float        mass;         // kg (za edukaciju)
    float        strength;     // N koliko moze izdrzati
};

// ========================= PLACED BUILDING =========================

struct PlacedBuilding {
    BuildingType type;
    sf::Vector2i tilePos;   // gornji lijevi tile
    sf::Vector2i size;
    sf::Color    color;
    bool         solid;
    float        hp;
    float        maxHp;
    bool         destroyed{ false };
};

// ========================= BUILDING SYSTEM =========================

class BuildingSystem {
public:
    BuildingSystem();

    // Vrati sve definicije
    const std::vector<BuildingDef>& getDefs() const { return m_defs; }
    const BuildingDef& getDef(BuildingType t) const;

    // Pokusaj graditi na tile poziciji
    // Vraca true ako je uspjesno
    bool tryPlace(BuildingType type, sf::Vector2i tilePos,
        Inventory& inv, const TileMap& map);

    // Unisti zgradu na poziciji (vraca materijal)
    bool tryDestroy(sf::Vector2f worldPos, Inventory& inv);

    // Provjeri koliziju sa world pozicijom
    bool isSolid(sf::Vector2f worldPos) const;
    bool isSolid(int tileX, int tileY)  const;

    // Preview — valid ili ne
    bool canPlace(BuildingType type, sf::Vector2i tilePos,
        const Inventory& inv, const TileMap& map) const;

    void update(float dt);
    void collectRenderItems(std::vector<RenderItem>& out) const;
    void drawPreview(sf::RenderWindow& window, BuildingType type,
        sf::Vector2i tilePos, bool valid);

    const std::vector<PlacedBuilding>& getBuildings() const { return m_buildings; }

private:
    bool overlapsExisting(sf::Vector2i tilePos, sf::Vector2i size) const;
    void drawBuilding(sf::RenderWindow& window, const PlacedBuilding& b) const;

    std::vector<BuildingDef>     m_defs;
    std::vector<PlacedBuilding>  m_buildings;
};