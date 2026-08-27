#include "ui/HUD.h"
#include "systems/MarketSystem.h"
#include "entities/Player.h"
#include <cmath>

HUD::HUD() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
    (void)m_goldTex.loadFromFile("assets/textures/gold_coins.png");
    (void)m_silverTex.loadFromFile("assets/textures/silver_coins.png");
    (void)m_heartTex.loadFromFile("assets/textures/heart.png");
    (void)m_healthBarTex.loadFromFile("assets/textures/healthbar.png");
    m_healthBarTex.setSmooth(false);
    m_heartTex.setSmooth(false);
    m_goldTex.setSmooth(false);
    m_silverTex.setSmooth(false);
}

void HUD::draw(sf::RenderWindow& window, float hp, float maxHp,
    float gameHour, const Wallet& wallet, int currentLives) {

    sf::Vector2u ws = window.getSize();
    float sx = ws.x / 1280.f;
    float sy = ws.y / 720.f;
    float s = std::min(sx, sy);

    auto X = [&](float v) { return v * sx; };
    auto Y = [&](float v) { return v * sy; };
    auto S = [&](float v) { return static_cast<unsigned int>(v * s); };

    float barW = X(260.f);
    float barH = Y(22.f);
    float x = X(16.f);
    float y = Y(16.f);
    float ratio = std::max(0.f, hp / maxHp);

    sf::Sprite hpBar(m_healthBarTex);
    float fullWidth = static_cast<float>(m_healthBarTex.getSize().x);
    float fullHeight = static_cast<float>(m_healthBarTex.getSize().y);
    float targetW = 275.f;
    float targetH = 58.f;
    hpBar.setScale({ targetW / fullWidth, targetH / fullHeight });
    hpBar.setPosition({ x, y });
    window.draw(hpBar);

    sf::RectangleShape fill({ (targetW - 44.f) * ratio, targetH - 22.f });
    fill.setPosition({ x + 19.f, y + 11.f });
    fill.setFillColor(sf::Color(40, 160, 40));
    window.draw(fill);

    int  hour = static_cast<int>(gameHour) % 24;
    int  minute = static_cast<int>((gameHour - std::floor(gameHour)) * 60.f);
    bool isAM = hour < 12;
    int  h12 = hour % 12;
    if (h12 == 0) h12 = 12;

    std::string timeStr =
        (h12 < 10 ? "0" : "") + std::to_string(h12) + ":" +
        (minute < 10 ? "0" : "") + std::to_string(minute) +
        (isAM ? " AM" : " PM");

    sf::Text timeText(m_font, timeStr, S(14.f));
    timeText.setFillColor(sf::Color(220, 220, 180));
    timeText.setPosition({ x, y + barH + Y(35.f) });
    window.draw(timeText);

    float wy = y + barH + Y(70.f);

    sf::Sprite goldIcon(m_goldTex);
    goldIcon.setScale({ s * 1.5f, s * 1.5f });
    goldIcon.setPosition({ x, wy });
    window.draw(goldIcon);

    sf::Text goldText(m_font, std::to_string(wallet.gold) + "g", S(13.f));
    goldText.setFillColor(sf::Color(240, 200, 60));
    goldText.setPosition({ x + X(26.f), wy + Y(1.f) });
    window.draw(goldText);

    sf::Sprite silvIcon(m_silverTex);
    silvIcon.setScale({ s * 1.5f, s * 1.5f });
    silvIcon.setPosition({ x + X(62.f), wy });
    window.draw(silvIcon);

    sf::Text silvText(m_font, std::to_string(wallet.silver) + "s", S(13.f));
    silvText.setFillColor(sf::Color(200, 200, 220));
    silvText.setPosition({ x + X(88.f), wy + Y(1.f) });
    window.draw(silvText);

    if (wallet.emerald > 0) {
        sf::Text emText(m_font, std::to_string(wallet.emerald) + "em", S(12.f));
        emText.setFillColor(sf::Color(60, 220, 160));
        emText.setPosition({ x + X(130.f), wy + Y(1.f) });
        window.draw(emText);
    }
}