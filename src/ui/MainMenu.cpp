#include "ui/MainMenu.h"

static const char* BTN_LABELS[] = { "New Game", "Continue", "Settings", "Exit" };

MainMenu::MainMenu() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
    (void)m_bg.loadFromFile("assets/textures/main_menu.png");
}

MenuAction MainMenu::handleEvent(const sf::Event& event, sf::Vector2u ws) {
    if (const auto* m = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mp((float)m->position.x, (float)m->position.y);
        m_hovered = -1;
        for (int i = 0; i < 4; ++i)
            if (buildBtn(i, ws).contains(mp)) { m_hovered = i; break; }
    }
    if (const auto* c = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (c->button == sf::Mouse::Button::Left) {
            sf::Vector2f mp((float)c->position.x, (float)c->position.y);
            for (int i = 0; i < 4; ++i) {
                if (buildBtn(i, ws).contains(mp)) {
                    if (i == 0) return MenuAction::NewGame;
                    if (i == 1) return MenuAction::Continue;
                    if (i == 3) return MenuAction::Exit;
                    return MenuAction::None; // Settings
                }
            }
        }
    }
    return MenuAction::None;
}

sf::FloatRect MainMenu::buildBtn(int i, sf::Vector2u ws) {
    float bw = 260.f, bh = 52.f, gap = 18.f;
    float totalH = 4 * bh + 3 * gap;
    float startY = ws.y * 0.52f - totalH * 0.5f;
    float x = (ws.x - bw) * 0.5f;
    float y = startY + i * (bh + gap);
    return sf::FloatRect({ x, y }, { bw, bh });
}

void MainMenu::draw(sf::RenderWindow& window) {
    sf::Vector2u ws = window.getSize();

    // Background
    sf::Sprite bg(m_bg);
    float sx = (float)ws.x / m_bg.getSize().x;
    float sy = (float)ws.y / m_bg.getSize().y;
    float s = std::max(sx, sy);
    bg.setScale({ s, s });
    float offX = (ws.x - m_bg.getSize().x * s) * 0.5f;
    float offY = (ws.y - m_bg.getSize().y * s) * 0.5f;
    bg.setPosition({ offX, offY });
    window.draw(bg);

    // Dugmad
    for (int i = 0; i < 4; ++i)
        drawButton(window, i, ws);
}

void MainMenu::drawButton(sf::RenderWindow& w, int i, sf::Vector2u ws) {
    sf::FloatRect r = buildBtn(i, ws);
    bool hov = (m_hovered == i);
    bool disabled = (i == 2); // Settings

    sf::RectangleShape bg(r.size);
    bg.setPosition(r.position);
    bg.setFillColor(hov && !disabled
        ? sf::Color(60, 50, 20, 230)
        : sf::Color(15, 15, 25, 210));
    bg.setOutlineThickness(2.f);
    bg.setOutlineColor(disabled
        ? sf::Color(50, 50, 60)
        : hov ? sf::Color(255, 210, 60) : sf::Color(120, 100, 40, 180));
    w.draw(bg);

    sf::Text t(m_font, BTN_LABELS[i], 20);
    t.setFillColor(disabled
        ? sf::Color(70, 70, 80)
        : hov ? sf::Color(255, 220, 80) : sf::Color(220, 210, 180));
    auto tb = t.getLocalBounds();
    t.setPosition({
        r.position.x + (r.size.x - tb.size.x) * 0.5f,
        r.position.y + (r.size.y - tb.size.y) * 0.5f - 2.f
        });
    w.draw(t);
}