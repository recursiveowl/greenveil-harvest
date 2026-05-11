#include "world/CoinPickup.h"
#include <cmath>
#include <random>

static constexpr float PI = 3.14159265f;

static float dist(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

CoinPickup::CoinPickup(AssetManager& assets)
    : m_tex(assets.getTexture("assets/textures/silver_coins.png"))
{
}

void CoinPickup::spawnCoins(sf::Vector2f center, int count,
    unsigned seed, const TileMap& map) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> ang(0.f, 2.f * PI);
    std::uniform_real_distribution<float> near(600.f, 1500.f);
    std::uniform_real_distribution<float> far(1500.f, 4000.f);
    std::uniform_real_distribution<float> roll(0.f, 1.f);
    
    float r = roll(rng) < 0.2f ? near(rng) : far(rng);

    int placed = 0, attempts = 0;
    while (placed < count && attempts < count * 20) {
        ++attempts;
        float a = ang(rng);
        float r = roll(rng) < 0.7f ? near(rng) : far(rng);
        sf::Vector2f pos = { center.x + std::cos(a) * r,
                             center.y + std::sin(a) * r };

        int tx = static_cast<int>(pos.x) / TileMap::TILE_SIZE;
        int ty = static_cast<int>(pos.y) / TileMap::TILE_SIZE;
        const Tile& t = map.getTile(tx, ty);

        if (t.solid || t.type == TileType::Water) continue;

        Coin c;
        c.pos = pos;
        c.value = (placed % 13 == 0) ? 10 : (placed % 7 == 0) ? 5 : 1;
        m_coins.push_back(c);
        ++placed;
    }
}

void CoinPickup::update(sf::Vector2f playerPos) {
    for (auto& c : m_coins) {
        if (c.collected) continue;
        if (dist(c.pos, playerPos) < PICKUP_RANGE) {
            c.collected = true;
            m_accum += c.value;
        }
    }
}

int CoinPickup::pollCollected() {
    int v = m_accum;
    m_accum = 0;
    return v;
}

void CoinPickup::draw(sf::RenderWindow& window) {
    for (auto& c : m_coins) {
        if (c.collected) continue;

        // Krug pozadina
        sf::CircleShape coin(18.f);
        coin.setOrigin(sf::Vector2f(18.f, 18.f));
        coin.setPosition(sf::Vector2f(c.pos));
        coin.setFillColor(sf::Color::Transparent);
        coin.setOutlineColor(sf::Color::Transparent);
        window.draw(coin);

        // Slika unutar kruga
        sf::Sprite sprite(m_tex);
        float texW = static_cast<float>(m_tex.getSize().x);
        float texH = static_cast<float>(m_tex.getSize().y);
        float diameter = 28.f;
        float scaleX = diameter / texW;
        float scaleY = diameter / texH;
        sprite.setScale(sf::Vector2f(scaleX, scaleY));
        sprite.setOrigin(sf::Vector2f(texW * 0.5f, texH * 0.5f));
        sprite.setPosition(sf::Vector2f(c.pos));
        window.draw(sprite);
    }
}