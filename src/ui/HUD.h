#pragma once
#include "pch.h"
#include "systems/MarketSystem.h"

class HUD {
public:
    HUD();
    void draw(sf::RenderWindow& window, float hp, float maxHp,
        float gameHour, const Wallet& wallet);

private:
    sf::Font m_font;
};