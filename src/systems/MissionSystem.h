#pragma once
#include "pch.h"

enum class MissionStatus : uint8_t {
    Inactive,
    Active,
    ReadyToTurn,
    Completed
};

struct Mission {
    int           id{ -1 };
    std::string   title;
    std::string   objective;
    std::string   description;
    MissionStatus status{ MissionStatus::Inactive };
    int           npcId{ -1 };
    int           requireCrops{ 0 };
    int           rewardCoins{ 0 };
    int           rewardXP{ 0 };

    // Crops zabiljezeni u trenutku aktivacije questa
    // (da ne bi prethodni crops racunali)
    int           cropsAtActivation{ 0 };

    // Explore quest — tracked area
    sf::FloatRect exploredBounds = sf::FloatRect({ 0.f, 0.f }, { 0.f, 0.f });
    bool          exploreReady{ false };
};

class MissionSystem {
public:
    MissionSystem();

    bool activateMission(int id, int npcId);
    void update(int totalCrops, int totalFish = 0);
    void updateExplore(sf::Vector2f playerPos);
    bool tryTurnIn(int missionId);

    const Mission* getMission(int id)        const;
    const Mission* getActiveMission()        const;
    bool                        isActive(int id)          const;
    bool                        isReadyToTurnIn(int id)   const;
    int                         getCurrentCrops()         const { return m_currentCrops; }
    int                         getLastRewardSilver()     const { return m_lastRewardSilver; }
    bool                        hasActiveMission()        const;

    // Za progress prikaz — crops od aktivacije questa
    int   getQuestCrops()  const;
    float getExploreProgress() const; // 0.0 - 1.0

    const std::vector<Mission>& getAllMissions() const { return m_missions; }

private:
    std::vector<Mission> m_missions;
    int                  m_currentCrops{ 0 };
    int                  m_lastRewardSilver{ 0 };
    int                  m_currentFish{ 0 };

    // Explore tracking
    sf::Vector2f         m_exploreMin{ 1e9f,  1e9f };
    sf::Vector2f         m_exploreMax{ -1e9f, -1e9f };
    bool                 m_exploreTracking{ false };
    sf::Vector2f         m_exploreStart{ 0.f, 0.f };
};