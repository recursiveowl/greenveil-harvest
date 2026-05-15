#include "ui/MarketUI.h"
#include <cmath>
#include <algorithm>

MarketUI::MarketUI() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
    (void)m_goldTex.loadFromFile("assets/textures/gold_coins.png");
    (void)m_silverTex.loadFromFile("assets/textures/silver_coins.png");
    m_goldTex.setSmooth(false);
    m_silverTex.setSmooth(false);
}

void MarketUI::open(float gameHour) {
    m_open = true;
    m_selectedIdx = 0;
    m_quantity = 1;
    m_scrollOffset = 0;
    m_statusMsg = "";
    m_statusTimer = 0.f;
    m_priceFlashTimer = 0.f;
}

void MarketUI::close() { m_open = false; }

// Svake sekunde real-time osvjezi cijene
void MarketUI::update(float dt, float gameHour, MarketSystem& market) {
    if (!m_open) return;
    m_priceFlashTimer += dt;
    if (m_priceFlashTimer >= 1.0f) {
        m_priceFlashTimer = 0.f;
        // Spremi stare cijene za flash
        m_prevPrices.clear();
        for (auto& e : market.getEntries())
            m_prevPrices.push_back(static_cast<float>(e.currentBuyPrice));
        // Force recalc
        market.forceRecalc(gameHour);
    }
    if (m_statusTimer > 0.f) m_statusTimer -= dt;
}

static void btn(sf::RenderWindow& w, sf::Font& f,
    float x, float y, float bw, float bh,
    const std::string& lbl, sf::Color col, bool enabled) {
    sf::RectangleShape bg({ bw, bh });
    bg.setPosition({ x, y });
    bg.setFillColor(enabled ? sf::Color(28, 34, 50) : sf::Color(18, 20, 28));
    bg.setOutlineThickness(1.5f);
    bg.setOutlineColor(enabled ? col : sf::Color(col.r / 3, col.g / 3, col.b / 3));
    w.draw(bg);
    sf::Text t(f, lbl, 13);
    t.setFillColor(enabled ? col : sf::Color(col.r / 3, col.g / 3, col.b / 3));
    auto tb = t.getLocalBounds();
    t.setPosition({ x + (bw - tb.size.x) * 0.5f, y + (bh - tb.size.y) * 0.5f - 2.f });
    w.draw(t);
}

// -------------------------------------------------------
//  Main draw
// -------------------------------------------------------
void MarketUI::draw(sf::RenderWindow& window,
    MarketSystem& market,
    Inventory& inv,
    Wallet& wallet,
    float gameHour) {
    m_windowSize = window.getSize();
    if (!m_open) return;

    sf::Vector2u ws = window.getSize();
    float px = (ws.x - PANEL_W) * 0.5f;
    float py = (ws.y - PANEL_H) * 0.5f;

    // Panel bg
    sf::RectangleShape panel({ PANEL_W, PANEL_H });
    panel.setPosition({ px, py });
    panel.setFillColor(sf::Color(10, 12, 22, 252));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(160, 130, 50, 220));
    window.draw(panel);

    // Vertikalni separator lista | detalji
    sf::RectangleShape sep({ 2.f, PANEL_H - HEADER_H - ACTION_H });
    sep.setPosition({ px + LIST_W + 16.f, py + HEADER_H });
    sep.setFillColor(sf::Color(70, 60, 30, 160));
    window.draw(sep);

    drawHeader(window, wallet, gameHour, px, py);

    const auto& entries = market.getEntries();
    drawItemList(window, entries, inv, px, py);
    drawScrollbar(window, px, py, static_cast<int>(entries.size()));

    // Detalji selektovanog
    if (m_selectedIdx < static_cast<int>(entries.size())) {
        const MarketEntry& sel = entries[m_selectedIdx];
        int playerCount = inv.countOf(sel.item);
        drawDetail(window, sel, playerCount, wallet, px, py);
        drawActions(window, &sel, playerCount, wallet, px, py);
    }

    drawStatusMsg(window, px, py);

    sf::Text hint(m_font, "[ M / Esc ] Close  |  Scroll = navigate list", 10);
    hint.setFillColor(sf::Color(70, 70, 90));
    hint.setPosition({ px + 10.f, py + PANEL_H - 16.f });
    window.draw(hint);
}

