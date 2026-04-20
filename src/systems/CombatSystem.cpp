#include "systems/CombatSystem.h"
#include <cmath>
#include <algorithm>

static constexpr float PI = 3.14159265f;

static float length(sf::Vector2f v) {
    return std::sqrt(v.x*v.x + v.y*v.y);
}
static sf::Vector2f normalize(sf::Vector2f v) {
    float l = length(v);
    if (l < 0.0001f) return {0.f, 0.f};
    return {v.x/l, v.y/l};
}
static float dist(sf::Vector2f a, sf::Vector2f b) {
    return length(b - a);
}
static float angleDeg(sf::Vector2f v) {
    float a = std::atan2(v.y, v.x) * 180.f / PI;
    if (a < 0.f) a += 360.f;
    return a;
}
static bool angleInArc(float a, float center, float half) {
    float diff = a - center;
    while (diff >  180.f) diff -= 360.f;
    while (diff < -180.f) diff += 360.f;
    return std::abs(diff) <= half;
}

Enemy CombatSystem::makeEnemy(EnemyType type, sf::Vector2f pos) {
    Enemy e;
    e.type = type;
    e.pos  = pos;
    switch (type) {
        case EnemyType::Slime:
            e.hp = e.maxHp   = 30.f;
            e.speed          = 55.f;
            e.aggroRange     = 160.f;
            e.attackRange    = 28.f;
            e.attackDamage   = 5.f;
            e.attackCooldown = 1.5f;
            e.color          = sf::Color(80, 200, 80);
            e.size           = 12.f;
            e.xpReward       = 10;
            break;
        case EnemyType::Goblin:
            e.hp = e.maxHp   = 60.f;
            e.speed          = 85.f;
            e.aggroRange     = 220.f;
            e.attackRange    = 32.f;
            e.attackDamage   = 12.f;
            e.attackCooldown = 0.8f;
            e.color          = sf::Color(180, 120, 40);
            e.size           = 14.f;
            e.xpReward       = 20;
            break;
        case EnemyType::Troll:
            e.hp = e.maxHp   = 200.f;
            e.speed          = 35.f;
            e.aggroRange     = 140.f;
            e.attackRange    = 45.f;
            e.attackDamage   = 30.f;
            e.attackCooldown = 2.5f;
            e.color          = sf::Color(100, 80, 160);
            e.size           = 22.f;
            e.xpReward       = 50;
            break;
        case EnemyType::Skeleton:
            e.hp = e.maxHp   = 50.f;
            e.speed          = 70.f;
            e.aggroRange     = 200.f;
            e.attackRange    = 30.f;
            e.attackDamage   = 10.f;
            e.attackCooldown = 1.0f;
            e.color          = sf::Color(220, 220, 200);
            e.size           = 13.f;
            e.xpReward       = 18;
            break;
        case EnemyType::Wolf:
            e.hp = e.maxHp   = 45.f;
            e.speed          = 120.f;
            e.aggroRange     = 250.f;
            e.attackRange    = 25.f;
            e.attackDamage   = 8.f;
            e.attackCooldown = 0.6f;
            e.color          = sf::Color(140, 100, 80);
            e.size           = 11.f;
            e.xpReward       = 15;
            break;
        case EnemyType::Boss:
            e.hp = e.maxHp   = 800.f;
            e.speed          = 60.f;
            e.aggroRange     = 400.f;
            e.attackRange    = 60.f;
            e.attackDamage   = 40.f;
            e.attackCooldown = 1.8f;
            e.color          = sf::Color(180, 0, 0);
            e.size           = 36.f;
            e.isBoss         = true;
            e.xpReward       = 500;
            break;
        default: break;
    }
    e.attackTimer = static_cast<float>(std::rand() % 100) /
                    100.f * e.attackCooldown;
    return e;
}

void CombatSystem::spawnEnemy(sf::Vector2f pos, EnemyType type) {
    m_enemies.push_back(makeEnemy(type, pos));
}

void CombatSystem::spawnWave(sf::Vector2f center, int count, EnemyType type) {
    for (int i = 0; i < count; ++i) {
        float angle  = (PI * 2.f / count) * i;
        float radius = 120.f + (std::rand() % 80);
        m_enemies.push_back(makeEnemy(type, {
            center.x + std::cos(angle) * radius,
            center.y + std::sin(angle) * radius
        }));
    }
}

void CombatSystem::spawnBoss(sf::Vector2f pos) {
    if (m_bossSpawned) return;
    m_bossSpawned = true;
    m_enemies.push_back(makeEnemy(EnemyType::Boss, pos));
}

bool CombatSystem::isBossAlive() const {
    for (auto& e : m_enemies)
        if (e.isBoss && e.state != EnemyState::Dead)
            return true;
    return false;
}

