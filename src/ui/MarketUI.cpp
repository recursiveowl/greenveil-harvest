#include "ui/MarketUI.h"
#include <cmath>
#include <algorithm>

MarketUI::MarketUI() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

void MarketUI::open(float gameHour) {
    m_open = true;
    m_selectedIdx = 0;
    m_quantity = 1;
    m_statusMsg = "";
    m_statusTimer = 0.f;
}

void MarketUI::close() { m_open = false; }

// -------------------------------------------------------
//  Helper: nacrtaj pravougaonik + tekst (dugme)
// -------------------------------------------------------
static void drawButton(sf::RenderWindow& window, sf::Font& font,
    float x, float y, float w, float h,
    const std::string& label, sf::Color borderCol) {
    sf::RectangleShape bg({ w, h });
    bg.setPosition({ x, y });
    bg.setFillColor(sf::Color(30, 30, 40));
    bg.setOutlineThickness(1.5f);
    bg.setOutlineColor(borderCol);
    window.draw(bg);

    sf::Text t(font, label, 13);
    t.setFillColor(borderCol);
    auto tb = t.getLocalBounds();
    t.setPosition({ x + (w - tb.size.x) * 0.5f,
                   y + (h - tb.size.y) * 0.5f - 2.f });
    window.draw(t);
}

// -------------------------------------------------------
//  Draw
// -------------------------------------------------------
void MarketUI::draw(sf::RenderWindow& window,
    MarketSystem& market,
    Inventory& inv,
    Wallet& wallet,
    float gameHour) {
    if (!m_open) return;
    if (m_statusTimer > 0.f) m_statusTimer -= 0.016f;

    sf::Vector2u ws = window.getSize();
    float px = (ws.x - PANEL_W) * 0.5f;
    float py = (ws.y - PANEL_H) * 0.5f;

    // Panel
    sf::RectangleShape panel({ PANEL_W, PANEL_H });
    panel.setPosition({ px, py });
    panel.setFillColor(sf::Color(12, 14, 22, 248));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(180, 150, 60, 220));
    window.draw(panel);

    // Separator lista | detalji
    sf::RectangleShape sep({ 2.f, PANEL_H - HEADER_H - ACTION_H });
    sep.setPosition({ px + 280.f, py + HEADER_H });
    sep.setFillColor(sf::Color(80, 70, 40, 180));
    window.draw(sep);

    drawHeader(window, wallet, gameHour, px, py);

    const auto& entries = market.getEntries();
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        int playerCount = inv.countOf(entries[i].item);
        drawEntryRow(window, entries[i], i, i == m_selectedIdx, playerCount, px, py);
    }

    // Desna strana — detalji
    if (m_selectedIdx < static_cast<int>(entries.size())) {
        const MarketEntry& sel = entries[m_selectedIdx];
        int playerCount = inv.countOf(sel.item);

        float dx = px + 292.f;
        float dy = py + HEADER_H + 12.f;

        sf::Text nameT(m_font, sel.name, 17);
        nameT.setFillColor(sel.color);
        nameT.setPosition({ dx, dy });
        window.draw(nameT);

        sf::Text descT(m_font, sel.description, 11);
        descT.setFillColor(sf::Color(160, 160, 160));
        descT.setPosition({ dx, dy + 22.f });
        window.draw(descT);

        sf::Text buyT(m_font,
            "Buy:  " + MarketSystem::formatPrice(sel.currentBuyPrice) + " each", 13);
        buyT.setFillColor(sf::Color(100, 200, 100));
        buyT.setPosition({ dx, dy + 46.f });
        window.draw(buyT);

        sf::Text sellT(m_font,
            "Sell: " + MarketSystem::formatPrice(sel.currentSellPrice) + " each", 13);
        sellT.setFillColor(sf::Color(200, 120, 80));
        sellT.setPosition({ dx, dy + 64.f });
        window.draw(sellT);

        float mult = MarketSystem::getDayPriceMult(gameHour);
        std::string multStr = "Market activity: ";
        sf::Color   multCol;
        if (mult > 1.2f) { multStr += "HIGH";   multCol = sf::Color(80, 220, 80); }
        else if (mult > 1.0f) { multStr += "NORMAL"; multCol = sf::Color(220, 220, 80); }
        else { multStr += "LOW";    multCol = sf::Color(220, 100, 80); }

        sf::Text multT(m_font, multStr, 11);
        multT.setFillColor(multCol);
        multT.setPosition({ dx, dy + 84.f });
        window.draw(multT);

        sf::Text ownT(m_font, "You own: " + std::to_string(playerCount), 12);
        ownT.setFillColor(sf::Color(180, 180, 220));
        ownT.setPosition({ dx, dy + 102.f });
        window.draw(ownT);

        // Sparkline
        auto& hist = market.getHistory(sel.item);
        float bx = px + 292.f, by = py + HEADER_H + 130.f;
        if (hist.size() >= 2) {
            sf::Text histLabel(m_font, "Price history", 10);
            histLabel.setFillColor(sf::Color(120, 120, 140));
            histLabel.setPosition({ bx, by - 14.f });
            window.draw(histLabel);
            drawSparkline(window, hist, bx, by, 310.f, 60.f);
        }
        else {
            sf::Text noHist(m_font, "Price history available\nafter market opens...", 10);
            noHist.setFillColor(sf::Color(100, 100, 120));
            noHist.setPosition({ bx, by });
            window.draw(noHist);
        }

        drawActions(window, &sel, playerCount, wallet, px, py);
    }

    drawStatusMsg(window, px, py);

    sf::Text hint(m_font, "[ M ] Close  |  Click to select  |  Scroll = qty", 11);
    hint.setFillColor(sf::Color(80, 80, 100));
    hint.setPosition({ px + 12.f, py + PANEL_H - 20.f });
    window.draw(hint);
}

