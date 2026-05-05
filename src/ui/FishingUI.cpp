#include "ui/FishingUI.h"
#include <cmath>
#include <algorithm>

static constexpr float PI = 3.14159265f;

FishingUI::FishingUI() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

void FishingUI::draw(sf::RenderWindow& window, const FishingSystem& fs,
    bool canFish) {
    m_blinkTimer += 0.016f;
    FishingState state = fs.getState();
    if (state == FishingState::Reeling)
        drawMinigame(window, fs.getMinigame());
    drawStateMsg(window, fs);
}

void FishingUI::drawBobber(sf::RenderWindow& w,
    const BobberAnim& bobber,
    sf::Vector2f castOrigin,
    FishingState state) {
    if (state == FishingState::Idle) return;

    sf::Vector2f bpos = bobber.pos;

    // Struna
    sf::VertexArray line(sf::PrimitiveType::Lines, 2);
    line[0] = { castOrigin + sf::Vector2f{0.f, -20.f},
               sf::Color(200, 170, 100, 150) };
    line[1] = { bpos, sf::Color(200, 170, 100, 150) };
    w.draw(line);

    // Splash rings
    if (bobber.splashTimer > 0.f) {
        float t = 1.f - bobber.splashTimer / 0.4f;
        uint8_t a = static_cast<uint8_t>((1.f - t) * 180.f);
        for (int ring = 0; ring < 2; ++ring) {
            float rad = t * (ring == 0 ? 22.f : 12.f);
            sf::CircleShape s(rad);
            s.setOrigin({ rad, rad });
            s.setPosition(bpos + sf::Vector2f{ 0.f, (float)ring * 2.f });
            s.setFillColor(sf::Color::Transparent);
            s.setOutlineThickness(1.5f);
            s.setOutlineColor(sf::Color(120, 190, 255, a));
            w.draw(s);
        }
    }

    // Tijelo plovka
    float r = 5.5f;
    sf::CircleShape body(r);
    body.setOrigin({ r, r });
    body.setPosition(bpos);

    if (state == FishingState::Biting) {
        float t = std::sin(m_blinkTimer * 12.f) * 0.5f + 0.5f;
        body.setFillColor(sf::Color(
            static_cast<uint8_t>(180 + 75 * t), 40, 40));
    }
    else if (state == FishingState::Reeling) {
        body.setFillColor(sf::Color(60, 200, 80));
    }
    else {
        body.setFillColor(sf::Color(200, 55, 55));
    }
    body.setOutlineThickness(1.5f);
    body.setOutlineColor(sf::Color(0, 0, 0, 150));
    w.draw(body);

    // Bijeli vrh
    sf::CircleShape tip(2.5f);
    tip.setOrigin({ 2.5f, 2.5f });
    tip.setPosition(bpos + sf::Vector2f{ 0.f, -r + 1.f });
    tip.setFillColor(sf::Color(255, 255, 255, 210));
    w.draw(tip);

    // Vodena linija
    if (bobber.inWater && state != FishingState::Casting) {
        sf::RectangleShape wl({ 14.f, 2.f });
        wl.setOrigin({ 7.f, 1.f });
        wl.setPosition(bpos + sf::Vector2f{ 0.f, 3.f });
        wl.setFillColor(sf::Color(100, 160, 220, 100));
        w.draw(wl);
    }
}

