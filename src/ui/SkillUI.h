#pragma once
#include "pch.h"
#include "systems/ProgressionSystem.h"

class SkillUI {
public:
    SkillUI();
    void toggle() { m_visible = !m_visible; }
    bool isVisible() const { return m_visible; }
    void draw(sf::RenderWindow& window, const ProgressionSystem& prog);

    // Vrati koji skill je kliknut (-1 = nista)
    int  handleClick(sf::Vector2f mousePos, const ProgressionSystem& prog);

private:
    sf::Font m_font;
    bool     m_visible{false};
};
