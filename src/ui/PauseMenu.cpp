#include "ui/PauseMenu.h"

static const char* LABELS[] = {
    "Back To Game", "Settings", "Save / Load Game", "Quit"
};

PauseMenu::PauseMenu() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

sf::FloatRect PauseMenu::btnRect(int i, sf::Vector2u ws) const {
    float bw = 280.f, bh = 52.f, gap = 16.f;
    float totalH = 4 * bh + 3 * gap;
    float x = (ws.x - bw) * 0.5f;
    float y = (ws.y - totalH) * 0.5f + i * (bh + gap);
    return sf::FloatRect({ x, y }, { bw, bh });
}

PauseAction PauseMenu::handleClick(sf::Vector2f mouse, sf::Vector2u ws) {
    for (int i = 0; i < 4; ++i) {
        if (btnRect(i, ws).contains(mouse)) {
            if (i == 0) return PauseAction::Resume;
            if (i == 1) return PauseAction::Settings;
            if (i == 2) return PauseAction::SaveLoad;
            if (i == 3) return PauseAction::Quit;
        }
    }
    return PauseAction::None;
}

void PauseMenu::draw(sf::RenderWindow& window) {
    sf::Vector2u ws = window.getSize();

    // Overlay
    sf::RectangleShape overlay({ (float)ws.x, (float)ws.y });
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    // Naslov
    sf::Text title(m_font, "PAUSED", 28);
    title.setFillColor(sf::Color(220, 185, 70));
    auto tb = title.getLocalBounds();
    title.setPosition({ (ws.x - tb.size.x) * 0.5f,
        btnRect(0, ws).position.y - 60.f });
    window.draw(title);

    for (int i = 0; i < 4; ++i) {
        sf::FloatRect r = btnRect(i, ws);
        bool disabled = (i == 1); // Settings

        sf::RectangleShape bg(r.size);
        bg.setPosition(r.position);
        bg.setFillColor(sf::Color(15, 15, 25, 210));
        bg.setOutlineThickness(2.f);
        bg.setOutlineColor(disabled
            ? sf::Color(50, 50, 60)
            : sf::Color(120, 100, 40, 180));
        window.draw(bg);

        sf::Text t(m_font, LABELS[i], 20);
        t.setFillColor(disabled
            ? sf::Color(70, 70, 80)
            : sf::Color(220, 210, 180));
        auto lb = t.getLocalBounds();
        t.setPosition({
            r.position.x + (r.size.x - lb.size.x) * 0.5f,
            r.position.y + (r.size.y - lb.size.y) * 0.5f - 2.f
            });
        window.draw(t);
    }
}