// -------------------------------------------------------
//  Header
// -------------------------------------------------------
void MarketUI::drawHeader(sf::RenderWindow& w, const Wallet& wallet,
    float gameHour, float px, float py) {
    // Naslov
    sf::Text title(m_font, "GREENVEIL MARKET", 20);
    title.setFillColor(sf::Color(220, 185, 70));
    title.setPosition({ px + 12.f, py + 10.f });
    w.draw(title);

    // Market activity
    float mult = MarketSystem::getDayPriceMult(gameHour);
    std::string actStr;
    sf::Color   actCol;
    if (mult > 1.2f) { actStr = "Activity: HIGH";   actCol = sf::Color(80, 220, 80); }
    else if (mult > 1.0f) { actStr = "Activity: NORMAL"; actCol = sf::Color(220, 200, 60); }
    else { actStr = "Activity: LOW";    actCol = sf::Color(220, 100, 80); }

    // Activity dot + text
    sf::CircleShape dot(5.f);
    dot.setOrigin({ 5.f, 5.f });
    dot.setFillColor(actCol);
    dot.setPosition({ px + 12.f, py + 38.f });
    w.draw(dot);
    sf::Text actT(m_font, actStr, 12);
    actT.setFillColor(actCol);
    actT.setPosition({ px + 24.f, py + 30.f });
    w.draw(actT);

    // Separator
    sf::RectangleShape line({ PANEL_W, 2.f });
    line.setPosition({ px, py + HEADER_H - 2.f });
    line.setFillColor(sf::Color(80, 65, 25, 180));
    w.draw(line);

    // Wallet — desno u headeru sa sprite ikonama
    float wy = py + 10.f;
    float wx = px + PANEL_W - 14.f;

    // Silver
    sf::Text st(m_font, std::to_string(wallet.silver) + "s", 13);
    st.setFillColor(sf::Color(200, 200, 220));
    auto sb = st.getLocalBounds();
    wx -= sb.size.x;
    st.setPosition({ wx, wy });
    w.draw(st);
    wx -= 20.f;
    sf::Sprite sIcon(m_silverTex);
    sIcon.setScale({ 1.25f, 1.25f });
    sIcon.setPosition({ wx, wy - 1.f });
    w.draw(sIcon);
    wx -= 8.f;

    // Gold
    sf::Text gt(m_font, std::to_string(wallet.gold) + "g", 13);
    gt.setFillColor(sf::Color(240, 195, 55));
    auto gb = gt.getLocalBounds();
    wx -= gb.size.x;
    gt.setPosition({ wx, wy });
    w.draw(gt);
    wx -= 20.f;
    sf::Sprite gIcon(m_goldTex);
    gIcon.setScale({ 1.25f, 1.25f });
    gIcon.setPosition({ wx, wy - 1.f });
    w.draw(gIcon);
    wx -= 8.f;

    // Emerald
    if (wallet.emerald > 0) {
        sf::Text et(m_font, std::to_string(wallet.emerald) + "em", 12);
        et.setFillColor(sf::Color(60, 220, 160));
        wx -= et.getLocalBounds().size.x + 8.f;
        et.setPosition({ wx, wy });
        w.draw(et);
    }

    // Kolona labele
    sf::Text cItem(m_font, "ITEM", 10); cItem.setFillColor(sf::Color(110, 110, 140));
    sf::Text cBuy(m_font, "BUY", 10); cBuy.setFillColor(sf::Color(80, 170, 80));
    sf::Text cSell(m_font, "SELL", 10); cSell.setFillColor(sf::Color(170, 80, 60));
    cItem.setPosition({ px + 12.f,  py + HEADER_H - 16.f });
    cBuy.setPosition({ px + 168.f, py + HEADER_H - 16.f });
    cSell.setPosition({ px + 218.f, py + HEADER_H - 16.f });
    w.draw(cItem); w.draw(cBuy); w.draw(cSell);
}

