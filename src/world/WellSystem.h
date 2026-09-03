#pragma once
#include "pch.h"

class WellSystem {
public:
    void place(sf::Vector2i marketPos, sf::Vector2i marketSize);

    sf::Vector2f getWorldPos() const;
    bool         isNear(sf::Vector2f playerPos, float range = 48.f) const;

    void draw(sf::RenderWindow& window) const;

private:
    sf::Vector2i m_tilePos{ 0, 0 };
};