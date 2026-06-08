#pragma once
#include "pch.h"

enum class MenuAction { None, NewGame, Continue, Exit };

class MainMenu {
public:
    MainMenu();
    MenuAction    handleEvent(const sf::Event& event, sf::Vector2u ws);
    void          draw(sf::RenderWindow& window);
private:
    sf::FloatRect buildBtn(int i, sf::Vector2u ws);
    void          drawButton(sf::RenderWindow& w, int i, sf::Vector2u ws);
    sf::Font      m_font;
    sf::Texture   m_bg;
    int           m_hovered{ -1 };
};