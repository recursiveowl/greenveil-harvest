#pragma once
#include "pch.h"

class HUD {
public:
    HUD();
    void draw(sf::RenderWindow& window, float hp, float maxHp, float gameHour);

private:
    sf::Font m_font;
};
