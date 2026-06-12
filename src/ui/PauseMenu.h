#pragma once
#include "pch.h"

enum class PauseAction { None, Resume, Settings, SaveLoad, Quit };

class PauseMenu {
public:
    PauseMenu();
    PauseAction handleClick(sf::Vector2f mouse, sf::Vector2u ws);
    void draw(sf::RenderWindow& window);
private:
    sf::FloatRect btnRect(int i, sf::Vector2u ws) const;
    sf::Font m_font;
};