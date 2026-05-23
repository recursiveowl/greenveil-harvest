#include "systems/FishingSystem.h"
#include <cmath>
#include <algorithm>

static constexpr float PI = 3.14159265f;

FishingSystem::FishingSystem()
    : m_rng(std::random_device{}())
{
    m_fishDefs = {
        { ItemType::Carp,       "Carp",        8,  14, 0.00f, 3.f,  7.f, 3.0f, sf::Color(80,160,100) },
        { ItemType::Trout,      "Trout",       15, 25, 0.25f, 4.f,  9.f, 2.5f, sf::Color(60,120,200) },
        { ItemType::Salmon,     "Salmon",      28, 45, 0.50f, 5.f, 12.f, 2.0f, sf::Color(220,100, 80) },
        { ItemType::Catfish,    "Catfish",     40, 70, 0.72f, 6.f, 15.f, 1.5f, sf::Color(100, 80,140) },
        { ItemType::GoldenFish, "Golden Fish", 90,180, 0.92f, 8.f, 20.f, 1.0f, sf::Color(255,200,  0) },
    };
}

bool FishingSystem::isNearWater(sf::Vector2f pos, const TileMap& map) const {
    int cx = static_cast<int>(pos.x) / TileMap::TILE_SIZE;
    int cy = static_cast<int>(pos.y) / TileMap::TILE_SIZE;
    int r = static_cast<int>(WATER_CHECK_DIST / TileMap::TILE_SIZE) + 1;
    for (int dy = -r; dy <= r; ++dy)
        for (int dx = -r; dx <= r; ++dx)
            if (map.getTile(cx + dx, cy + dy).type == TileType::Water)
                return true;
    return false;
}

bool FishingSystem::isPosOnWater(sf::Vector2f pos, const TileMap& map) const {
    int tx = static_cast<int>(pos.x) / TileMap::TILE_SIZE;
    int ty = static_cast<int>(pos.y) / TileMap::TILE_SIZE;
    return map.getTile(tx, ty).type == TileType::Water;
}

bool FishingSystem::canFish(sf::Vector2f playerPos, const TileMap& map,
    const Inventory& inv) const {
    if (inv.selectedItem() != ItemType::FishingRod) return false;
    return isNearWater(playerPos, map);
}

void FishingSystem::selectFish(sf::Vector2f playerPos, const TileMap& map) {
    std::uniform_real_distribution<float> roll(0.f, 1.f);
    float r = roll(m_rng);
    const FishDef* selected = &m_fishDefs[0];
    for (auto& f : m_fishDefs)
        if (r >= f.rarity) selected = &f;
    m_currentFish = selected;

    std::uniform_real_distribution<float> waitDist(
        m_currentFish->biteTimeMin, m_currentFish->biteTimeMax);
    m_waitTarget = waitDist(m_rng);
    m_waitTimer = 0.f;
    m_biteWindow = m_currentFish->reelWindow;
    m_biteTimer = 0.f;
}

void FishingSystem::onActionPressed(sf::Vector2f playerPos,
    sf::Vector2f facingDir) {
    m_playerPos = playerPos;
    m_facingDir = facingDir;
    m_castOrigin = playerPos;

    switch (m_state) {

    case FishingState::Idle:
        m_state = FishingState::Casting;
        m_castTimer = CAST_DURATION;
        {
            sf::Vector2f target = playerPos + facingDir * 112.f;
            m_bobber.pos = playerPos + sf::Vector2f{ 0.f, -16.f };
            m_bobber.targetPos = target;
            m_bobber.velocity = (target - m_bobber.pos) / CAST_DURATION;
            m_bobber.velocity.y -= 160.f;
            m_bobber.inWater = false;
            m_bobber.wobbleAmp = 0.f;
            m_bobber.wobbleTimer = 0.f;
            m_bobber.splashTimer = 0.f;
        }
        break;

    case FishingState::Biting:
        m_state = FishingState::Reeling;
        m_mini = {};
        m_mini.difficulty = m_currentFish->rarity;
        m_mini.catchProgress = 0.f;
        m_mini.fishPullTimer = 1.2f;
        break;

    case FishingState::Reeling:
        if (m_mini.fishPulling && m_mini.pullPauseTimer <= 0.f) {
            float timeSinceLast = m_mini.lastSpamTime;
            float spamBonus = timeSinceLast < 0.15f ? 0.22f
                : timeSinceLast < 0.25f ? 0.16f
                : 0.10f;
            m_mini.catchProgress += spamBonus;
            m_mini.escapeProgress = std::max(0.f,
                m_mini.escapeProgress - spamBonus * 0.6f);
            m_mini.lastSpamTime = 0.f;
            m_bobber.wobbleAmp = 8.f;
        }
        break;

    case FishingState::Casting:
    case FishingState::Splashing:
    case FishingState::Waiting:
    case FishingState::Caught:
    case FishingState::Failed:
        m_state = FishingState::Idle;
        m_currentFish = nullptr;
        m_bobber.inWater = false;
        break;
    }
}

