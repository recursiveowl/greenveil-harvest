#include "entities/Player.h"
#include "world/TileMap.h"
#include "core/PlaceholderAssets.h"
#include <cmath>

static constexpr float PI = 3.14159265f;

Player::Player(AssetManager& assets)
    : m_assets(assets)
    , m_texture(PlaceholderAssets::makePlayer())
    , m_sprite(m_texture)
{
    m_sprite.setOrigin({16.f, 16.f});
}

void Player::setPosition(sf::Vector2f pos) {
    m_sprite.setPosition(pos);
}

void Player::setFacing(sf::Vector2f dir) {
    if (std::abs(dir.x) > std::abs(dir.y))
        m_facing = dir.x > 0.f ? FacingDir::Right : FacingDir::Left;
    else
        m_facing = dir.y > 0.f ? FacingDir::Down : FacingDir::Up;
}

sf::Vector2f Player::getFacing() const {
    switch (m_facing) {
        case FacingDir::Up:    return { 0.f, -1.f};
        case FacingDir::Down:  return { 0.f,  1.f};
        case FacingDir::Left:  return {-1.f,  0.f};
        case FacingDir::Right: return { 1.f,  0.f};
    }
    return {0.f, 1.f};
}

float Player::getSwingAngle() const {
    // Pocetni kut swinga — swing ide od -90 do +90 stepeni oko facing smjera
    switch (m_facing) {
        case FacingDir::Right: return 0.f;
        case FacingDir::Down:  return 90.f;
        case FacingDir::Left:  return 180.f;
        case FacingDir::Up:    return 270.f;
    }
    return 0.f;
}

void Player::update(float dt, const TileMap& map) {
    sf::Vector2f vel{0.f, 0.f};
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) vel.y -= SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) vel.y += SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) vel.x -= SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) vel.x += SPEED;

    if (vel.x != 0.f || vel.y != 0.f)
        setFacing(vel);

    if (vel.x != 0.f && vel.y != 0.f) {
        vel.x *= 0.7071f;
        vel.y *= 0.7071f;
    }

    sf::Vector2f newPos = m_sprite.getPosition() + vel * dt;
    if (!wouldCollide(newPos, map))
        m_sprite.setPosition(newPos);

    if (m_attackTimer > 0.f) m_attackTimer -= dt;
    if (m_hitFlash    > 0.f) m_hitFlash    -= dt;
    if (m_iFrames     > 0.f) m_iFrames     -= dt;

    // Swing animacija — napreduje od 0 do 1
    if (m_swingTimer > 0.f) {
        m_swingTimer -= dt;
        m_swingProgress = 1.f - (m_swingTimer / SWING_DURATION);
    } else {
        m_swingProgress = 0.f;
    }
}

bool Player::tryAttack() {
    if (m_attackTimer > 0.f) return false;
    m_attackTimer   = ATTACK_CD;
    m_swingTimer    = SWING_DURATION;
    m_swingProgress = 0.f;
    return true;
}

void Player::takeDamage(float dmg) {
    if (m_iFrames > 0.f) return;
    m_hp       = std::max(0.f, m_hp - dmg);
    m_hitFlash = 0.2f;
    m_iFrames  = IFRAME_DURATION;
}

void Player::draw(sf::RenderWindow& window) {
    m_sprite.setColor(m_hitFlash > 0.f
        ? sf::Color(255, 100, 100)
        : sf::Color::White);
    window.draw(m_sprite);

    // Swing arc — polukrug koji animira od jedne strane do druge
    if (m_swingTimer > 0.f) {
        sf::Vector2f pos     = m_sprite.getPosition();
        float        baseAng = getSwingAngle();

        // Swing ide od -100 do +100 stepeni oko facing ose
        float sweepTotal = 200.f;
        float startAng   = baseAng - 100.f;
        float currentAng = startAng + sweepTotal * m_swingProgress;

        // Nacrtaj arc kao niz trouglova (fan)
        const int   SEGMENTS = 16;
        const float INNER    = 10.f;
        const float OUTER    = ATTACK_RANGE;
        const float arcStep  = sweepTotal / SEGMENTS;

        float alpha = (1.f - m_swingProgress) * 200.f;

        sf::VertexArray arc(sf::PrimitiveType::Triangles,
                            static_cast<std::size_t>(SEGMENTS * 3));

        for (int i = 0; i < SEGMENTS; ++i) {
            float a0 = (startAng + arcStep * i)       * PI / 180.f;
            float a1 = (startAng + arcStep * (i + 1)) * PI / 180.f;

            // Samo nacrtaj segmente koji su vec "prošli" u swingovanju
            float segProgress = static_cast<float>(i) / SEGMENTS;
            if (segProgress > m_swingProgress) break;

            sf::Vector2f mid = pos + sf::Vector2f{
                std::cos((a0 + a1) * 0.5f) * OUTER * 0.6f,
                std::sin((a0 + a1) * 0.5f) * OUTER * 0.6f
            };

            // Fade — pocetak swinga blijedi, kraj je jasniji
            float segAlpha = alpha * (0.3f + 0.7f * segProgress);
            sf::Color col(255, 220, 60, static_cast<uint8_t>(
                std::min(255.f, segAlpha)));

            arc[i * 3 + 0].position = pos + sf::Vector2f{
                std::cos(a0) * INNER, std::sin(a0) * INNER};
            arc[i * 3 + 1].position = pos + sf::Vector2f{
                std::cos(a0) * OUTER, std::sin(a0) * OUTER};
            arc[i * 3 + 2].position = pos + sf::Vector2f{
                std::cos(a1) * OUTER, std::sin(a1) * OUTER};

            arc[i * 3 + 0].color = col;
            arc[i * 3 + 1].color = col;
            arc[i * 3 + 2].color = col;
        }
        window.draw(arc);
    }
}

bool Player::wouldCollide(sf::Vector2f pos, const TileMap& map) const {
    int tileX = static_cast<int>(pos.x) / TileMap::TILE_SIZE;
    int tileY = static_cast<int>(pos.y) / TileMap::TILE_SIZE;
    return map.getTile(tileX, tileY).solid;
}
