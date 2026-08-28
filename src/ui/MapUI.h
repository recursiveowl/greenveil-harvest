#pragma once
#include "pch.h"
#include "world/MapSystem.h"

class MapUI {
public:
    MapUI();

    void toggle() { m_open = !m_open; }
    void close() { m_open = false; }
    bool isOpen() const { return m_open; }

    void draw(sf::RenderWindow& window, const MapSystem& mapSystem,
        sf::Vector2f playerWorldPos, sf::Vector2f mouseUI);

private:
    sf::Vector2f worldToPanel(sf::Vector2f worldPos, const MapSystem& mapSystem) const;

    void drawMarkers(sf::RenderWindow& window, const MapSystem& mapSystem, sf::Vector2f mouseUI);
    void drawPlayerDot(sf::RenderWindow& window, sf::Vector2f panelPos, sf::Vector2f mouseUI);
    void drawLegend(sf::RenderWindow& window);
    void drawTooltip(sf::RenderWindow& window, sf::Vector2f mouseUI, const std::string& text);

    static sf::Color   markerColor(MapMarkerType type);
    static std::string markerLabel(MapMarkerType type);

    sf::Font     m_font;
    bool         m_open{ false };

    sf::Vector2f m_panelPos{ 0.f, 0.f };
    sf::Vector2f m_panelSize{ 0.f, 0.f };
    sf::Vector2u m_windowSize{ 1280u, 720u };

    static constexpr float MARKER_RADIUS = 6.f;
    static constexpr float HOVER_RADIUS = 10.f;
};