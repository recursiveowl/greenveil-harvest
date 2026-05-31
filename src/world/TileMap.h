#pragma once
#include "pch.h"
#include "core/AssetManager.h"

enum class TileType : uint8_t { Grass = 0, Dirt, Water, Sand, COUNT };

struct Tile {
    TileType type    = TileType::Grass;
    bool     solid   = false;
    bool     fertile = false; // blizu vode = plodno
};

class TileMap {
public:
    static constexpr int TILE_SIZE = 32;

    explicit TileMap(AssetManager& assets);
    void generate(int width, int height, unsigned seed = 42);
    void draw(sf::RenderWindow& window);
    void setTileType(int x, int y, TileType type);
    void forceWaterRegion(int centerX, int centerY, int radius);

    const Tile& getTile(int x, int y) const;
    Tile&       getTileMut(int x, int y);
    int getWidth()  const { return m_width;  }
    int getHeight() const { return m_height; }

private:
    bool isNearWater(int x, int y, int radius = 4) const;

    AssetManager&     m_assets;
    std::vector<Tile> m_tiles;
    int               m_width{0}, m_height{0};

    struct TileLayer {
        sf::VertexArray vertices;
        sf::Texture     texture;
    };
    std::array<TileLayer, static_cast<int>(TileType::COUNT)> m_layers;
};
