#pragma once
#include "pch.h"
#include "systems/BuildingSystem.h"
#include "systems/Inventory.h"

class BuildingUI {
public:
    BuildingUI();

    void toggle();
    void close();
    bool isOpen() const { return m_open; }

    // Vrati selektovani tip (-1 = nista)
    BuildingType getSelected() const { return m_selected; }
    bool         hasSelected() const { return m_hasSelected; }
    void         clearSelected() { m_hasSelected = false; }

    bool handleClick(sf::Vector2f mouse,
        const BuildingSystem& bs,
        const Inventory& inv);

    void draw(sf::RenderWindow& window,
        const BuildingSystem& bs,
        const Inventory& inv);

private:
    void drawCard(sf::RenderWindow& w, const BuildingDef& def,
        int idx, bool selected, bool affordable,
        float px, float py);
    void drawPhysicsInfo(sf::RenderWindow& w, const BuildingDef& def,
        float px, float py);

    sf::Font     m_font;
    sf::Vector2u m_windowSize{ 1280u, 720u };
    bool         m_open{ false };
    BuildingType m_selected{ BuildingType::WoodWall };
    bool         m_hasSelected{ false };
    int          m_hoveredIdx{ -1 };

    static constexpr float PANEL_W = 700.f;
    static constexpr float PANEL_H = 440.f;
    static constexpr float CARD_W = 140.f;
    static constexpr float CARD_H = 100.f;
    static constexpr float CARD_PAD = 12.f;
};