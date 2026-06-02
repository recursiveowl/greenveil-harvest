#pragma once
#include "pch.h"
#include "core/SaveSystem.h"

enum class SaveAction {
    None,
    Save0, Save1, Save2,
    Load0, Load1, Load2,
    Delete0, Delete1, Delete2,
    Close
};

class SaveUI {
public:
    SaveUI();
    void draw(sf::RenderWindow& w, const SaveData slots[3]);
    SaveAction handleClick(sf::Vector2f mouse, const SaveData slots[3]);
private:
    sf::Font m_font;
    sf::FloatRect cardRect(int i) const;
    sf::Vector2u m_ws{ 1280u, 720u };
};