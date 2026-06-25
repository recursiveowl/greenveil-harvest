#pragma once
#include "pch.h"
#include "core/AssetManager.h"
#include "world/BushSystem.h"
#include "world/TreeSystem.h"

class TileMap;
class BuildingSystem;

enum class FacingDir : uint8_t { Down = 0, Up, Left, Right };
enum class WeaponType : uint8_t { Bare, Rod };

class Player {
public:
    static constexpr float SPEED = 160.f;
    static constexpr float MAX_HP = 100.f;
    static constexpr float IFRAME_DURATION = 0.6f;
    static constexpr float JUMP_DURATION = 0.45f;
    static constexpr float JUMP_PEAK = 38.f;
    static constexpr float GRAVITY = 1800.f;
    static constexpr float JUMP_FORCE = 380.f;
    static constexpr float RESPAWN_IFRAMES = 2.5f;

    static constexpr int MAX_LIVES = 5;

    explicit Player(AssetManager& assets);
    void setPosition(sf::Vector2f pos);
    void update(float dt, const TileMap& map, const BuildingSystem& buildings, const BushSystem& bushes, const TreeSystem& trees);
    void draw(sf::RenderWindow& window);
    void takeDamage(float dmg);

    void         setFacing(sf::Vector2f dir);
    void         setWeaponType(WeaponType w) { m_weapon = w; }
    void         startJump() { if (!m_jumping) { std::uniform_real_distribution<float> var(-80.f, 80.f); m_jumping = true; m_jumpVelocity = JUMP_FORCE; m_jumpHeight = 0.f; } }
    FacingDir    getFacingDir()     const { return m_facing; }
    sf::Vector2f getFacing()        const;

    sf::Vector2f getPosition() const { return m_sprite.getPosition(); }
    float        getHp()       const { return m_hp; }
    float        getMaxHp()    const { return MAX_HP; }
    bool         isAlive()     const { return m_hp > 0.f; }
    bool         isJumping()   const { return m_jumping; }

    int getLives() const { return m_lives; }

private:
    bool wouldCollide(sf::Vector2f pos, const TileMap& map) const;

    AssetManager& m_assets;
    sf::Texture   m_texture;
    sf::Sprite    m_sprite;
    float         m_hp{ MAX_HP };
    float         m_hitFlash{ 0.f };
    float         m_iFrames{ 0.f };
    float         m_jumpHeight{ 0.f };
    float         m_jumpTimer{ 0.f };
    float         m_jumpVelocity{ 0.f };

    int           m_lives{ MAX_LIVES };

    bool          m_jumping{ false };

    std::mt19937 m_rng{ std::random_device{}() };

    FacingDir     m_facing{ FacingDir::Down };
    WeaponType    m_weapon{ WeaponType::Bare };
};