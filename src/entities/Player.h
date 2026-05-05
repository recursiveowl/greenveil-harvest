#pragma once
#include "pch.h"
#include "core/AssetManager.h"

class TileMap;
class BuildingSystem;

enum class FacingDir : uint8_t { Down = 0, Up, Left, Right };
enum class WeaponType : uint8_t { Bare, Rod, Sword };

class Player {
public:
    static constexpr float SPEED = 160.f;
    static constexpr float MAX_HP = 100.f;
    static constexpr float ATTACK_RANGE = 52.f;
    static constexpr float ATTACK_DMG = 20.f;
    static constexpr float ATTACK_CD = 0.4f;
    static constexpr float IFRAME_DURATION = 0.6f;
    static constexpr float SWING_DURATION = 0.18f;

    explicit Player(AssetManager& assets);
    void setPosition(sf::Vector2f pos);
    void update(float dt, const TileMap& map, const BuildingSystem& buildings);
    void draw(sf::RenderWindow& window);
    void takeDamage(float dmg);

    void         setFacing(sf::Vector2f dir);
    void         setWeaponType(WeaponType w) { m_weapon = w; }
    FacingDir    getFacingDir()     const { return m_facing; }
    sf::Vector2f getFacing()        const;
    float        getSwingAngle()    const;
    float        getSwingProgress() const { return m_swingProgress; }

    sf::Vector2f getPosition() const { return m_sprite.getPosition(); }
    float        getHp()       const { return m_hp; }
    float        getMaxHp()    const { return MAX_HP; }
    bool         isAlive()     const { return m_hp > 0.f; }
    bool         isAttacking() const { return m_swingTimer > 0.f; }
    bool         tryAttack();

private:
    bool wouldCollide(sf::Vector2f pos, const TileMap& map) const;

    AssetManager& m_assets;
    sf::Texture   m_texture;
    sf::Sprite    m_sprite;
    float         m_hp{ MAX_HP };
    float         m_attackTimer{ 0.f };
    float         m_swingTimer{ 0.f };
    float         m_swingProgress{ 0.f };
    float         m_hitFlash{ 0.f };
    float         m_iFrames{ 0.f };
    FacingDir     m_facing{ FacingDir::Down };
    WeaponType    m_weapon{ WeaponType::Bare };
};