// -------------------------------------------------------
//  Header
// -------------------------------------------------------
void MarketUI::drawHeader(sf::RenderWindow& window, const Wallet& wallet,
    float gameHour, float px, float py) {
    sf::Text title(m_font, "GREENVEIL MARKET", 20);
    title.setFillColor(sf::Color(220, 190, 80));
    title.setPosition({ px + 14.f, py + 10.f });
    window.draw(title);

    sf::RectangleShape line({ PANEL_W, 2.f });
    line.setPosition({ px, py + HEADER_H - 4.f });
    line.setFillColor(sf::Color(100, 80, 30, 180));
    window.draw(line);

    // Emerald
    if (wallet.emerald > 0) {
        sf::CircleShape gem(6.f);
        gem.setOrigin({ 6.f, 6.f });
        gem.setFillColor(sf::Color(60, 220, 160));
        gem.setOutlineThickness(1.f);
        gem.setOutlineColor(sf::Color(30, 180, 120));
        gem.setPosition({ px + PANEL_W - 210.f, py + 22.f });
        window.draw(gem);
        sf::Text et(m_font, std::to_string(wallet.emerald) + " em", 13);
        et.setFillColor(sf::Color(60, 220, 160));
        et.setPosition({ px + PANEL_W - 198.f, py + 14.f });
        window.draw(et);
    }

    // Gold
    sf::CircleShape goldCoin(7.f);
    goldCoin.setOrigin({ 7.f, 7.f });
    goldCoin.setFillColor(sf::Color(220, 180, 30));
    goldCoin.setOutlineThickness(1.f);
    goldCoin.setOutlineColor(sf::Color(160, 120, 10));
    goldCoin.setPosition({ px + PANEL_W - 140.f, py + 22.f });
    window.draw(goldCoin);

    sf::Text gt(m_font, std::to_string(wallet.gold) + "g", 14);
    gt.setFillColor(sf::Color(240, 200, 60));
    gt.setPosition({ px + PANEL_W - 126.f, py + 14.f });
    window.draw(gt);

    // Silver
    sf::CircleShape silvCoin(6.f);
    silvCoin.setOrigin({ 6.f, 6.f });
    silvCoin.setFillColor(sf::Color(180, 180, 200));
    silvCoin.setOutlineThickness(1.f);
    silvCoin.setOutlineColor(sf::Color(120, 120, 150));
    silvCoin.setPosition({ px + PANEL_W - 72.f, py + 22.f });
    window.draw(silvCoin);

    sf::Text st(m_font, std::to_string(wallet.silver) + "s", 14);
    st.setFillColor(sf::Color(200, 200, 220));
    st.setPosition({ px + PANEL_W - 58.f, py + 14.f });
    window.draw(st);

    // Kolona labele
    sf::Text colItem(m_font, "ITEM", 11);
    colItem.setFillColor(sf::Color(120, 120, 140));
    colItem.setPosition({ px + 14.f, py + HEADER_H - 20.f });
    window.draw(colItem);

    sf::Text colBuy(m_font, "BUY", 11);
    colBuy.setFillColor(sf::Color(100, 180, 100));
    colBuy.setPosition({ px + 160.f, py + HEADER_H - 20.f });
    window.draw(colBuy);

    sf::Text colSell(m_font, "SELL", 11);
    colSell.setFillColor(sf::Color(180, 100, 80));
    colSell.setPosition({ px + 218.f, py + HEADER_H - 20.f });
    window.draw(colSell);
}

