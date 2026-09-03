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
#include "world/CoinPickup.h"
#include "core/UIScale.h"
#include "world/InteractTile.h"
#include "ui/InteractPrompt.h"
#include "world/GrassDecor.h"
#include "world/BushSystem.h"
#include "ui/SaveUI.h"
#include "ui/MainMenu.h"
#include "ui/PauseMenu.h"
#include "world/TreeSystem.h"
#include "world/PlayerHouse.h"
#include "world/LandmarkSystem.h"
#include "core/RenderQueue.h"
#include "ui/LoadingScreen.h"
#include "world/MapSystem.h"
#include "ui/MapUI.h"
#include "world/WellSystem.h"

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
    void autosave();
    void loadGame();
    void applyProgressionStats();
    void applyLoadedData(const SaveData& sd, int slot = 0);
    void deleteSave();
    void beginLoading(std::vector<std::function<void()>> steps);
    void updateLoading(float dt);
    SaveData buildSaveData();

    static constexpr unsigned WINDOW_W = 1280;
    static constexpr unsigned WINDOW_H = 720;
    unsigned m_windowW{ WINDOW_W };
    unsigned m_windowH{ WINDOW_H };

    sf::RenderWindow    m_window;
    sf::Clock           m_clock;
    AssetManager        m_assets;
    Camera              m_camera;
    DayNight            m_dayNight;
    TileMap             m_tileMap;
    Player              m_player;
    FarmingSystem       m_farming;
    Inventory           m_inventory;
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
    InteractTileManager m_interactTiles;
    InteractPrompt      m_interactPrompt;
    GrassDecor          m_grassDecor;
    BushSystem          m_bushes;
    PauseMenu           m_pauseMenu;
    TreeSystem          m_trees;
    PlayerHouseSystem   m_playerHouse;
    LandmarkSystem      m_landmarks;
    MapSystem           m_mapSystem;
    MapUI               m_mapUI;
    WellSystem          m_well;

    float               m_gameHour{ 8.f };
    float               m_prevHour{ 8.f };
    float               m_wellFillTimer{ 0.f };
    bool                m_gameOver{ false };
    unsigned            m_seed{ 0 };

    enum class GameState { MainMenu, SaveSelect, SaveMenu, PauseMenu, Playing, HouseInterior, Loading };
    GameState   m_gameState{ GameState::MainMenu };
    enum class SaveMode { NewGame, Continue };
    SaveMode  m_saveMode{ SaveMode::Continue };
    SaveData  m_slotPreviews[3]{};
    float     m_playSeconds{ 0.f };
    MainMenu  m_mainMenu;
    SaveUI    m_saveUI;

    int m_currentSlot{ 0 };
    int m_slotBaseMinutes{ 0 };

    LoadingScreen m_loadingScreen;
    std::vector<std::function<void()>> m_loadSteps;
    size_t   m_loadStepIndex{ 0 };
    SaveData m_pendingLoadData{};

    bool m_anyUIOpen() const {
        return m_interactPrompt.isVisible() ||
            m_npcManager.isDialogueOpen();
    }

    bool m_wellFillContext() const {
        return m_well.isNear(m_player.getPosition()) &&
            m_inventory.selectedItem() == ItemType::WateringCan;
    }

    sf::Vector2f m_returnPos{ 0.f, 0.f };
    
    bool m_wasAtDoorTrigger{ false };
    bool m_fKeyHeld{ false };
    bool m_eKeyHeld{ false };
    bool m_rKeyHeld{ false };
    bool m_bKeyHeld{ false };
    bool m_mKeyHeld{ false };
    bool m_nKeyHeld{ false };
    bool m_cKeyHeld{ false };
    bool m_tabKeyHeld{ false };
    bool m_spaceKeyHeld{ false };
};