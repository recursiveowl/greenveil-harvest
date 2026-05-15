#include "systems/MissionSystem.h"
#include <cmath>
#include <algorithm>

MissionSystem::MissionSystem() {
    auto make = [](int id, int npcId,
        std::string title, std::string objective, std::string desc,
        int crops, int kills, int rewardSilver, int xp) {
            Mission m;
            m.id = id;
            m.npcId = npcId;
            m.title = std::move(title);
            m.objective = std::move(objective);
            m.description = std::move(desc);
            m.requireCrops = crops;
            m.requireKills = kills;
            m.rewardCoins = rewardSilver; // rewardCoins sada drzi silver vrijednost
            m.rewardXP = xp;
            m.status = MissionStatus::Inactive;
            return m;
        };

    // Nagrade u silver:
    // Marko  (farmer)   — 3 crops  → 25s  + 50 XP
    // Ana    (merchant) — 5 crops  → 80s  + 80 XP
    // Boris  (guard)    — 3 kills  → 120s + 100 XP
    // Elder  (explore)  — explore  → 200s + 150 XP
    m_missions.push_back(make(0, 0,
        "Hungry Farmer",
        "Collect 3 crops and return to Marko.",
        "Marko needs 3 crops.", 3, 0, 25, 50));
    m_missions.push_back(make(1, 1,
        "Trade Deal",
        "Collect 5 crops and return to Ana.",
        "Ana wants 5 crops for a trade.", 5, 0, 80, 80));
    m_missions.push_back(make(2, 2,
        "Goblin Threat",
        "Slay 3 goblins and return to Boris.",
        "Boris wants 3 goblins slain.", 0, 3, 120, 100));
    m_missions.push_back(make(3, 3,
        "Elder's Appetite",
        "Catch 3 fish and return to Elder Vid.",
        "Elder Vid wants 3 fish.", 3, 0, 200, 150));
}

bool MissionSystem::hasActiveMission() const {
    for (auto& m : m_missions)
        if (m.status == MissionStatus::Active ||
            m.status == MissionStatus::ReadyToTurn)
            return true;
    return false;
}

bool MissionSystem::activateMission(int id, int npcId) {
    if (hasActiveMission()) return false;

    for (auto& m : m_missions) {
        if (m.id != id || m.status != MissionStatus::Inactive) continue;
        m.status = MissionStatus::Active;
        m.npcId = npcId;
        m.killsAtActivation = m_totalKills;
        m.cropsAtActivation = m_currentCrops;

        if (m.id == 3)
            m.cropsAtActivation = 0;
        else
            m.cropsAtActivation = m_currentCrops;
        return true;
    }
    return false;
}

void MissionSystem::update(int totalCrops, int totalKills, int totalFish) {
    m_currentCrops = totalCrops;
    m_currentFish = totalFish;

    for (auto& m : m_missions) {
        if (m.status != MissionStatus::Active) continue;

        if (m.id == 3) {
            if (m_currentFish >= m.requireCrops)
                m.status = MissionStatus::ReadyToTurn;
            continue;
        }

        int killsSince = m_totalKills - m.killsAtActivation;
        int cropsSince = m_currentCrops - m.cropsAtActivation;

        if (cropsSince >= m.requireCrops && killsSince >= m.requireKills)
            m.status = MissionStatus::ReadyToTurn;
    }
}

void MissionSystem::updateExplore(sf::Vector2f playerPos) {
    if (!m_exploreTracking) return;
    m_exploreMin.x = std::min(m_exploreMin.x, playerPos.x);
    m_exploreMin.y = std::min(m_exploreMin.y, playerPos.y);
    m_exploreMax.x = std::max(m_exploreMax.x, playerPos.x);
    m_exploreMax.y = std::max(m_exploreMax.y, playerPos.y);
}

float MissionSystem::getExploreProgress() const {
    if (!m_exploreTracking) return 0.f;
    float w = (m_exploreMax.x - m_exploreMin.x) / 4800.f;
    float h = (m_exploreMax.y - m_exploreMin.y) / 4800.f;
    return std::min(std::min(w, h) / 800.f, 1.f);
}

int MissionSystem::getQuestKills() const {
    for (auto& m : m_missions)
        if ((m.status == MissionStatus::Active ||
            m.status == MissionStatus::ReadyToTurn) && m.requireKills > 0)
            return std::max(0, m_totalKills - m.killsAtActivation);
    return 0;
}

int MissionSystem::getQuestCrops() const {
    for (auto& m : m_missions)
        if ((m.status == MissionStatus::Active ||
            m.status == MissionStatus::ReadyToTurn) && m.requireCrops > 0)
            return (m.id == 3)
            ? std::max(0, m_currentFish)
            : std::max(0, m_currentCrops - m.cropsAtActivation);
    return 0;
}

bool MissionSystem::tryTurnIn(int missionId) {
    for (auto& m : m_missions) {
        if (m.id == missionId && m.status == MissionStatus::ReadyToTurn) {
            m.status = MissionStatus::Completed;
            m_lastRewardSilver = m.rewardCoins; // spremi nagradu da je Game moze uzeti
            m_exploreTracking = false;
            return true;
        }
    }
    return false;
}

void MissionSystem::registerKill() { ++m_totalKills; }

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