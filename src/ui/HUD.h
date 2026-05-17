#pragma once
#include "pch.h"
#include "systems/MarketSystem.h"

class HUD {
public:
    HUD();
    void draw(sf::RenderWindow& window, float hp, float maxHp,
        float gameHour, const Wallet& wallet, int currentLives);

private:
    sf::Font    m_font;
    sf::Texture m_goldTex;
    sf::Texture m_silverTex;
    sf::Texture m_heartTex;
};