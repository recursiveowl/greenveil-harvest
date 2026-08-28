#pragma once
#include "pch.h"
#include <vector>
#include <unordered_set>
#include <memory>

class TileMap;
class TreeSystem;
class BushSystem;
class LandmarkSystem;

enum class MapMarkerType { Market, ForSale, NPCRanch, PlayerHouse };

struct MapMarker {
    sf::Vector2f  worldPos;
    MapMarkerType type;
};

class MapSystem {
public:
    static constexpr float PIXELS_PER_TILE = 4.f;

    // Peče cijeli svijet + landmark markere u internu RenderTexture.
    // Poziva se JEDNOM po generaciji svijeta.
    void bake(const TileMap& map, const TreeSystem& trees,
        const BushSystem& bushes, const LandmarkSystem& landmarks);

    bool                isBaked() const { return m_baked; }
    const sf::Texture& getTexture() const;
    sf::Vector2u        getTextureSize() const { return m_texSize; }

    const std::vector<MapMarker>& getMarkers() const { return m_markers; }

private:
    std::unique_ptr<sf::RenderTexture> m_renderTex;
    sf::Vector2u                       m_texSize{ 0u, 0u };
    bool                                m_baked{ false };

    std::vector<MapMarker>  m_markers;
    std::unordered_set<int> m_roadSet; // key = y*10000+x, za vizuelni overlay puteva
};