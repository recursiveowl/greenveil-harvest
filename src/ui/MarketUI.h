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

    void update(float dt, float gameHour, MarketSystem& market);

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
    void drawHeader(sf::RenderWindow& w, const Wallet& wallet,
        float gameHour, float px, float py);
    void drawItemList(sf::RenderWindow& w, const std::vector<MarketEntry>& entries,
        const Inventory& inv, float px, float py);
    void drawDetail(sf::RenderWindow& w, const MarketEntry& e,
        int playerCount, const Wallet& wallet,
        float px, float py);
    void drawSparkline(sf::RenderWindow& w,
        const std::vector<MarketSystem::PricePoint>& hist,
        float bx, float by, float bw, float bh);
    void drawActions(sf::RenderWindow& w, const MarketEntry* e,
        int playerCount, const Wallet& wallet,
        float px, float py);
    void drawStatusMsg(sf::RenderWindow& w, float px, float py);
    void drawScrollbar(sf::RenderWindow& w, float px, float py,
        int totalItems);

    sf::Font    m_font;
    sf::Texture m_goldTex;
    sf::Texture m_silverTex;
    sf::Vector2u m_windowSize{ 1280u, 720u };
    bool        m_open{ false };

    int   m_selectedIdx{ 0 };
    int   m_quantity{ 1 };
    int   m_scrollOffset{ 0 };   // prvi vidljivi item u listi

    // Cijene se osvjezavaju svake sekunde real time
    float m_priceFlashTimer{ 0.f };
    std::vector<float> m_prevPrices; // za flash efekt

    std::string m_statusMsg;
    float       m_statusTimer{ 0.f };
    bool        m_statusOk{ true };

    static constexpr float PANEL_W = 680.f;
    static constexpr float PANEL_H = 520.f;
    static constexpr float LIST_W = 260.f;
    static constexpr float ROW_H = 48.f;
    static constexpr float HEADER_H = 64.f;
    static constexpr float ACTION_H = 110.f;
    static constexpr int   VISIBLE_ROWS = 7;
};