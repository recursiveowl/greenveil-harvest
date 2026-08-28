#include "ui/MapUI.h"
#include "world/TileMap.h"

MapUI::MapUI() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

sf::Vector2f MapUI::worldToPanel(sf::Vector2f worldPos, const MapSystem& mapSystem) const {
    sf::Vector2u texSize = mapSystem.getTextureSize();
    if (texSize.x == 0 || texSize.y == 0 || m_panelSize.x == 0.f) return m_panelPos;

    float tilePx = worldPos.x / static_cast<float>(TileMap::TILE_SIZE) * MapSystem::PIXELS_PER_TILE;
    float tilePy = worldPos.y / static_cast<float>(TileMap::TILE_SIZE) * MapSystem::PIXELS_PER_TILE;

    float scaleX = m_panelSize.x / static_cast<float>(texSize.x);
    float scaleY = m_panelSize.y / static_cast<float>(texSize.y);

    return { m_panelPos.x + tilePx * scaleX, m_panelPos.y + tilePy * scaleY };
}

sf::Color MapUI::markerColor(MapMarkerType type) {
    switch (type) {
    case MapMarkerType::Market:      return sf::Color(90, 160, 230);
    case MapMarkerType::ForSale:     return sf::Color(225, 225, 225);
    case MapMarkerType::NPCRanch:    return sf::Color(230, 150, 60);
    case MapMarkerType::PlayerHouse: return sf::Color(120, 220, 120);
    }
    return sf::Color::White;
}

std::string MapUI::markerLabel(MapMarkerType type) {
    switch (type) {
    case MapMarkerType::Market:      return "Greenveil Market";
    case MapMarkerType::ForSale:     return "Ranch parcel - for sale";
    case MapMarkerType::NPCRanch:    return "NPC's Ranch";
    case MapMarkerType::PlayerHouse: return "Elias's House";
    }
    return "";
}

void MapUI::draw(sf::RenderWindow& window, const MapSystem& mapSystem,
    sf::Vector2f playerWorldPos, sf::Vector2f mouseUI) {
    m_windowSize = window.getSize();
    if (!m_open || !mapSystem.isBaked()) return;

    sf::Vector2u ws = m_windowSize;

    sf::RectangleShape overlay({ (float)ws.x, (float)ws.y });
    overlay.setFillColor(sf::Color(0, 0, 0, 190));
    window.draw(overlay);

    // Panel malo pomjeren lijevo da ostavi prostor za legendu desno
    float side = std::min(ws.x, ws.y) * 0.75f;
    m_panelSize = { side, side };
    m_panelPos = { (ws.x - side) * 0.5f - 110.f, (ws.y - side) * 0.5f - 10.f };

    sf::RectangleShape border(m_panelSize);
    border.setPosition(m_panelPos);
    border.setFillColor(sf::Color(10, 12, 20));
    border.setOutlineThickness(3.f);
    border.setOutlineColor(sf::Color(180, 160, 100, 220));
    window.draw(border);

    sf::Vector2u texSize = mapSystem.getTextureSize();
    sf::Sprite mapSprite(mapSystem.getTexture());
    mapSprite.setScale({ m_panelSize.x / (float)texSize.x,
                          m_panelSize.y / (float)texSize.y });
    mapSprite.setPosition(m_panelPos);
    window.draw(mapSprite);

    // Player marker (live pozicija)
    sf::Vector2f playerPanelPos = worldToPanel(playerWorldPos, mapSystem);
    drawPlayerDot(window, playerPanelPos, mouseUI);

    drawMarkers(window, mapSystem, mouseUI);
    drawLegend(window);

    sf::Text title(m_font, "MAP OF GREENVEIL", 20);
    title.setFillColor(sf::Color(220, 185, 70));
    auto tb = title.getLocalBounds();
    title.setPosition({ m_panelPos.x + (m_panelSize.x - tb.size.x) * 0.5f, m_panelPos.y - 34.f });
    window.draw(title);

    sf::Text hint(m_font, "[ Right Click / Esc ] Close", 12);
    hint.setFillColor(sf::Color(140, 140, 140));
    auto hb = hint.getLocalBounds();
    hint.setPosition({ m_panelPos.x + (m_panelSize.x - hb.size.x) * 0.5f,
                        m_panelPos.y + m_panelSize.y + 12.f });
    window.draw(hint);
}

