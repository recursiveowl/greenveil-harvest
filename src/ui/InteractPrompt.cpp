#include "ui/InteractPrompt.h"

InteractPrompt::InteractPrompt() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

void InteractPrompt::show(const std::string& title, const std::string& body) {
    m_title = title; m_body = body; m_visible = true;
}
void InteractPrompt::hide() { m_visible = false; }

void InteractPrompt::draw(sf::RenderWindow& w) {
    if (!m_visible) return;
    sf::Vector2u ws = w.getSize();
    float bw = ws.x * 0.60f, bh = 140.f;
    float bx = (ws.x - bw) * 0.5f, by = ws.y - bh - 24.f;

    sf::RectangleShape bg({ bw, bh });
    bg.setPosition({ bx, by });
    bg.setFillColor(sf::Color(15, 15, 25, 235));
    bg.setOutlineThickness(2.f);
    bg.setOutlineColor(sf::Color(255, 220, 60, 220));
    w.draw(bg);

    sf::RectangleShape titleBar({ 220.f, 34.f });
    titleBar.setPosition({ bx + 12.f, by - 34.f });
    titleBar.setFillColor(sf::Color(15, 15, 25, 235));
    titleBar.setOutlineThickness(2.f);
    titleBar.setOutlineColor(sf::Color(255, 220, 60, 220));
    w.draw(titleBar);

    sf::Text titleT(m_font, m_title, 20);
    titleT.setFillColor(sf::Color(255, 220, 60));
    titleT.setPosition({ bx + 18.f, by - 32.f });
    w.draw(titleT);

    sf::Text bodyT(m_font, m_body, 18);
    bodyT.setFillColor(sf::Color(235, 235, 235));
    bodyT.setPosition({ bx + 22.f, by + 14.f });
    w.draw(bodyT);

    sf::Text hint(m_font, "[ F / Esc ] Close", 12);
    hint.setFillColor(sf::Color(140, 140, 140));
    hint.setPosition({ bx + bw - 160.f, by + bh - 20.f });
    w.draw(hint);
}