// -------------------------------------------------------
//  Item list sa scrollom
// -------------------------------------------------------
void MarketUI::drawItemList(sf::RenderWindow& w,
    const std::vector<MarketEntry>& entries,
    const Inventory& inv,
    float px, float py) {
    float listTop = py + HEADER_H;
    float listH = PANEL_H - HEADER_H - ACTION_H;

    // Clip zone (simuliramo sa scissor ili prosto ne crtamo van)
    for (int i = 0; i < VISIBLE_ROWS; ++i) {
        int idx = m_scrollOffset + i;
        if (idx >= static_cast<int>(entries.size())) break;

        const MarketEntry& e = entries[idx];
        float rowY = listTop + i * ROW_H;
        bool  sel = (idx == m_selectedIdx);

        // Row bg
        if (sel) {
            sf::RectangleShape bg({ LIST_W, ROW_H - 1.f });
            bg.setPosition({ px + 1.f, rowY });
            bg.setFillColor(sf::Color(45, 38, 15, 210));
            bg.setOutlineThickness(1.f);
            bg.setOutlineColor(sf::Color(160, 130, 40, 180));
            w.draw(bg);
        }

        // Item icon
        sf::RectangleShape icon({ 20.f, 20.f });
        icon.setPosition({ px + 8.f, rowY + (ROW_H - 20.f) * 0.5f });
        icon.setFillColor(e.color);
        icon.setOutlineThickness(1.f);
        icon.setOutlineColor(sf::Color(0, 0, 0, 100));
        w.draw(icon);

        // Naziv
        sf::Text nt(m_font, e.name, 12);
        nt.setFillColor(sel ? sf::Color(255, 215, 80) : sf::Color(200, 200, 210));
        nt.setPosition({ px + 34.f, rowY + 4.f });
        w.draw(nt);

        // Posjedujes
        int own = inv.countOf(e.item);
        sf::Text ot(m_font, "x" + std::to_string(own), 9);
        ot.setFillColor(sf::Color(120, 120, 150));
        ot.setPosition({ px + 34.f, rowY + 20.f });
        w.draw(ot);

        // Buy price
        sf::Text bt(m_font, MarketSystem::formatPrice(e.currentBuyPrice), 11);
        bt.setFillColor(e.canBuy ? sf::Color(80, 200, 80) : sf::Color(60, 70, 60));
        bt.setPosition({ px + 158.f, rowY + 14.f });
        w.draw(bt);

        // Sell price
        sf::Text st(m_font, MarketSystem::formatPrice(e.currentSellPrice), 11);
        st.setFillColor(e.canSell ? sf::Color(200, 100, 70) : sf::Color(70, 60, 60));
        st.setPosition({ px + 210.f, rowY + 14.f });
        w.draw(st);

        // Trend
        sf::RectangleShape trend({ 3.f, ROW_H - 2.f });
        trend.setPosition({ px + LIST_W + 2.f, rowY + 1.f });
        trend.setFillColor(e.currentBuyPrice > e.baseBuyPrice
            ? sf::Color(200, 60, 60, 140) : sf::Color(60, 200, 60, 140));
        w.draw(trend);
    }
}

