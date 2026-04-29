#include "ui/HUD.h"
#include "systems/MarketSystem.h"
#include <cmath>

HUD::HUD() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

void HUD::draw(sf::RenderWindow& window, float hp, float maxHp,
    float gameHour, const Wallet& wallet) {
    sf::Vector2u ws = window.getSize();

    // HP bar
    float barW = 200.f;
    float barH = 18.f;
    float x = 16.f;
    float y = 16.f;
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
        std::to_string(static_cast<int>(maxHp)), 13);
    hpText.setFillColor(sf::Color::White);
    hpText.setPosition({ x + 4.f, y + 1.f });
    window.draw(hpText);

    // Sat
    int  hour = static_cast<int>(gameHour) % 24;
    int  minute = static_cast<int>((gameHour - std::floor(gameHour)) * 60.f);
    bool isAM = hour < 12;
    int  h12 = hour % 12;
    if (h12 == 0) h12 = 12;

    std::string timeStr =
        (h12 < 10 ? "0" : "") + std::to_string(h12) + ":" +
        (minute < 10 ? "0" : "") + std::to_string(minute) +
        (isAM ? " AM" : " PM");

    sf::Text timeText(m_font, timeStr, 14);
    timeText.setFillColor(sf::Color(220, 220, 180));
    timeText.setPosition({ x, y + barH + 6.f });
    window.draw(timeText);

    // Wallet
    std::string walletStr = MarketSystem::formatWallet(wallet);
    sf::Text walletText(m_font, walletStr, 13);
    walletText.setFillColor(sf::Color(240, 200, 60));
    walletText.setPosition({ x, y + barH + 26.f });
    window.draw(walletText);

    // Hint
    sf::Text mHint(m_font, "[ M ] Market", 11);
    mHint.setFillColor(sf::Color(120, 120, 140));
    mHint.setPosition({ x, y + barH + 44.f });
    window.draw(mHint);
}