// -------------------------------------------------------
//  Entry row
// -------------------------------------------------------
void MarketUI::drawEntryRow(sf::RenderWindow& window, const MarketEntry& e,
    int idx, bool selected, int playerCount,
    float px, float py) {
    float rowY = py + HEADER_H + idx * ROW_H;

    if (selected) {
        sf::RectangleShape bg({ 278.f, ROW_H - 2.f });
        bg.setPosition({ px + 1.f, rowY + 1.f });
        bg.setFillColor(sf::Color(40, 35, 15, 200));
        bg.setOutlineThickness(1.f);
        bg.setOutlineColor(sf::Color(180, 150, 40, 180));
        window.draw(bg);
    }

    sf::RectangleShape icon({ 24.f, 24.f });
    icon.setPosition({ px + 10.f, rowY + (ROW_H - 24.f) * 0.5f });
    icon.setFillColor(e.color);
    icon.setOutlineThickness(1.f);
    icon.setOutlineColor(sf::Color(0, 0, 0, 120));
    window.draw(icon);

    sf::Text nameT(m_font, e.name, 13);
    nameT.setFillColor(selected ? sf::Color(255, 220, 100) : sf::Color(200, 200, 200));
    nameT.setPosition({ px + 40.f, rowY + 8.f });
    window.draw(nameT);

    sf::Text ownT(m_font, "x" + std::to_string(playerCount), 10);
    ownT.setFillColor(sf::Color(140, 140, 160));
    ownT.setPosition({ px + 40.f, rowY + 26.f });
    window.draw(ownT);

    sf::Text buyT(m_font, MarketSystem::formatPrice(e.currentBuyPrice), 12);
    buyT.setFillColor(sf::Color(100, 200, 100));
    buyT.setPosition({ px + 155.f, rowY + 18.f });
    window.draw(buyT);

    sf::Text sellT(m_font, MarketSystem::formatPrice(e.currentSellPrice), 12);
    sellT.setFillColor(sf::Color(200, 120, 80));
    sellT.setPosition({ px + 213.f, rowY + 18.f });
    window.draw(sellT);

    // Trend bar
    sf::RectangleShape trend({ 4.f, ROW_H - 4.f });
    trend.setPosition({ px + 275.f, rowY + 2.f });
    trend.setFillColor(e.currentBuyPrice > e.baseBuyPrice
        ? sf::Color(220, 80, 80, 160)
        : sf::Color(80, 200, 80, 160));
    window.draw(trend);
}

