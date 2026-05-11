#pragma once
#include "pch.h"
#include "core/AssetManager.h"
#include "core/Camera.h"
#include "core/DayNight.h"
#include "core/SaveSystem.h"
#include "world/TileMap.h"
#include "entities/Player.h"
#include "systems/FarmingSystem.h"
#include "systems/Inventory.h"
#include "systems/CombatSystem.h"
#include "systems/NPCManager.h"
#include "systems/ProgressionSystem.h"
#include "systems/MarketSystem.h"
#include "ui/HUD.h"
#include "ui/ObjectiveUI.h"
#include "ui/SkillUI.h"
#include "ui/MarketUI.h"
#include "systems/FishingSystem.h"
#include "systems/BuildingSystem.h"
#include "ui/FishingUI.h"
#include "ui/BuildingUI.h"
#include "world/CoinPickup.h";

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void spawnVillage();
    void saveGame();
    void loadGame();
    void applyProgressionStats();

    static constexpr unsigned WINDOW_W = 1280;
    static constexpr unsigned WINDOW_H = 720;
    static constexpr float    BOSS_SPAWN_HOUR = 0.f;

    sf::RenderWindow    m_window;
    sf::Clock           m_clock;
    AssetManager        m_assets;
    Camera              m_camera;
    DayNight            m_dayNight;
    TileMap             m_tileMap;
    Player              m_player;
    FarmingSystem       m_farming;
    Inventory           m_inventory;
    CombatSystem        m_combat;
    NPCManager          m_npcManager;
    ProgressionSystem   m_progression;
    MarketSystem        m_market;
    Wallet              m_wallet;
    HUD                 m_hud;
    ObjectiveUI         m_objectiveUI;
    SkillUI             m_skillUI;
    MarketUI            m_marketUI;
    FishingSystem       m_fishing;
    FishingUI           m_fishingUI;
    BuildingSystem      m_building;
    BuildingUI          m_buildingUI;
    CoinPickup          m_coinPickup;

    float               m_enemySpawnTimer{ 0.f };
    float               m_gameHour{ 8.f };
    float               m_prevHour{ 8.f };
    bool                m_gameOver{ false };
    bool                m_bossSpawnedThisNight{ false };
    unsigned            m_seed{ 0 };
};