void FishingUI::drawMinigame(sf::RenderWindow& w,
    const FishingMinigame& mg) {
    sf::Vector2u ws = w.getSize();
    float panW = 340.f;
    float panH = 110.f;
    float px = (ws.x - panW) * 0.5f;
    float py = ws.y * 0.30f;

    sf::RectangleShape panel({ panW, panH });
    panel.setPosition({ px, py });
    panel.setFillColor(sf::Color(10, 13, 24, 248));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(mg.fishPulling
        ? sf::Color(180, 50, 50, 220)
        : sf::Color(50, 110, 180, 220));
    w.draw(panel);

    std::string instr = mg.fishPulling
        ? "!! PULLING !!  Spam [F]"
        : "Press [F] to reel in";
    sf::Color instrCol = mg.fishPulling
        ? sf::Color(255, 90, 90)
        : sf::Color(100, 200, 255);

    sf::Text instrT(m_font, instr, 14);
    instrT.setFillColor(instrCol);
    auto ib = instrT.getLocalBounds();
    instrT.setPosition({ px + (panW - ib.size.x) * 0.5f, py + 8.f });
    w.draw(instrT);

    float barW = panW - 24.f;
    float barH = 18.f;
    float barX = px + 12.f;

    // CATCH bar (zelena)
    float catchY = py + 34.f;
    sf::Text catchLbl(m_font, "CATCH", 11);
    catchLbl.setFillColor(sf::Color(80, 200, 100));
    catchLbl.setPosition({ barX, catchY - 1.f });
    w.draw(catchLbl);

    sf::RectangleShape catchBg({ barW, barH });
    catchBg.setPosition({ barX, catchY + 14.f });
    catchBg.setFillColor(sf::Color(15, 35, 15));
    catchBg.setOutlineThickness(1.f);
    catchBg.setOutlineColor(sf::Color(30, 70, 30));
    w.draw(catchBg);

    float cw = barW * std::clamp(mg.catchProgress, 0.f, 1.f);
    sf::RectangleShape catchFill({ cw, barH });
    catchFill.setPosition({ barX, catchY + 14.f });
    catchFill.setFillColor(sf::Color(50, 190, 70));
    w.draw(catchFill);

    if (cw > 4.f) {
        sf::RectangleShape tip({ 4.f, barH });
        tip.setPosition({ barX + cw - 4.f, catchY + 14.f });
        tip.setFillColor(sf::Color(160, 255, 160, 180));
        w.draw(tip);
    }

    sf::Text catchPct(m_font,
        std::to_string(static_cast<int>(mg.catchProgress * 100)) + "%", 10);
    catchPct.setFillColor(sf::Color(120, 220, 140));
    catchPct.setPosition({ barX + barW - 30.f, catchY - 1.f });
    w.draw(catchPct);

    // ESCAPE bar (crvena)
    float escY = py + 72.f;
    sf::Text escLbl(m_font, "ESCAPE", 11);
    escLbl.setFillColor(sf::Color(200, 80, 80));
    escLbl.setPosition({ barX, escY - 1.f });
    w.draw(escLbl);

    sf::RectangleShape escBg({ barW, barH });
    escBg.setPosition({ barX, escY + 14.f });
    escBg.setFillColor(sf::Color(35, 10, 10));
    escBg.setOutlineThickness(1.f);
    escBg.setOutlineColor(sf::Color(70, 20, 20));
    w.draw(escBg);

    float ew = barW * std::clamp(mg.escapeProgress, 0.f, 1.f);
    sf::RectangleShape escFill({ ew, barH });
    escFill.setPosition({ barX, escY + 14.f });
    escFill.setFillColor(sf::Color(190, 50, 50));
    w.draw(escFill);

    if (ew > 4.f) {
        sf::RectangleShape tip({ 4.f, barH });
        tip.setPosition({ barX + ew - 4.f, escY + 14.f });
        tip.setFillColor(sf::Color(255, 140, 140, 180));
        w.draw(tip);
    }

    sf::Text escPct(m_font,
        std::to_string(static_cast<int>(mg.escapeProgress * 100)) + "%", 10);
    escPct.setFillColor(sf::Color(220, 110, 110));
    escPct.setPosition({ barX + barW - 30.f, escY - 1.f });
    w.draw(escPct);

    // Pull flash overlay
    if (mg.fishPulling) {
        float a = (std::sin(m_blinkTimer * 7.f) * 0.5f + 0.5f) * 30.f;
        sf::RectangleShape flash({ panW, panH });
        flash.setPosition({ px, py });
        flash.setFillColor(sf::Color(200, 0, 0, static_cast<uint8_t>(a)));
        w.draw(flash);
    }
}

void FishingUI::drawStateMsg(sf::RenderWindow& w, const FishingSystem& fs) {
    sf::Vector2u ws = w.getSize();
    FishingState state = fs.getState();

    std::string msg;
    sf::Color   col;

    switch (state) {
    case FishingState::Casting:
        msg = "Casting...";
        col = sf::Color(160, 190, 255);
        break;
    case FishingState::Splashing:
        msg = "* splash *";
        col = sf::Color(120, 190, 255);
        break;

    case FishingState::Biting: {
        float remain = fs.getBiteWindow() - fs.getBiteTimer();
        float t = std::sin(m_blinkTimer * 10.f) * 0.5f + 0.5f;
        msg = "!! BITE !! Auto-reeling...";
        col = sf::Color(static_cast<uint8_t>(200 + 55 * t), 60, 60);
        break;
    }
    case FishingState::Caught:
        msg = "Fish caught!";
        col = sf::Color(80, 230, 80);
        break;
    case FishingState::Failed:
        msg = "It got away...";
        col = sf::Color(180, 100, 80);
        break;
    case FishingState::Waiting:
        if (fs.isPullMode()) {
            msg = "Not in water!  Press [F] to pull back";
            col = sf::Color(220, 160, 60);
        }
        else {
            int dots = static_cast<int>(fs.getWaitTimer() * 2.f) % 4;
            msg = "Waiting";
            for (int i = 0; i < dots; ++i) msg += ".";
            col = sf::Color(130, 150, 200);
        }
        break;
    default: return;
    }

    sf::Text t(m_font, msg, 15);
    t.setFillColor(col);
    auto tb = t.getLocalBounds();
    float tx = (ws.x - tb.size.x) * 0.5f;
    float ty = ws.y * 0.56f;

    sf::RectangleShape bd({ tb.size.x + 18.f, tb.size.y + 10.f });
    bd.setPosition({ tx - 9.f, ty - 4.f });
    bd.setFillColor(sf::Color(8, 10, 20, 215));
    bd.setOutlineThickness(1.f);
    bd.setOutlineColor(sf::Color(50, 70, 120, 160));
    w.draw(bd);
    w.draw(t);
}