// -------------------------------------------------------
//  Scrollbar
// -------------------------------------------------------
void MarketUI::drawScrollbar(sf::RenderWindow& w, float px, float py,
    int totalItems) {
    if (totalItems <= VISIBLE_ROWS) return;

    float trackX = px + LIST_W + 6.f;
    float trackY = py + HEADER_H;
    float trackH = PANEL_H - HEADER_H - ACTION_H;
    float trackW = 8.f;

    // Track
    sf::RectangleShape track({ trackW, trackH });
    track.setPosition({ trackX, trackY });
    track.setFillColor(sf::Color(20, 22, 35));
    track.setOutlineThickness(1.f);
    track.setOutlineColor(sf::Color(50, 55, 80));
    w.draw(track);

    // Thumb
    float ratio = static_cast<float>(VISIBLE_ROWS) / totalItems;
    float thumbH = trackH * ratio;
    float thumbY = trackY + (trackH - thumbH) *
        static_cast<float>(m_scrollOffset) /
        std::max(1, totalItems - VISIBLE_ROWS);

    sf::RectangleShape thumb({ trackW - 2.f, thumbH });
    thumb.setPosition({ trackX + 1.f, thumbY });
    thumb.setFillColor(sf::Color(120, 100, 40));
    w.draw(thumb);
}

// -------------------------------------------------------
//  Detail panel (desna strana)
// -------------------------------------------------------
void MarketUI::drawDetail(sf::RenderWindow& w, const MarketEntry& e,
    int playerCount, const Wallet& wallet,
    float px, float py) {
    float dx = px + LIST_W + 26.f;
    float dy = py + HEADER_H + 8.f;
    float dw = PANEL_W - LIST_W - 36.f;

    // Naziv + boja
    sf::Text nameT(m_font, e.name, 17);
    nameT.setFillColor(e.color);
    nameT.setPosition({ dx, dy });
    w.draw(nameT);

    sf::Text descT(m_font, e.description, 10);
    descT.setFillColor(sf::Color(140, 140, 160));
    descT.setPosition({ dx, dy + 24.f });
    w.draw(descT);

    // Cijene
    // Buy row sa gold/silver ikonom
    {
        sf::Sprite gi(m_goldTex); gi.setScale({ 1.f,1.f });
        gi.setPosition({ dx, dy + 43.f });
        w.draw(gi);
        sf::Text buyT(m_font,
            MarketSystem::formatPrice(e.currentBuyPrice) + " / item", 12);
        buyT.setFillColor(e.canBuy ? sf::Color(80, 210, 80) : sf::Color(70, 70, 70));
        buyT.setPosition({ dx + 20.f, dy + 42.f });
        w.draw(buyT);
    }
    {
        sf::Sprite si(m_silverTex); si.setScale({ 1.f,1.f });
        si.setPosition({ dx, dy + 61.f });
        w.draw(si);
        sf::Text sellT(m_font,
            MarketSystem::formatPrice(e.currentSellPrice) + " / item", 12);
        sellT.setFillColor(e.canSell ? sf::Color(210, 100, 70) : sf::Color(70, 70, 70));
        sellT.setPosition({ dx + 20.f, dy + 60.f });
        w.draw(sellT);
    }

    // Posjedujes
    sf::Text ownT(m_font, "You own: " + std::to_string(playerCount), 11);
    ownT.setFillColor(sf::Color(160, 160, 200));
    ownT.setPosition({ dx, dy + 76.f });
    w.draw(ownT);

    // Sparkline
    // (placeholder — stvarna historija dolazi iz market.getHistory)
    sf::Text histLbl(m_font, "Price history (last 24 ticks)", 9);
    histLbl.setFillColor(sf::Color(100, 100, 130));
    histLbl.setPosition({ dx, dy + 94.f });
    w.draw(histLbl);
}

