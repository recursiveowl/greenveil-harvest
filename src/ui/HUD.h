#pragma once
#include "pch.h"
#include "systems/MarketSystem.h"

class HUD {
public:
    HUD();
    void draw(sf::RenderWindow& window, float hp, float maxHp,
        float gameHour, const Wallet& wallet, int currentLives);

    const sf::Font& getFont() const { return m_font; }

private:
    sf::Font    m_font;
    sf::Texture m_goldTex;
    sf::Texture m_silverTex;
    sf::Texture m_heartTex;
    sf::Texture m_healthBarTex;
};