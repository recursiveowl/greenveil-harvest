#pragma once
#include "pch.h"
#include "entities/Enemy.h"
#include "world/TileMap.h"

struct DamageNumber {
    sf::Vector2f pos;
    float        value;
    float        timer{1.5f};
    sf::Color    color;
};

class CombatSystem {
public:
    void spawnEnemy(sf::Vector2f pos, EnemyType type);
    void spawnWave(sf::Vector2f center, int count, EnemyType type);
    void spawnBoss(sf::Vector2f pos);

    void update(float dt, sf::Vector2f playerPos, const TileMap& map);
    void draw(sf::RenderWindow& window);

    bool  playerAttack(sf::Vector2f playerPos, sf::Vector2f facingDir,
                       float range, float damage);
    float pollPlayerDamage();
    void  setStrengthMult(float mult) { m_strengthMult = mult; }
    int   pollXPGained();
    bool  isBossAlive() const;
    int   getEnemyCount() const { return static_cast<int>(m_enemies.size()); }

private:
    void  updateEnemy(Enemy& e, float dt, sf::Vector2f playerPos,
                      const TileMap& map);
    void  addDamageNumber(sf::Vector2f pos, float val, sf::Color col);

    std::vector<Enemy>        m_enemies;
    std::vector<DamageNumber> m_dmgNumbers;
    float                     m_playerDamageAccum{0.f};
    float                     m_strengthMult{1.f};
    int                       m_xpGained{0};
    bool                      m_bossSpawned{false};

    static Enemy makeEnemy(EnemyType type, sf::Vector2f pos);
};