// -------------------------------------------------------
//  Actions panel
// -------------------------------------------------------
void MarketUI::drawActions(sf::RenderWindow& window, const MarketEntry* e,
    int playerCount, const Wallet& wallet,
    float px, float py) {
    if (!e) return;
    float ay = py + PANEL_H - ACTION_H;

    sf::RectangleShape sep({ PANEL_W, 2.f });
    sep.setPosition({ px, ay });
    sep.setFillColor(sf::Color(80, 70, 40, 180));
    window.draw(sep);

    // Qty label
    sf::Text qLabel(m_font, "Qty:", 13);
    qLabel.setFillColor(sf::Color(160, 160, 180));
    qLabel.setPosition({ px + 14.f, ay + 14.f });
    window.draw(qLabel);

    // [ - ]
    drawButton(window, m_font, px + 60.f, ay + 10.f, 24.f, 24.f,
        "-", sf::Color(180, 120, 80));

    // qty value
    sf::Text qVal(m_font, std::to_string(m_quantity), 15);
    qVal.setFillColor(sf::Color(240, 220, 140));
    qVal.setPosition({ px + 90.f, ay + 12.f });
    window.draw(qVal);

    // [ + ]
    drawButton(window, m_font, px + 120.f, ay + 10.f, 24.f, 24.f,
        "+", sf::Color(100, 180, 100));

    // Totali
    int totalBuy = e->currentBuyPrice * m_quantity;
    int totalSell = e->currentSellPrice * m_quantity;

    sf::Text totalBuyT(m_font,
        "Total buy:  " + MarketSystem::formatPrice(totalBuy), 12);
    totalBuyT.setFillColor(wallet.canAfford(totalBuy)
        ? sf::Color(100, 200, 100) : sf::Color(180, 80, 80));
    totalBuyT.setPosition({ px + 160.f, ay + 12.f });
    window.draw(totalBuyT);

    sf::Text totalSellT(m_font,
        "Total sell: " + MarketSystem::formatPrice(totalSell), 12);
    totalSellT.setFillColor(playerCount >= m_quantity
        ? sf::Color(200, 140, 80) : sf::Color(140, 80, 80));
    totalSellT.setPosition({ px + 160.f, ay + 30.f });
    window.draw(totalSellT);

    // BUY dugme
    bool canBuy = e->canBuy && wallet.canAfford(totalBuy);
    bool canSell = e->canSell && playerCount >= m_quantity;

    drawButton(window, m_font, px + 14.f, ay + 48.f, 100.f, 32.f,
        "BUY", canBuy ? sf::Color(80, 200, 80) : sf::Color(60, 80, 60));
    drawButton(window, m_font, px + 124.f, ay + 48.f, 100.f, 32.f,
        "SELL", canSell ? sf::Color(200, 120, 60) : sf::Color(80, 60, 50));
}

// -------------------------------------------------------
//  Sparkline
// -------------------------------------------------------
void MarketUI::drawSparkline(sf::RenderWindow& window,
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
    bg.setFillColor(sf::Color(20, 20, 35, 180));
    bg.setOutlineThickness(1.f);
    bg.setOutlineColor(sf::Color(60, 60, 80));
    window.draw(bg);

    sf::VertexArray line(sf::PrimitiveType::LineStrip,
        static_cast<std::size_t>(hist.size()));
    for (int i = 0; i < static_cast<int>(hist.size()); ++i) {
        float t = static_cast<float>(i) / (hist.size() - 1);
        float x = bx + t * bw;
        float norm = static_cast<float>(hist[i].buyPrice - minP) /
            static_cast<float>(maxP - minP);
        float y = by + bh - norm * bh * 0.85f - 4.f;
        sf::Color col = (i > 0 && hist[i].buyPrice >= hist[i - 1].buyPrice)
            ? sf::Color(80, 220, 100) : sf::Color(220, 80, 80);
        line[i] = { {x, y}, col };
    }
    window.draw(line);

    sf::Text minT(m_font, MarketSystem::formatPrice(minP), 9);
    minT.setFillColor(sf::Color(180, 80, 80));
    minT.setPosition({ bx + 2.f, by + bh - 14.f });
    window.draw(minT);

    sf::Text maxT(m_font, MarketSystem::formatPrice(maxP), 9);
    maxT.setFillColor(sf::Color(80, 200, 80));
    maxT.setPosition({ bx + 2.f, by + 2.f });
    window.draw(maxT);
}

