#pragma once
#include "pch.h"
#include "systems/Inventory.h"
#include "world/TileMap.h"

struct FishDef {
    ItemType    type;
    std::string name;
    int         minSellPrice;
    int         maxSellPrice;
    float       rarity;
    float       biteTimeMin;
    float       biteTimeMax;
    float       reelWindow;
    sf::Color   color;
};

enum class FishingState : uint8_t {
    Idle,
    Casting,     // animacija bacanja
    Splashing,   // udica slece u vodu
    Waiting,
    Biting,
    Reeling,
    Caught,
    Failed
};

// ========================= BOBBER ANIM =========================

struct BobberAnim {
    sf::Vector2f pos;           // trenutna pozicija
    sf::Vector2f targetPos;     // gdje treba biti
    sf::Vector2f velocity;      // za fiziku bacanja
    float        wobbleTimer{ 0.f };  // lijevo-desno kad se riba otima
    float        wobbleAmp{ 0.f };    // amplituda wobble
    float        splashTimer{ 0.f };  // splash animacija pri slijetanju
    bool         inWater{ false };
};

// ========================= MINIGAME =========================

struct FishingMinigame {
    float barPos{ 0.5f };
    float barVelocity{ 0.f };
    float fishPos{ 0.5f };
    float fishTarget{ 0.5f };
    float fishChangeCd{ 0.f };
    float catchProgress{ 0.f };
    float escapeProgress{ 0.f };
    float difficulty{ 0.5f };
    float fishPullTimer{ 0.f };   // koliko dugo riba vuce
    bool  fishPulling{ false };   // riba aktivno vuce
    bool  playerInZone{ false };
};

// ========================= FISHING SYSTEM =========================

class FishingSystem {
public:
    FishingSystem();

    void update(float dt, sf::Vector2f playerPos, const TileMap& map);

    bool canFish(sf::Vector2f playerPos, const TileMap& map,
        const Inventory& inv) const;

    // F key — cast / react to bite / spam during reel
    void onActionPressed(sf::Vector2f playerPos, sf::Vector2f facingDir);

    FishingState           getState()    const { return m_state; }
    bool                   isActive()    const { return m_state != FishingState::Idle; }
    const FishDef* pollCatch();
    const FishingMinigame& getMinigame() const { return m_mini; }
    const BobberAnim& getBobber()   const { return m_bobber; }
    float getBiteTimer()   const { return m_biteTimer; }
    float getBiteWindow()  const { return m_biteWindow; }
    float getWaitTimer()   const { return m_waitTimer; }
    float getWaitTarget()  const { return m_waitTarget; }
    sf::Vector2f getCastOrigin()    const { return m_castOrigin; }
    bool         isBobberInWater()  const { return m_bobber.inWater && m_bobberOnWater; }
    bool         isPullMode()       const { return m_pullMode; }
    float        getPullProgress()  const { return m_pullProgress; }

private:
    void selectFish(sf::Vector2f playerPos, const TileMap& map);
    bool isNearWater(sf::Vector2f pos, const TileMap& map) const;
    bool isPosOnWater(sf::Vector2f pos, const TileMap& map) const;
    void updateBobber(float dt);
    void updateMinigame(float dt);

    FishingState    m_state{ FishingState::Idle };
    FishingMinigame m_mini;
    BobberAnim      m_bobber;

    float m_waitTimer{ 0.f };
    float m_waitTarget{ 0.f };
    float m_biteTimer{ 0.f };
    float m_biteWindow{ 0.f };
    float m_castTimer{ 0.f };
    float m_splashTimer{ 0.f };
    float m_resultTimer{ 0.f };

    const FishDef* m_currentFish{ nullptr };
    const FishDef* m_caughtFish{ nullptr };
    bool           m_hasCatch{ false };

    sf::Vector2f m_castOrigin;
    sf::Vector2f m_playerPos;
    sf::Vector2f m_facingDir;

    std::vector<FishDef> m_fishDefs;
    std::mt19937         m_rng;

    bool  m_bobberOnWater{ false }; // je li bobber stvarno u vodi
    bool  m_pullMode{ false };      // rod na kopnu = pull mod
    float m_pullProgress{ 0.f };   // napredak privlacenja

    static constexpr float WATER_CHECK_DIST = 80.f;
    static constexpr float CAST_DURATION = 0.5f;
    static constexpr float SPLASH_DURATION = 0.4f;
};