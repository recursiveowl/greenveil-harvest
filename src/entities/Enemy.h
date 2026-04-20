#pragma once
#include "pch.h"
#include "world/TileMap.h"

enum class EnemyType : uint8_t {
    Slime = 0,
    Goblin,
    Troll,
    Skeleton,
    Wolf,
    Boss,
    COUNT
};

enum class EnemyState : uint8_t {
    Idle,
    Chase,
    Attack,
    Enraged, // Boss special
    Dead
};

struct Enemy {
    EnemyType    type{EnemyType::Slime};
    EnemyState   state{EnemyState::Idle};
    sf::Vector2f pos{0.f, 0.f};
    float        hp{0.f};
    float        maxHp{0.f};
    float        speed{0.f};
    float        attackRange{0.f};
    float        attackDamage{0.f};
    float        attackCooldown{0.f};
    float        attackTimer{0.f};
    float        aggroRange{0.f};
    float        deathTimer{0.f};
    float        iFrames{0.f};
    float        strengthMult{1.f};
    sf::Color    color{sf::Color::White};
    float        size{0.f};
    bool         isBoss{false};
    int          xpReward{0};
};