// -------------------------------------------------------
//  Status msg
// -------------------------------------------------------
void MarketUI::drawStatusMsg(sf::RenderWindow& window, float px, float py) {
    if (m_statusTimer <= 0.f || m_statusMsg.empty()) return;
    float alpha = std::min(1.f, m_statusTimer) * 255.f;
    sf::Color col = m_statusOk
        ? sf::Color(80, 220, 80, static_cast<uint8_t>(alpha))
        : sf::Color(220, 80, 80, static_cast<uint8_t>(alpha));
    sf::Text msg(m_font, m_statusMsg, 14);
    msg.setFillColor(col);
    msg.setPosition({ px + 14.f, py + PANEL_H - ACTION_H - 22.f });
    window.draw(msg);
}

// -------------------------------------------------------
//  Click handler
// -------------------------------------------------------
bool MarketUI::handleClick(sf::Vector2f mouse,
    MarketSystem& market,
    Inventory& inv,
    Wallet& wallet) {
    if (!m_open) return false;

    sf::Vector2u ws = { 1280u, 720u };
    float px = (ws.x - PANEL_W) * 0.5f;
    float py = (ws.y - PANEL_H) * 0.5f;

    const auto& entries = market.getEntries();

    // Klik na red
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        float rowY = py + HEADER_H + i * ROW_H;
        // SFML3: contains prima Vector2f
        sf::FloatRect row(sf::Vector2f{ px, rowY }, sf::Vector2f{ 280.f, ROW_H });
        if (row.contains(mouse)) {
            m_selectedIdx = i;
            m_quantity = 1;
            return true;
        }
    }

    float ay = py + PANEL_H - ACTION_H;

    // [ - ]
    if (sf::FloatRect(sf::Vector2f{ px + 60.f, ay + 10.f }, sf::Vector2f{ 24.f,24.f }).contains(mouse)) {
        m_quantity = std::max(1, m_quantity - 1); return true;
    }
    // [ + ]
    if (sf::FloatRect(sf::Vector2f{ px + 120.f, ay + 10.f }, sf::Vector2f{ 24.f,24.f }).contains(mouse)) {
        m_quantity = std::min(99, m_quantity + 1); return true;
    }

    if (m_selectedIdx >= static_cast<int>(entries.size())) return false;
    const MarketEntry& sel = entries[m_selectedIdx];

    // BUY
    if (sf::FloatRect(sf::Vector2f{ px + 14.f, ay + 48.f }, sf::Vector2f{ 100.f,32.f }).contains(mouse)) {
        if (market.buyItem(sel.item, m_quantity, wallet, inv)) {
            m_statusMsg = "Bought " + std::to_string(m_quantity) + "x " + sel.name + "!";
            m_statusOk = true;
        }
        else {
            m_statusMsg = "Not enough funds!";
            m_statusOk = false;
        }
        m_statusTimer = 2.f;
        return true;
    }

    // SELL
    if (sf::FloatRect(sf::Vector2f{ px + 124.f, ay + 48.f }, sf::Vector2f{ 100.f,32.f }).contains(mouse)) {
        if (market.sellItem(sel.item, m_quantity, wallet, inv)) {
            m_statusMsg = "Sold " + std::to_string(m_quantity) + "x " + sel.name + "!";
            m_statusOk = true;
        }
        else {
            m_statusMsg = "Not enough items!";
            m_statusOk = false;
        }
        m_statusTimer = 2.f;
        return true;
    }

    return false;
}

void MarketUI::handleScroll(float delta) {
    if (!m_open) return;
    m_quantity = std::max(1, std::min(99, m_quantity + (delta > 0 ? 1 : -1)));
}