#pragma once
#include "pch.h"
#include "systems/MarketSystem.h"
#include "systems/Inventory.h"

class MarketUI {
public:
    MarketUI();

    void open(float gameHour);
    void close();
    bool isOpen() const { return m_open; }

    void draw(sf::RenderWindow& window,
        MarketSystem& market,
        Inventory& inv,
        Wallet& wallet,
        float gameHour);

    bool handleClick(sf::Vector2f mouse,
        MarketSystem& market,
        Inventory& inv,
        Wallet& wallet);

    void handleScroll(float delta);

private:
    // Helpers — sve koriste SFML3 position/size API
    void drawHeader(sf::RenderWindow& w, const Wallet& wallet, float gameHour,
        float px, float py);
    void drawEntryRow(sf::RenderWindow& w, const MarketEntry& e,
        int idx, bool selected, int playerCount,
        float px, float py);
    void drawActions(sf::RenderWindow& w, const MarketEntry* e,
        int playerCount, const Wallet& wallet,
        float px, float py);
    void drawSparkline(sf::RenderWindow& w,
        const std::vector<MarketSystem::PricePoint>& hist,
        float bx, float by, float bw, float bh);
    void drawStatusMsg(sf::RenderWindow& w, float px, float py);

    sf::Font m_font;
    bool     m_open{ false };
    int      m_selectedIdx{ 0 };
    int      m_quantity{ 1 };

    std::string m_statusMsg;
    float       m_statusTimer{ 0.f };
    bool        m_statusOk{ true };

    static constexpr float PANEL_W = 620.f;
    static constexpr float PANEL_H = 480.f;
    static constexpr float ROW_H = 60.f;
    static constexpr float HEADER_H = 70.f;
    static constexpr float ACTION_H = 100.f;
};