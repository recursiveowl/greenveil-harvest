#pragma once
#include "TileMap.h"
#include <unordered_set>

struct PlayerHouseData {
    sf::Vector2i tilePos{ 0, 0 };
    sf::Vector2i size{ 5, 4 };
    bool         placed{ false };
};

class PlayerHouseSystem {
public:
    // New game — nasumično postavi kuću u krugu oko igrača
    void spawnNear(sf::Vector2f playerPos, unsigned seed, const TileMap& map);

    // Load — direktno postavi poznatu poziciju iz save-a
    void setTilePos(sf::Vector2i tilePos);

    bool isSolid(sf::Vector2f worldPos) const;
    bool hasHouse() const { return m_house.placed; }
    sf::Vector2i getTilePos() const { return m_house.tilePos; }
    sf::Vector2i getSize() const { return m_house.size; }

    sf::Texture m_tex;       // house.png (exterior)
    sf::Texture m_texInt;    // house_int.png (interior)

    bool isNearDoor(sf::Vector2f playerPos) const;
    sf::Vector2f getFrontOfDoorPos() const;

    static constexpr float INTERIOR_SCALE = 0.6f;

    std::unordered_set<int> getOccupiedTileSet() const;

    sf::Vector2f getInteriorSize() const;
    sf::Vector2f getInteriorSpawnPos() const;
    bool isWithinInteriorBounds(sf::Vector2f localPos) const;

    void drawInterior(sf::RenderWindow& window);

    void draw(sf::RenderWindow& window);

private:
    PlayerHouseData m_house;
};