void MapUI::drawPlayerDot(sf::RenderWindow& window, sf::Vector2f panelPos, sf::Vector2f mouseUI) {
    sf::CircleShape playerDot(6.f);
    playerDot.setOrigin({ 6.f, 6.f });
    playerDot.setPosition(panelPos);
    playerDot.setFillColor(sf::Color(255, 220, 60));
    playerDot.setOutlineThickness(2.f);
    playerDot.setOutlineColor(sf::Color(0, 0, 0, 200));
    window.draw(playerDot);

    float dx = mouseUI.x - panelPos.x;
    float dy = mouseUI.y - panelPos.y;
    if (dx * dx + dy * dy <= HOVER_RADIUS * HOVER_RADIUS)
        drawTooltip(window, mouseUI, "Farmer Elias (You)");
}

void MapUI::drawMarkers(sf::RenderWindow& window, const MapSystem& mapSystem, sf::Vector2f mouseUI) {
    for (const auto& marker : mapSystem.getMarkers()) {
        sf::Vector2f p = worldToPanel(marker.worldPos, mapSystem);

        sf::CircleShape dot(MARKER_RADIUS);
        dot.setOrigin({ MARKER_RADIUS, MARKER_RADIUS });
        dot.setPosition(p);
        dot.setFillColor(markerColor(marker.type));
        dot.setOutlineThickness(2.f);
        dot.setOutlineColor(sf::Color(0, 0, 0, 200));
        window.draw(dot);

        float dx = mouseUI.x - p.x;
        float dy = mouseUI.y - p.y;
        if (dx * dx + dy * dy <= HOVER_RADIUS * HOVER_RADIUS)
            drawTooltip(window, mouseUI, markerLabel(marker.type));
    }
}

void MapUI::drawTooltip(sf::RenderWindow& window, sf::Vector2f mouseUI, const std::string& text) {
    sf::Text label(m_font, text, 13);
    label.setFillColor(sf::Color(230, 230, 240));
    auto tb = label.getLocalBounds();

    sf::Vector2f boxPos{ mouseUI.x + 14.f, mouseUI.y - tb.size.y - 14.f };

    sf::RectangleShape bg({ tb.size.x + 16.f, tb.size.y + 12.f });
    bg.setPosition(boxPos);
    bg.setFillColor(sf::Color(14, 16, 28, 235));
    bg.setOutlineThickness(1.f);
    bg.setOutlineColor(sf::Color(80, 90, 130));
    window.draw(bg);

    label.setPosition({ boxPos.x + 8.f, boxPos.y + 5.f });
    window.draw(label);
}

void MapUI::drawLegend(sf::RenderWindow& window) {
    static constexpr MapMarkerType kTypes[] = {
        MapMarkerType::PlayerHouse, MapMarkerType::Market,
        MapMarkerType::ForSale, MapMarkerType::NPCRanch
    };
    static constexpr int kRowCount = 5; // 4 markera + "You"

    float legendX = m_panelPos.x + m_panelSize.x + 24.f;
    float legendY = m_panelPos.y;
    float rowH = 26.f;
    float legendW = 210.f;
    float legendH = rowH * kRowCount + 44.f;

    sf::RectangleShape bg({ legendW, legendH });
    bg.setPosition({ legendX, legendY });
    bg.setFillColor(sf::Color(12, 14, 24, 235));
    bg.setOutlineThickness(2.f);
    bg.setOutlineColor(sf::Color(180, 160, 100, 200));
    window.draw(bg);

    sf::Text title(m_font, "LEGEND", 14);
    title.setFillColor(sf::Color(220, 185, 70));
    title.setPosition({ legendX + 10.f, legendY + 8.f });
    window.draw(title);

    float y = legendY + 36.f;

    auto drawRow = [&](sf::Color col, const std::string& label) {
        sf::CircleShape dot(6.f);
        dot.setOrigin({ 6.f, 6.f });
        dot.setPosition({ legendX + 22.f, y + 8.f });
        dot.setFillColor(col);
        dot.setOutlineThickness(2.f);
        dot.setOutlineColor(sf::Color(0, 0, 0, 200));
        window.draw(dot);

        sf::Text t(m_font, label, 12);
        t.setFillColor(sf::Color(220, 220, 230));
        t.setPosition({ legendX + 38.f, y + 1.f });
        window.draw(t);
        y += rowH;
        };

    drawRow(sf::Color(255, 220, 60), "You");
    for (auto type : kTypes)
        drawRow(markerColor(type), markerLabel(type));
}