// -------------------------------------------------------
//  Sparkline
// -------------------------------------------------------
void MarketUI::drawSparkline(sf::RenderWindow& w,
    const std::vector<MarketSystem::PricePoint>& hist,
    float bx, float by, float bw, float bh) {
    if (hist.size() < 2) return;

    int minP = hist[0].buyPrice, maxP = hist[0].buyPrice;
    for (auto& p : hist) {
        minP = std::min(minP, p.buyPrice);
        maxP = std::max(maxP, p.buyPrice);
    }
    if (maxP == minP) maxP = minP + 1;

    sf::RectangleShape bg({ bw, bh });
    bg.setPosition({ bx, by });
    bg.setFillColor(sf::Color(16, 18, 32, 200));
    bg.setOutlineThickness(1.f);
    bg.setOutlineColor(sf::Color(50, 55, 80));
    w.draw(bg);

    sf::VertexArray line(sf::PrimitiveType::LineStrip, hist.size());
    for (int i = 0; i < static_cast<int>(hist.size()); ++i) {
        float t = static_cast<float>(i) / (hist.size() - 1);
        float x = bx + t * bw;
        float norm = static_cast<float>(hist[i].buyPrice - minP) /
            static_cast<float>(maxP - minP);
        float y = by + bh - norm * bh * 0.85f - 4.f;
        sf::Color col = (i > 0 && hist[i].buyPrice >= hist[i - 1].buyPrice)
            ? sf::Color(70, 210, 90) : sf::Color(210, 70, 70);
        line[i] = { {x, y}, col };
    }
    w.draw(line);
}

// -------------------------------------------------------
//  Actions
// -------------------------------------------------------
void MarketUI::drawActions(sf::RenderWindow& w, const MarketEntry* e,
    int playerCount, const Wallet& wallet,
    float px, float py) {
    if (!e) return;
    float ay = py + PANEL_H - ACTION_H;

    sf::RectangleShape sep({ PANEL_W, 2.f });
    sep.setPosition({ px, ay });
    sep.setFillColor(sf::Color(70, 60, 25, 160));
    w.draw(sep);

    // Qty controls
    float qy = ay + 12.f;
    sf::Text qlbl(m_font, "Qty:", 12);
    qlbl.setFillColor(sf::Color(160, 160, 180));
    qlbl.setPosition({ px + 14.f, qy });
    w.draw(qlbl);

    btn(w, m_font, px + 56.f, qy - 2.f, 26.f, 24.f, "-",
        sf::Color(180, 120, 80), m_quantity > 1);
    sf::Text qval(m_font, std::to_string(m_quantity), 14);
    qval.setFillColor(sf::Color(240, 215, 120));
    qval.setPosition({ px + 90.f, qy });
    w.draw(qval);
    btn(w, m_font, px + 118.f, qy - 2.f, 26.f, 24.f, "+",
        sf::Color(100, 180, 100), m_quantity < 99);

    int totalBuy = e->currentBuyPrice * m_quantity;
    int totalSell = e->currentSellPrice * m_quantity;
    bool canBuy = e->canBuy && wallet.canAfford(totalBuy);
    bool canSell = e->canSell && playerCount >= m_quantity;

    sf::Text tbT(m_font, "Total buy:  " + MarketSystem::formatPrice(totalBuy), 11);
    tbT.setFillColor(canBuy ? sf::Color(80, 200, 80) : sf::Color(160, 60, 60));
    tbT.setPosition({ px + 160.f, ay + 10.f });
    w.draw(tbT);

    sf::Text tsT(m_font, "Total sell: " + MarketSystem::formatPrice(totalSell), 11);
    tsT.setFillColor(canSell ? sf::Color(200, 130, 70) : sf::Color(120, 80, 60));
    tsT.setPosition({ px + 160.f, ay + 26.f });
    w.draw(tsT);

    // BUY / SELL buttons
    btn(w, m_font, px + 14.f, ay + 48.f, 110.f, 36.f,
        "BUY", sf::Color(70, 210, 70), canBuy);
    btn(w, m_font, px + 134.f, ay + 48.f, 110.f, 36.f,
        "SELL", sf::Color(210, 110, 60), canSell);

    // Sparkline desno od dugmadi
    float dx = px + LIST_W + 26.f;
    // Nacrtaj sparkline ovdje ispod detail sekcije
}

