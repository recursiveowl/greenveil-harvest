#include "ui/LoadingScreen.h"
#include <algorithm>
#include <cmath>

const std::vector<std::string> LoadingScreen::s_tips = {
    // Farming
    "Healthy crops produce better harvests.",
    "Different crops thrive in different seasons.",
    "Water your crops regularly to maximize yield.",
    "Harvest crops on time before they wither.",
    "Fertile soil can make all the difference.",
    // Resources
    "Wood and stone are the foundation of every successful farm.",
    "Keep a stockpile of essential resources for future projects.",
    "Rare ores are often hidden in the deepest places.",
    "Don't sell everything - you may need those materials later.",
    // Animals
    "Happy animals produce higher-quality goods.",
    "Feed your livestock every day.",
    "Expanding your barns allows you to raise more animals.",
    "Taking good care of your animals always pays off.",
    // Exploration
    "Every corner of Greenveil hides something worth discovering.",
    "Some locations change with the seasons.",
    "Hidden paths often lead to valuable rewards.",
    "Keep exploring - there's always something new to find.",
    // Crafting
    "Better tools save both time and energy.",
    "Upgrade your equipment whenever possible.",
    "Crafting stations unlock new possibilities.",
    "Organize your storage to work more efficiently.",
    // Villagers
    "Talk to the villagers often - they may have useful advice.",
    "Building friendships can unlock unique rewards.",
    "Some villagers are only available at certain times of day.",
    "Gifts are a great way to strengthen relationships.",
    // Time & Seasons
    "Rainy days give your crops a break from watering.",
    "Prepare your farm before winter arrives.",
    "Plan your day wisely - there's only so much daylight.",
    "Every season brings new opportunities.",
    // General Tips
    "Always leave some space in your inventory before exploring.",
    "Saving your game regularly is never a bad idea.",
    "Every upgrade is an investment in your future.",
    "A well-organized farm is easier to manage.",
    // Lore
    "Greenveil's forests hold many forgotten secrets.",
    "Ancient ruins often hide valuable treasures.",
    "Not every legend is just a story.",
    "Some mysteries can only be uncovered through patience.",
    // Fun Tips
    "Chickens have an incredible talent for being in the way.",
    "Every great farmer started with a single seed.",
    "Sometimes the best plan is simply planting more potatoes.",
    "Curiosity often leads to unexpected discoveries.",
    // Rare Tips
    "Some secrets only appear under the full moon.",
    "Keep an eye out for unusual landmarks.",
    "Fortune favors those who love to explore.",
    "The smallest details can reveal the biggest secrets.",
};

LoadingScreen::LoadingScreen() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

void LoadingScreen::pickNewTip() {
    if (s_tips.empty()) return;
    std::uniform_int_distribution<size_t> dist(0, s_tips.size() - 1);
    std::string next;
    do {
        next = s_tips[dist(m_rng)];
    } while (s_tips.size() > 1 && next == m_currentTip);
    m_currentTip = next;
    m_tipTimer = TIP_INTERVAL;
}

void LoadingScreen::show() {
    m_targetProgress = 0.f;
    m_displayedProgress = 0.f;
    pickNewTip();
}

void LoadingScreen::setProgress(float t) {
    m_targetProgress = std::clamp(t, 0.f, 1.f);
}

void LoadingScreen::update(float dt) {
    float diff = m_targetProgress - m_displayedProgress;
    m_displayedProgress += diff * std::min(1.f, dt * LERP_SPEED);
    if (std::abs(m_targetProgress - m_displayedProgress) < 0.001f)
        m_displayedProgress = m_targetProgress;

    m_tipTimer -= dt;
    if (m_tipTimer <= 0.f)
        pickNewTip();
}

bool LoadingScreen::isFinished() const {
    return m_targetProgress >= 0.999f &&
        m_displayedProgress >= m_targetProgress - 0.002f;
}

void LoadingScreen::draw(sf::RenderWindow& window) {
    sf::Vector2u ws = window.getSize();

    sf::RectangleShape bg({ static_cast<float>(ws.x), static_cast<float>(ws.y) });
    bg.setFillColor(sf::Color(12, 13, 20));
    window.draw(bg);

    // Naslov
    sf::Text title(m_font, "GREENVEIL HARVEST", 34);
    title.setFillColor(sf::Color(220, 185, 70));
    auto tb = title.getLocalBounds();
    title.setPosition({ (ws.x - tb.size.x) * 0.5f, ws.y * 0.32f });
    window.draw(title);

    // "TIP" label
    sf::Text tipLbl(m_font, "TIP", 13);
    tipLbl.setFillColor(sf::Color(120, 120, 150));
    auto lb = tipLbl.getLocalBounds();
    tipLbl.setPosition({ (ws.x - lb.size.x) * 0.5f, ws.y * 0.52f });
    window.draw(tipLbl);

    // Tekst tipa
    sf::Text tipText(m_font, m_currentTip, 18);
    tipText.setFillColor(sf::Color(220, 220, 220));
    auto tpb = tipText.getLocalBounds();
    tipText.setPosition({ (ws.x - tpb.size.x) * 0.5f, ws.y * 0.52f + 22.f });
    window.draw(tipText);

    // Progress bar
    float barW = ws.x * 0.42f;
    float barH = 22.f;
    float barX = (ws.x - barW) * 0.5f;
    float barY = ws.y * 0.66f;

    sf::Text loadingT(m_font, "Loading...", 13);
    loadingT.setFillColor(sf::Color(110, 110, 140));
    auto ldb = loadingT.getLocalBounds();
    loadingT.setPosition({ (ws.x - ldb.size.x) * 0.5f, barY - 26.f });
    window.draw(loadingT);

    sf::RectangleShape barBg({ barW, barH });
    barBg.setPosition({ barX, barY });
    barBg.setFillColor(sf::Color(24, 26, 40));
    barBg.setOutlineThickness(2.f);
    barBg.setOutlineColor(sf::Color(160, 130, 50, 220));
    window.draw(barBg);

    float fillW = barW * std::clamp(m_displayedProgress, 0.f, 1.f);
    if (fillW > 0.f) {
        sf::RectangleShape barFill({ fillW, barH });
        barFill.setPosition({ barX, barY });
        barFill.setFillColor(sf::Color(220, 185, 70));
        window.draw(barFill);
    }

    // Procenat
    int pct = static_cast<int>(m_displayedProgress * 100.f + 0.5f);
    sf::Text pctText(m_font, std::to_string(pct) + "%", 14);
    pctText.setFillColor(sf::Color(200, 200, 210));
    auto pb = pctText.getLocalBounds();
    pctText.setPosition({ (ws.x - pb.size.x) * 0.5f, barY + barH + 10.f });
    window.draw(pctText);
}