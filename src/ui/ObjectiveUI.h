#pragma once
#include "pch.h"
#include "systems/MissionSystem.h"

class ObjectiveUI {
public:
    ObjectiveUI();
    void draw(sf::RenderWindow& window,
              const MissionSystem& missions,
              bool turnInAvailable);

private:
    sf::Font m_font;
};
