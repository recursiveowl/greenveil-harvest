#pragma once
#include "pch.h"
#include "systems/ProgressionSystem.h"
#include "systems/Inventory.h"
#include "systems/MissionSystem.h"
#include "world/TileMap.h"

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

    int  playMinutes{ 0 };
    int  playerLevel{ 1 };

    int  houseTileX{ 0 };
    int  houseTileY{ 0 };
    bool houseHasData{ false };

    unsigned mapSeed{ 0 };

    bool hasData{ false };
};

class SaveSystem {
public:
    static bool save(const std::string& path, const SaveData& data);
    static bool load(const std::string& path, SaveData& data);
    static bool save(int slot, const SaveData& data);
    static bool saveTiles(int slot, const TileMap& map);
    static bool loadTiles(int slot, TileMap& map);
    static bool load(int slot, SaveData& data);
    static void deleteSlot(int slot);
    static std::string slotPath(int slot);
};