void CombatSystem::updateEnemy(Enemy& e, float dt,
                                sf::Vector2f playerPos,
                                const TileMap& map) {
    if (e.state == EnemyState::Dead) {
        e.deathTimer -= dt;
        return;
    }
    if (e.iFrames > 0.f) e.iFrames -= dt;

    float d = dist(e.pos, playerPos);

    // Boss enrage sotto il 30% HP
    if (e.isBoss && e.hp / e.maxHp < 0.3f)
        e.state = EnemyState::Enraged;
    else if (d < e.attackRange) e.state = EnemyState::Attack;
    else if (d < e.aggroRange)  e.state = EnemyState::Chase;
    else                        e.state = EnemyState::Idle;

    float spd = e.speed;
    float dmg = e.attackDamage * m_strengthMult;
    if (e.state == EnemyState::Enraged) {
        spd *= 1.5f;
        dmg *= 1.5f;
    }

    if (e.state == EnemyState::Chase ||
        e.state == EnemyState::Enraged) {
        sf::Vector2f dir    = normalize(playerPos - e.pos);
        sf::Vector2f newPos = e.pos + dir * spd * dt;
        int tx = static_cast<int>(newPos.x) / TileMap::TILE_SIZE;
        int ty = static_cast<int>(newPos.y) / TileMap::TILE_SIZE;
        if (!map.getTile(tx, ty).solid)
            e.pos = newPos;
    }

    if (e.state == EnemyState::Attack ||
        e.state == EnemyState::Enraged) {
        if (d < e.attackRange) {
            e.attackTimer -= dt;
            if (e.attackTimer <= 0.f) {
                e.attackTimer = e.attackCooldown;
                m_playerDamageAccum += dmg;
                addDamageNumber(playerPos - sf::Vector2f{0.f, 24.f},
                                dmg, sf::Color(255, 80, 80));
            }
        }
    } else {
        if (e.attackTimer <= 0.f)
            e.attackTimer = e.attackCooldown * 0.5f;
    }
}

void CombatSystem::update(float dt, sf::Vector2f playerPos,
                           const TileMap& map) {
    for (auto& e : m_enemies)
        updateEnemy(e, dt, playerPos, map);

    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const Enemy& e) {
                return e.state == EnemyState::Dead && e.deathTimer <= 0.f;
            }),
        m_enemies.end());

    for (auto& dn : m_dmgNumbers) {
        dn.timer -= dt;
        dn.pos.y -= 30.f * dt;
    }
    m_dmgNumbers.erase(
        std::remove_if(m_dmgNumbers.begin(), m_dmgNumbers.end(),
            [](const DamageNumber& d) { return d.timer <= 0.f; }),
        m_dmgNumbers.end());
}

bool CombatSystem::playerAttack(sf::Vector2f playerPos,
                                 sf::Vector2f facingDir,
                                 float range, float damage) {
    bool  hit       = false;
    float centerAng = angleDeg(facingDir);
    float halfArc   = 100.f;

    for (auto& e : m_enemies) {
        if (e.state == EnemyState::Dead) continue;
        if (e.iFrames > 0.f) continue;
        float d = dist(e.pos, playerPos);
        if (d > range + e.size) continue;
        sf::Vector2f toEnemy = e.pos - playerPos;
        if (!angleInArc(angleDeg(toEnemy), centerAng, halfArc)) continue;

        e.hp     -= damage;
        e.iFrames = 0.35f;
        addDamageNumber(e.pos - sf::Vector2f{0.f, 16.f},
                        damage, sf::Color(255, 220, 0));
        if (e.hp <= 0.f) {
            e.state      = EnemyState::Dead;
            e.deathTimer = 0.5f;
            m_xpGained  += e.xpReward;
        }
        hit = true;
    }
    return hit;
}

float CombatSystem::pollPlayerDamage() {
    float dmg           = m_playerDamageAccum;
    m_playerDamageAccum = 0.f;
    return dmg;
}

int CombatSystem::pollXPGained() {
    int xp       = m_xpGained;
    m_xpGained   = 0;
    return xp;
}

void CombatSystem::addDamageNumber(sf::Vector2f pos, float val,
                                    sf::Color col) {
    m_dmgNumbers.push_back({pos, val, 1.2f, col});
}

void CombatSystem::draw(sf::RenderWindow& window) {
    for (const auto& e : m_enemies) {
        if (e.state == EnemyState::Dead) continue;

        sf::CircleShape body(e.size);
        body.setOrigin({e.size, e.size});
        body.setPosition(e.pos);

        sf::Color col = e.color;
        if      (e.state == EnemyState::Enraged) col = sf::Color(255, 40, 40);
        else if (e.iFrames > 0.f)                col = sf::Color(255, 255, 255);
        else if (e.hp / e.maxHp < 0.3f)          col = sf::Color(255, 80, 80);
        body.setFillColor(col);
        body.setOutlineThickness(e.isBoss ? 3.f : 2.f);
        body.setOutlineColor(e.isBoss
            ? sf::Color(255, 100, 0, 200)
            : sf::Color(0, 0, 0, 160));
        window.draw(body);

        // HP bar
        float barW  = e.isBoss ? e.size * 4.f : e.size * 2.5f;
        float barH  = e.isBoss ? 8.f : 4.f;
        float ratio = std::max(0.f, e.hp / e.maxHp);

        sf::RectangleShape barBg({barW, barH});
        barBg.setPosition({e.pos.x - barW*0.5f, e.pos.y - e.size - 14.f});
        barBg.setFillColor(sf::Color(60, 0, 0));
        window.draw(barBg);

        sf::RectangleShape barFg({barW * ratio, barH});
        barFg.setPosition({e.pos.x - barW*0.5f, e.pos.y - e.size - 14.f});
        barFg.setFillColor(e.isBoss
            ? sf::Color(220, 60, 0) : sf::Color(200, 50, 50));
        window.draw(barFg);

        // Boss label
        if (e.isBoss) {
            sf::RectangleShape dot({6.f, 6.f});
            dot.setFillColor(sf::Color(255, 100, 0));
            dot.setPosition({e.pos.x - 3.f, e.pos.y - e.size - 26.f});
            window.draw(dot);
        }
    }

    // Damage numbers
    for (const auto& dn : m_dmgNumbers) {
        float alpha = std::min(1.f, dn.timer) * 255.f;
        sf::RectangleShape dot({5.f, 5.f});
        dot.setFillColor(sf::Color(dn.color.r, dn.color.g, dn.color.b,
            static_cast<uint8_t>(alpha)));
        dot.setPosition(dn.pos);
        window.draw(dot);
    }
}
