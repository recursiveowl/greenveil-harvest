#pragma once
#include "pch.h";

enum class InteractTileType : uint8_t { Sign, Well, Chest, Notice, Ruins };

struct InteractTile {
    InteractTileType type;
    sf::Vector2i     tilePos;
    std::string      title;
    std::string      body;
    bool             discovered{ false };
    bool             oneShot{ false }; // nestaje nakon čitanja
};

class InteractTileManager {
public:
    static constexpr float RANGE = 52.f;

    void add(InteractTile tile);
    void update(sf::Vector2f playerPos); // setuje m_nearest
    bool tryInteract(sf::Vector2f playerPos, std::string& outTitle, std::string& outBody);
    bool hasNearby() const { return m_nearest != nullptr; }
    void draw(sf::RenderWindow& w); // crta [F] bubble iznad tile-a

private:
    std::vector<InteractTile> m_tiles;
    InteractTile* m_nearest{ nullptr };
};