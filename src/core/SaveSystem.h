#pragma once
#include "pch.h"
#include "systems/ProgressionSystem.h"
#include "systems/Inventory.h"
#include "systems/MissionSystem.h"

struct SaveData {
    // Player
    float playerX{ 0.f }, playerY{ 0.f };
    float playerHp{ 100.f };
    float gameHour{ 8.f };
    // Progression
    int   level{ 1 };
    int   xp{ 0 };
    int   skillPoints{ 0 };
    int   skillLevels[static_cast<int>(SkillType::COUNT)]{};
    // Inventory — cijeli inventory (hotbar + backpack)
    int   itemTypes[Inventory::SLOTS]{};
    int   itemCounts[Inventory::SLOTS]{};
    // Economy
    int   coins{ 0 };      // ukupno u silver
    int   totalKills{ 0 };
    int   missionStatuses[4]{};
};

class SaveSystem {
public:
    static bool save(const std::string& path, const SaveData& data);
    static bool load(const std::string& path, SaveData& data);
};