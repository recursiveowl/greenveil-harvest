#include "systems/MissionSystem.h"
#include <cmath>
#include <algorithm>

MissionSystem::MissionSystem() {
    auto make = [](int id, int npcId,
                   std::string title, std::string objective, std::string desc,
                   int crops, int kills, int coins, int xp) {
        Mission m;
        m.id           = id;
        m.npcId        = npcId;
        m.title        = std::move(title);
        m.objective    = std::move(objective);
        m.description  = std::move(desc);
        m.requireCrops = crops;
        m.requireKills = kills;
        m.rewardCoins  = coins;
        m.rewardXP     = xp;
        m.status       = MissionStatus::Inactive;
        return m;
    };

    m_missions.push_back(make(0, 0,
        "Hungry Farmer",
        "Collect 3 crops and return to Marko.",
        "Marko needs 3 crops.", 3, 0, 25, 50));
    m_missions.push_back(make(1, 1,
        "Trade Deal",
        "Collect 5 crops and return to Ana.",
        "Ana wants 5 crops for a trade.", 5, 0, 50, 80));
    m_missions.push_back(make(2, 2,
        "Goblin Threat",
        "Slay 3 goblins and return to Boris.",
        "Boris wants 3 goblins slain.", 0, 3, 40, 100));
    m_missions.push_back(make(3, 3,
        "Ancient Treasure",
        "Explore a 150x150 area far from the village.",
        "Elder Vid wants you to explore.", 0, 0, 80, 150));
}

void MissionSystem::activateMission(int id, int npcId) {
    for (auto& m : m_missions) {
        if (m.id != id || m.status != MissionStatus::Inactive) continue;
        m.status = MissionStatus::Active;
        m.npcId  = npcId;

        // Zabilježi kills/crops u trenutku aktivacije
        m.killsAtActivation = m_totalKills;
        m.cropsAtActivation = m_currentCrops;

        // Aktiviraj explore tracking za misiju 3
        if (id == 3) {
            m_exploreTracking = true;
            m_exploreMin = {1e9f,  1e9f};
            m_exploreMax = {-1e9f, -1e9f};
        }
    }
}

void MissionSystem::update(int totalCrops, int totalKills) {
    m_currentCrops = totalCrops;

    for (auto& m : m_missions) {
        if (m.status != MissionStatus::Active) continue;

        if (m.id == 3) {
            if (m_exploreTracking) {
                float w = m_exploreMax.x - m_exploreMin.x;
                float h = m_exploreMax.y - m_exploreMin.y;
                // 150 tile-ova * 32 px = 4800 px
                if (w >= 4800.f && h >= 4800.f)
                    m.status = MissionStatus::ReadyToTurn;
            }
            continue;
        }

        // Kills/crops od aktivacije questa
        int killsSince  = m_totalKills   - m.killsAtActivation;
        int cropsSince  = m_currentCrops - m.cropsAtActivation;

        bool cropsOk = (cropsSince  >= m.requireCrops);
        bool killsOk = (killsSince  >= m.requireKills);

        if (cropsOk && killsOk)
            m.status = MissionStatus::ReadyToTurn;
    }
}

void MissionSystem::updateExplore(sf::Vector2f playerPos) {
    if (!m_exploreTracking) return;

    // Proširi bounds na osnovu pozicije igraca
    m_exploreMin.x = std::min(m_exploreMin.x, playerPos.x);
    m_exploreMin.y = std::min(m_exploreMin.y, playerPos.y);
    m_exploreMax.x = std::max(m_exploreMax.x, playerPos.x);
    m_exploreMax.y = std::max(m_exploreMax.y, playerPos.y);
}

float MissionSystem::getExploreProgress() const {
    if (!m_exploreTracking) return 0.f;
    float w = (m_exploreMax.x - m_exploreMin.x) / 4800.f;
    float h = (m_exploreMax.y - m_exploreMin.y) / 4800.f;
    // Progres je minimum od width i height (mora biti i širok i visok)
    return std::min(std::min(w, h), 1.f);
}

int MissionSystem::getQuestKills() const {
    for (auto& m : m_missions)
        if (m.status == MissionStatus::Active ||
            m.status == MissionStatus::ReadyToTurn)
            if (m.requireKills > 0)
                return std::max(0, m_totalKills - m.killsAtActivation);
    return 0;
}

int MissionSystem::getQuestCrops() const {
    for (auto& m : m_missions)
        if (m.status == MissionStatus::Active ||
            m.status == MissionStatus::ReadyToTurn)
            if (m.requireCrops > 0)
                return std::max(0, m_currentCrops - m.cropsAtActivation);
    return 0;
}

bool MissionSystem::tryTurnIn(int missionId) {
    for (auto& m : m_missions) {
        if (m.id == missionId && m.status == MissionStatus::ReadyToTurn) {
            m.status          = MissionStatus::Completed;
            m_coins          += m.rewardCoins;
            m_exploreTracking = false;
            return true;
        }
    }
    return false;
}

void MissionSystem::registerKill() {
    ++m_totalKills;
}

const Mission* MissionSystem::getMission(int id) const {
    for (auto& m : m_missions)
        if (m.id == id) return &m;
    return nullptr;
}

const Mission* MissionSystem::getActiveMission() const {
    for (auto& m : m_missions)
        if (m.status == MissionStatus::Active ||
            m.status == MissionStatus::ReadyToTurn)
            return &m;
    return nullptr;
}

bool MissionSystem::isActive(int id) const {
    for (auto& m : m_missions)
        if (m.id == id && (m.status == MissionStatus::Active ||
                           m.status == MissionStatus::ReadyToTurn))
            return true;
    return false;
}

bool MissionSystem::isReadyToTurnIn(int id) const {
    for (auto& m : m_missions)
        if (m.id == id && m.status == MissionStatus::ReadyToTurn)
            return true;
    return false;
}
