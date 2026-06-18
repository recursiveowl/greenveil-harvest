#include "entities/Player.h"
#include "world/TileMap.h"
#include "systems/BuildingSystem.h"
#include "core/PlaceholderAssets.h"
#include "world/BushSystem.h"
#include <cmath>

static constexpr float PI = 3.14159265f;

Player::Player(AssetManager& assets)
    : m_assets(assets)
    , m_texture(PlaceholderAssets::makePlayer())
    , m_sprite(m_texture)
{
    m_sprite.setOrigin({ 16.f, 16.f });
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
    case FacingDir::Up:    return { 0.f, -1.f };
    case FacingDir::Down:  return { 0.f,  1.f };
    case FacingDir::Left:  return { -1.f,  0.f };
    case FacingDir::Right: return { 1.f,  0.f };
    }
    return { 0.f, 1.f };
}

float Player::getSwingAngle() const {
    switch (m_facing) {
    case FacingDir::Right: return 0.f;
    case FacingDir::Down:  return 90.f;
    case FacingDir::Left:  return 180.f;
    case FacingDir::Up:    return 270.f;
    }
    return 0.f;
}

void Player::update(float dt, const TileMap& map, const BuildingSystem& buildings, const BushSystem& bushes, const TreeSystem& trees) {
    float speed = SPEED;
    if (bushes.isInBush(m_sprite.getPosition()))
        speed *= 0.45f;
    sf::Vector2f vel{ 0.f, 0.f };
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) vel.y -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) vel.y += speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) vel.x -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) vel.x += speed;

    if (m_jumping) {
        float g = m_jumpVelocity > 0.f ? 1800.f : 2400.f;
        m_jumpVelocity -= g * dt;
        m_jumpHeight += m_jumpVelocity * dt;
        if (m_jumpHeight <= 0.f) {
            m_jumpHeight = 0.f;
            m_jumpVelocity = 0.f;
            m_jumping = false;
        }
    }

    if (vel.x != 0.f || vel.y != 0.f)
        setFacing(vel);

    if (vel.x != 0.f && vel.y != 0.f) {
        vel.x *= 0.7071f;
        vel.y *= 0.7071f;
    }

    sf::Vector2f cur = m_sprite.getPosition();
    sf::Vector2f newPos = cur + vel * dt;

    // Tile kolizija
    if (wouldCollide(newPos, map)) newPos = cur;

    // Building kolizija — X i Y odvojeno za sliding
    sf::Vector2f tryX = { newPos.x, cur.y };
    sf::Vector2f tryY = { cur.x,    newPos.y };

    bool blockX = buildings.isSolid(tryX) || trees.isSolid(tryX);
    bool blockY = buildings.isSolid(tryY) || trees.isSolid(tryY);

    if (!blockX && !blockY)
        m_sprite.setPosition(newPos);
    else if (!blockX)
        m_sprite.setPosition(tryX);
    else if (!blockY)
        m_sprite.setPosition(tryY);
    // else — potpuno blokiran, ne pomjera se

    if (m_attackTimer > 0.f) m_attackTimer -= dt;
    if (m_hitFlash > 0.f) m_hitFlash -= dt;
    if (m_iFrames > 0.f) m_iFrames -= dt;

    if (m_swingTimer > 0.f) {
        m_swingTimer -= dt;
        m_swingProgress = 1.f - (m_swingTimer / SWING_DURATION);
    }
    else {
        m_swingProgress = 0.f;
    }
}

bool Player::tryAttack() {
    if (m_attackTimer > 0.f) return false;
    m_attackTimer = ATTACK_CD;
    m_swingTimer = SWING_DURATION;
    m_swingProgress = 0.f;
    return true;
}

void Player::takeDamage(float dmg) {
    if (m_iFrames > 0.f) return;
    m_hp -= dmg;
    if (m_hp <= 0.f) {
        --m_lives;
        if (m_lives > 0) {
            m_hp = MAX_HP;
            m_iFrames = RESPAWN_IFRAMES;
        }
        else {
            m_hp = 0.f;
        }
    }
    else {
        m_iFrames = IFRAME_DURATION;
    }
}

void Player::draw(sf::RenderWindow& window) {
    m_sprite.setColor(m_hitFlash > 0.f
        ? sf::Color(255, 100, 100)
        : sf::Color::White);

    sf::Vector2f originalPos = m_sprite.getPosition();
    m_sprite.setPosition({ originalPos.x, originalPos.y - m_jumpHeight });
    window.draw(m_sprite);
    m_sprite.setPosition(originalPos);

    if (m_swingTimer <= 0.f) return;

    sf::Vector2f pos = sf::Vector2f{ originalPos.x, originalPos.y - m_jumpHeight };
    float        baseAng = getSwingAngle();
    float        sweepTotal = 200.f;
    float        startAng = baseAng - 100.f;

    // Velicina i boja po oružju
    float  outerRange;
    sf::Color arcBaseCol;
    switch (m_weapon) {
    case WeaponType::Sword:
        outerRange = ATTACK_RANGE;           // puni
        arcBaseCol = sf::Color(255, 220, 60); // žuta
        break;
    case WeaponType::Rod:
        outerRange = ATTACK_RANGE * 0.65f;   // malo duži od bare
        arcBaseCol = sf::Color(140, 160, 200); // siva-plava
        break;
    case WeaponType::Bare:
    default:
        outerRange = ATTACK_RANGE * 0.63f;   // pola od 1.26
        arcBaseCol = sf::Color(160, 160, 175); // svijetlo siva, vidljiva
        break;
    }

    const float INNER = 8.f;
    const int   SEGMENTS = 16;
    const float arcStep = sweepTotal / SEGMENTS;
    float       alpha = (1.f - m_swingProgress) * 200.f;

    sf::VertexArray arc(sf::PrimitiveType::Triangles,
        static_cast<std::size_t>(SEGMENTS * 3));

    for (int i = 0; i < SEGMENTS; ++i) {
        float segProgress = static_cast<float>(i) / SEGMENTS;
        if (segProgress > m_swingProgress) break;

        float a0 = (startAng + arcStep * i) * PI / 180.f;
        float a1 = (startAng + arcStep * (i + 1)) * PI / 180.f;

        float segAlpha = alpha * (0.3f + 0.7f * segProgress);
        sf::Color col(arcBaseCol.r, arcBaseCol.g, arcBaseCol.b,
            static_cast<uint8_t>(std::min(255.f, segAlpha)));

        arc[i * 3 + 0].position = pos + sf::Vector2f{
            std::cos(a0) * INNER, std::sin(a0) * INNER };
        arc[i * 3 + 1].position = pos + sf::Vector2f{
            std::cos(a0) * outerRange, std::sin(a0) * outerRange };
        arc[i * 3 + 2].position = pos + sf::Vector2f{
            std::cos(a1) * outerRange, std::sin(a1) * outerRange };

        arc[i * 3 + 0].color = col;
        arc[i * 3 + 1].color = col;
        arc[i * 3 + 2].color = col;
    }
    window.draw(arc);
}

bool Player::wouldCollide(sf::Vector2f pos, const TileMap& map) const {
    int tileX = static_cast<int>(pos.x) / TileMap::TILE_SIZE;
    int tileY = static_cast<int>(pos.y) / TileMap::TILE_SIZE;
    return map.getTile(tileX, tileY).solid;
}