void FishingSystem::updateBobber(float dt) {
    switch (m_state) {

    case FishingState::Casting: {
        m_bobber.velocity.y += 420.f * dt;
        m_bobber.pos += m_bobber.velocity * dt;
        m_castTimer -= dt;
        if (m_castTimer <= 0.f) {
            m_bobber.pos = m_bobber.targetPos;
            m_bobber.velocity = { 0.f, 0.f };
            m_bobber.inWater = true;
            m_bobber.splashTimer = SPLASH_DURATION;
            m_state = FishingState::Splashing;
        }
        break;
    }

    case FishingState::Splashing:
        m_bobber.splashTimer -= dt;
        m_bobber.pos.y += std::sin(
            m_bobber.splashTimer * PI * 6.f) * 2.f * dt * 60.f;
        break;

    case FishingState::Waiting:
        m_bobber.wobbleTimer += dt * 1.5f;
        m_bobber.pos.y = m_bobber.targetPos.y +
            std::sin(m_bobber.wobbleTimer) * 2.5f;
        break;

    case FishingState::Biting:
        m_bobber.wobbleTimer += dt * 8.f;
        m_bobber.pos.y = m_bobber.targetPos.y +
            std::sin(m_bobber.wobbleTimer) * 5.f;
        m_bobber.pos.x = m_bobber.targetPos.x +
            std::sin(m_bobber.wobbleTimer * 1.3f) * 3.f;
        break;

    case FishingState::Reeling: {
        float targetAmp = m_mini.fishPulling ? 12.f : 3.f;
        m_bobber.wobbleAmp += (targetAmp - m_bobber.wobbleAmp) * dt * 4.f;
        m_bobber.wobbleTimer += dt * (m_mini.fishPulling ? 9.f : 4.f);

        m_bobber.pos.x = m_bobber.targetPos.x +
            std::sin(m_bobber.wobbleTimer) * m_bobber.wobbleAmp +
            std::sin(m_bobber.wobbleTimer * 2.3f) * m_bobber.wobbleAmp * 0.4f;
        m_bobber.pos.y = m_bobber.targetPos.y +
            std::sin(m_bobber.wobbleTimer * 1.7f) * m_bobber.wobbleAmp * 0.6f;

        if (m_mini.fishPulling) {
            float pullDir = std::sin(m_bobber.wobbleTimer * 0.5f);
            m_bobber.pos.x += pullDir * 6.f;
            m_bobber.pos.y += 4.f;
        }
        break;
    }

    default: break;
    }
}

void FishingSystem::updateMinigame(float dt) {
    FishingMinigame& mg = m_mini;

    mg.fishPullTimer -= dt;
    if (mg.fishPullTimer <= 0.f) {
        std::uniform_real_distribution<float> pullRoll(0.f, 1.f);
        std::uniform_real_distribution<float> pullDur(0.6f, 1.4f);
        bool willPull = pullRoll(m_rng) < 0.75f;
        if (willPull && !mg.fishPulling)
            mg.pullPauseTimer = 0.075f;
        mg.fishPulling = willPull;
        mg.fishPullTimer = pullDur(m_rng);
    }
    
    if (mg.fishPulling) {
        if (mg.pullPauseTimer > 0.f) {
            mg.pullPauseTimer -= dt;
            if (mg.pullPauseTimer < 0.f) mg.pullPauseTimer = 0.f;
        }
        else {
            float phaseMult = mg.catchProgress < 0.10f ? 0.4f : 1.1f;
            mg.escapeProgress += dt * (0.25f + mg.difficulty * 0.2f) * phaseMult;
            mg.catchProgress = std::max(0.f, mg.catchProgress - dt * 0.2f);
        }
    }
    else {
        mg.escapeProgress = std::max(0.f, mg.escapeProgress - dt * 0.2f);
    }

    mg.catchProgress = std::clamp(mg.catchProgress, 0.f, 1.f);
    mg.escapeProgress = std::clamp(mg.escapeProgress, 0.f, 1.f);

    if (mg.catchProgress >= 1.f) {
        m_state = FishingState::Caught;
        m_caughtFish = m_currentFish;
        m_hasCatch = true;
        m_resultTimer = 1.8f;
        m_bobber.wobbleAmp = 0.f;
    }
    else if (mg.escapeProgress >= 1.f) {
        m_state = FishingState::Failed;
        m_resultTimer = 1.5f;
        m_bobber.wobbleAmp = 0.f;
    }

    mg.lastSpamTime += dt;
}

void FishingSystem::update(float dt, sf::Vector2f playerPos,
    const TileMap& map) {
    m_playerPos = playerPos;

    if (m_state != FishingState::Idle)
        updateBobber(dt);

    switch (m_state) {

    case FishingState::Splashing:
        m_bobberOnWater = isPosOnWater(m_bobber.pos, map);
        m_bobber.inWater = m_bobberOnWater;
        if (m_bobber.splashTimer <= 0.f) {
            if (m_bobberOnWater) {
                m_pullMode = false;
                m_state = FishingState::Waiting;
                selectFish(playerPos, map);
            }
            else {
                m_pullMode = true;
                m_pullProgress = 0.f;
                m_state = FishingState::Waiting;
                m_waitTarget = 99.f;
            }
        }
        break;

    case FishingState::Waiting:
        if (m_pullMode) {
            m_pullProgress = std::max(0.f, m_pullProgress - dt * 0.8f);
            break;
        }
        m_waitTimer += dt;
        if (m_waitTimer >= m_waitTarget) {
            m_state = FishingState::Biting;
            m_biteTimer = 0.f;
        }
        break;

    case FishingState::Biting:
        m_biteTimer += dt;
        if (m_biteTimer >= 0.4f) {
            m_state = FishingState::Reeling;
            m_mini = {};
            m_mini.difficulty = m_currentFish->rarity;
            m_mini.fishPullTimer = 1.2f;
        }
        break;

    case FishingState::Reeling:
        updateMinigame(dt);
        break;

    case FishingState::Caught:
    case FishingState::Failed:
        m_resultTimer -= dt;
        if (m_resultTimer <= 0.f) {
            m_state = FishingState::Idle;
            m_currentFish = nullptr;
            m_bobber.inWater = false;
        }
        break;

    default: break;
    }
}

const FishDef* FishingSystem::pollCatch() {
    if (!m_hasCatch) return nullptr;
    m_hasCatch = false;
    return m_caughtFish;
}