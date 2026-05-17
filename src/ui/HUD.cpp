#include "ui/HUD.h"
#include "systems/MarketSystem.h"
#include "entities/Player.h"
#include <cmath>

HUD::HUD() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
    (void)m_goldTex.loadFromFile("assets/textures/gold_coins.png");
    (void)m_silverTex.loadFromFile("assets/textures/silver_coins.png");
    (void)m_heartTex.loadFromFile("assets/textures/heart.png");
    m_heartTex.setSmooth(false);
    m_goldTex.setSmooth(false);
    m_silverTex.setSmooth(false);
}

void HUD::draw(sf::RenderWindow& window, float hp, float maxHp,
    float gameHour, const Wallet& wallet, int currentLives) {
    sf::Vector2u ws = window.getSize();

    float barW = UI::X(200.f);
    float barH = UI::Y(18.f);
    float x = UI::X(16.f);
    float y = UI::Y(16.f);
    float ratio = std::max(0.f, hp / maxHp);

    sf::RectangleShape bg({ barW, barH });
    bg.setPosition({ x, y });
    bg.setFillColor(sf::Color(60, 0, 0, 200));
    bg.setOutlineThickness(2.f);
    bg.setOutlineColor(sf::Color(0, 0, 0, 180));
    window.draw(bg);

    sf::Color hpCol = ratio > 0.5f
        ? sf::Color(50, 200, 50)
        : ratio > 0.25f ? sf::Color(220, 180, 0) : sf::Color(220, 40, 40);

    sf::RectangleShape fill({ barW * ratio, barH });
    fill.setPosition({ x, y });
    fill.setFillColor(hpCol);
    window.draw(fill);

    sf::Text hpText(m_font,
        "HP: " + std::to_string(static_cast<int>(hp)) + " / " +
        std::to_string(static_cast<int>(maxHp)), UI::S(13.f));
    hpText.setFillColor(sf::Color::White);
    hpText.setPosition({ x + UI::X(4.f), y + UI::Y(1.f) });
    window.draw(hpText);

    int  hour = static_cast<int>(gameHour) % 24;
    int  minute = static_cast<int>((gameHour - std::floor(gameHour)) * 60.f);
    bool isAM = hour < 12;
    int  h12 = hour % 12;
    if (h12 == 0) h12 = 12;

    std::string timeStr =
        (h12 < 10 ? "0" : "") + std::to_string(h12) + ":" +
        (minute < 10 ? "0" : "") + std::to_string(minute) +
        (isAM ? " AM" : " PM");

    sf::Text timeText(m_font, timeStr, UI::S(14.f));
    timeText.setFillColor(sf::Color(220, 220, 180));
    timeText.setPosition({ x, y + barH + UI::Y(6.f) });
    window.draw(timeText);

    float wy = y + barH + UI::Y(24.f);

    sf::Sprite goldIcon(m_goldTex);
    goldIcon.setScale({ UI::S(1.5f), UI::S(1.5f) });
    goldIcon.setPosition({ x, wy });
    window.draw(goldIcon);

    sf::Text goldText(m_font, std::to_string(wallet.gold) + "g", UI::S(13.f));
    goldText.setFillColor(sf::Color(240, 200, 60));
    goldText.setPosition({ x + UI::X(26.f), wy + UI::Y(1.f) });
    window.draw(goldText);

    sf::Sprite silvIcon(m_silverTex);
    silvIcon.setScale({ UI::S(1.5f), UI::S(1.5f) });
    silvIcon.setPosition({ x + UI::X(62.f), wy });
    window.draw(silvIcon);

    sf::Text silvText(m_font, std::to_string(wallet.silver) + "s", UI::S(13.f));
    silvText.setFillColor(sf::Color(200, 200, 220));
    silvText.setPosition({ x + UI::X(88.f), wy + UI::Y(1.f) });
    window.draw(silvText);

    if (wallet.emerald > 0) {
        sf::Text emText(m_font, std::to_string(wallet.emerald) + "em", UI::S(12.f));
        emText.setFillColor(sf::Color(60, 220, 160));
        emText.setPosition({ x + UI::X(130.f), wy + UI::Y(1.f) });
        window.draw(emText);
    }

    sf::Text mHint(m_font, "[ M ] Market", UI::S(11.f));
    mHint.setFillColor(sf::Color(120, 120, 140));
    mHint.setPosition({ x, y + barH + UI::Y(44.f) });
    window.draw(mHint);

    sf::Text eqHint(m_font, "Sword=LClick  Rod=F near water", UI::S(10.f));
    eqHint.setFillColor(sf::Color(90, 90, 110));
    eqHint.setPosition({ x, y + barH + UI::Y(58.f) });
    window.draw(eqHint);

    // Srca — puna slijeva, prazna zdesna
    float hx = ws.x - Player::MAX_LIVES * 49.f - 16.f;
    float hy = 16.f;
    for (int i = 0; i < Player::MAX_LIVES; ++i) {
        sf::Sprite heart(m_heartTex);
        bool full = (i < currentLives);
        heart.setTextureRect(sf::IntRect({ 0, full ? 0 : 16 }, { 15, 16 }));
        heart.setScale({ 3.f, 3.f });
        heart.setPosition({ hx + i * 49.f, hy });
        window.draw(heart);
    }
}