// -------------------------------------------------------
//  Status
// -------------------------------------------------------
void MarketUI::drawStatusMsg(sf::RenderWindow& w, float px, float py) {
    if (m_statusTimer <= 0.f || m_statusMsg.empty()) return;
    float alpha = std::min(1.f, m_statusTimer) * 255.f;
    sf::Color col = m_statusOk
        ? sf::Color(70, 220, 70, static_cast<uint8_t>(alpha))
        : sf::Color(220, 70, 70, static_cast<uint8_t>(alpha));
    sf::Text msg(m_font, m_statusMsg, 13);
    msg.setFillColor(col);
    msg.setPosition({ px + 14.f, py + PANEL_H - ACTION_H - 20.f });
    w.draw(msg);
}

// -------------------------------------------------------
//  Click handler
// -------------------------------------------------------
bool MarketUI::handleClick(sf::Vector2f mouse,
    MarketSystem& market,
    Inventory& inv,
    Wallet& wallet) {
    if (!m_open) return false;

    sf::Vector2u ws = m_windowSize;
    float px = (ws.x - PANEL_W) * 0.5f;
    float py = (ws.y - PANEL_H) * 0.5f;

    const auto& entries = market.getEntries();

    // Klik na red u listi
    float listTop = py + HEADER_H;
    for (int i = 0; i < VISIBLE_ROWS; ++i) {
        int idx = m_scrollOffset + i;
        if (idx >= static_cast<int>(entries.size())) break;
        float rowY = listTop + i * ROW_H;
        sf::FloatRect row(sf::Vector2f{ px, rowY }, sf::Vector2f{ LIST_W, ROW_H });
        if (row.contains(mouse)) {
            m_selectedIdx = idx;
            m_quantity = 1;
            return true;
        }
    }

    float ay = py + PANEL_H - ACTION_H;

    // [ - ] qty
    if (sf::FloatRect(sf::Vector2f{ px + 56.f, ay + 10.f },
        sf::Vector2f{ 26.f,24.f }).contains(mouse)) {
        m_quantity = std::max(1, m_quantity - 1);
        return true;
    }
    // [ + ] qty
    if (sf::FloatRect(sf::Vector2f{ px + 118.f, ay + 10.f },
        sf::Vector2f{ 26.f,24.f }).contains(mouse)) {
        m_quantity = std::min(99, m_quantity + 1);
        return true;
    }

    if (m_selectedIdx >= static_cast<int>(entries.size())) return false;
    const MarketEntry& sel = entries[m_selectedIdx];

    // BUY
    if (sf::FloatRect(sf::Vector2f{ px + 14.f, ay + 48.f },
        sf::Vector2f{ 110.f,36.f }).contains(mouse)) {
        if (market.buyItem(sel.item, m_quantity, wallet, inv)) {
            m_statusMsg = "Bought " + std::to_string(m_quantity) +
                "x " + sel.name + "!";
            m_statusOk = true;
        }
        else {
            m_statusMsg = "Not enough funds!";
            m_statusOk = false;
        }
        m_statusTimer = 2.5f;
        return true;
    }

    // SELL
    if (sf::FloatRect(sf::Vector2f{ px + 134.f, ay + 48.f },
        sf::Vector2f{ 110.f,36.f }).contains(mouse)) {
        if (market.sellItem(sel.item, m_quantity, wallet, inv)) {
            m_statusMsg = "Sold " + std::to_string(m_quantity) +
                "x " + sel.name + "!";
            m_statusOk = true;
        }
        else {
            m_statusMsg = "Not enough items!";
            m_statusOk = false;
        }
        m_statusTimer = 2.5f;
        return true;
    }

    return false;
}

void MarketUI::handleScroll(float delta) {
    if (!m_open) return;
    // Scroll u listi
    int maxOffset = 0; // ovo se updatuje u draw na osnovu entries.size()
    if (delta < 0)
        m_scrollOffset = std::min(m_scrollOffset + 1, 20);
    else
        m_scrollOffset = std::max(m_scrollOffset - 1, 0);
    // Klamp na aktuelni max radi se u draw
}