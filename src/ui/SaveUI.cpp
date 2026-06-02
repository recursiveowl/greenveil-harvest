#include "ui/SaveUI.h"

static constexpr float CARD_W = 280.f;
static constexpr float CARD_H = 320.f;
static constexpr float CARD_PAD = 32.f;
static constexpr float BTN_W = 80.f;
static constexpr float BTN_H = 28.f;

SaveUI::SaveUI() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

sf::FloatRect SaveUI::cardRect(int i) const {
    float totalW = 3 * CARD_W + 2 * CARD_PAD;
    float startX = (m_ws.x - totalW) * 0.5f;
    float startY = (m_ws.y - CARD_H) * 0.5f;
    return { {startX + i * (CARD_W + CARD_PAD), startY}, {CARD_W, CARD_H} };
}

void SaveUI::draw(sf::RenderWindow& w, const SaveData slots[3]) {
    m_ws = w.getSize();

    sf::RectangleShape overlay({ (float)m_ws.x, (float)m_ws.y });
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    w.draw(overlay);

    sf::Text title(m_font, "SAVE GAME", 26);
    title.setFillColor(sf::Color(220, 185, 70));
    auto tb = title.getLocalBounds();
    title.setPosition({ (m_ws.x - tb.size.x) * 0.5f, cardRect(0).position.y - 60.f });
    w.draw(title);

    for (int i = 0; i < 3; ++i) {
        auto r = cardRect(i);
        bool empty = !slots[i].hasData;

        sf::RectangleShape card(r.size);
        card.setPosition(r.position);
        card.setFillColor(sf::Color(15, 18, 30, 245));
        card.setOutlineThickness(2.f);
        card.setOutlineColor(empty ? sf::Color(50, 55, 80) : sf::Color(100, 140, 220));
        w.draw(card);

        sf::Text slotName(m_font, "Slot " + std::to_string(i + 1), 18);
        slotName.setFillColor(sf::Color(180, 210, 255));
        slotName.setPosition({ r.position.x + 16.f, r.position.y + 14.f });
        w.draw(slotName);

        sf::RectangleShape sep({ CARD_W, 2.f });
        sep.setPosition({ r.position.x, r.position.y + 42.f });
        sep.setFillColor(sf::Color(50, 60, 100));
        w.draw(sep);

        if (empty) {
            sf::Text emptyT(m_font, "Empty", 15);
            emptyT.setFillColor(sf::Color(80, 80, 100));
            emptyT.setPosition({ r.position.x + 16.f, r.position.y + 60.f });
            w.draw(emptyT);
        }
        else {
            sf::Text lvl(m_font, "Level " + std::to_string(slots[i].playerLevel), 15);
            lvl.setFillColor(sf::Color(255, 220, 80));
            lvl.setPosition({ r.position.x + 16.f, r.position.y + 60.f });
            w.draw(lvl);

            int h = slots[i].playMinutes / 60;
            int m = slots[i].playMinutes % 60;
            std::string timeStr = std::to_string(h) + "h " + std::to_string(m) + "m";
            sf::Text playtime(m_font, "Playtime: " + timeStr, 13);
            playtime.setFillColor(sf::Color(160, 160, 190));
            playtime.setPosition({ r.position.x + 16.f, r.position.y + 86.f });
            w.draw(playtime);
        }

        // Dugmad
        float btnY = r.position.y + CARD_H - 48.f;
        float b0x = r.position.x + 10.f;
        float b1x = b0x + BTN_W + 8.f;
        float b2x = b1x + BTN_W + 8.f;

        auto drawBtn = [&](float bx, const std::string& lbl, bool enabled, sf::Color col) {
            sf::RectangleShape btn({ BTN_W, BTN_H });
            btn.setPosition({ bx, btnY });
            btn.setFillColor(enabled ? sf::Color(22, 28, 45) : sf::Color(14, 16, 24));
            btn.setOutlineThickness(1.5f);
            btn.setOutlineColor(enabled ? col : sf::Color(col.r / 3, col.g / 3, col.b / 3));
            w.draw(btn);
            sf::Text t(m_font, lbl, 12);
            t.setFillColor(enabled ? col : sf::Color(col.r / 3, col.g / 3, col.b / 3));
            auto bounds = t.getLocalBounds();
            t.setPosition({ bx + (BTN_W - bounds.size.x) * 0.5f, btnY + (BTN_H - bounds.size.y) * 0.5f - 2.f });
            w.draw(t);
            };

        drawBtn(b0x, "Save", true, sf::Color(80, 200, 80));
        drawBtn(b1x, "Load", !empty, sf::Color(80, 160, 255));
        drawBtn(b2x, "Delete", !empty, sf::Color(200, 80, 80));
    }

    sf::Text hint(m_font, "[ Esc ] Close", 12);
    hint.setFillColor(sf::Color(80, 80, 110));
    auto hb = hint.getLocalBounds();
    hint.setPosition({ (m_ws.x - hb.size.x) * 0.5f, cardRect(0).position.y + CARD_H + 16.f });
    w.draw(hint);
}

SaveAction SaveUI::handleClick(sf::Vector2f mouse, const SaveData slots[3]) {
    for (int i = 0; i < 3; ++i) {
        auto r = cardRect(i);
        float btnY = r.position.y + CARD_H - 48.f;
        float b0x = r.position.x + 10.f;
        float b1x = b0x + BTN_W + 8.f;
        float b2x = b1x + BTN_W + 8.f;

        if (sf::FloatRect({ b0x, btnY }, { BTN_W, BTN_H }).contains(mouse))
            return static_cast<SaveAction>(static_cast<int>(SaveAction::Save0) + i);
        if (!slots[i].hasData) continue;
        if (sf::FloatRect({ b1x, btnY }, { BTN_W, BTN_H }).contains(mouse))
            return static_cast<SaveAction>(static_cast<int>(SaveAction::Load0) + i);
        if (sf::FloatRect({ b2x, btnY }, { BTN_W, BTN_H }).contains(mouse))
            return static_cast<SaveAction>(static_cast<int>(SaveAction::Delete0) + i);
    }
    return SaveAction::None;
}