#include "core/Game.h"
#include <ctime>
#include <cstdlib>
#include <cmath>
#ifdef _WIN32
#include <windows.h>
#endif

static constexpr float FIXED_DT = 1.f / 60.f;
static constexpr float GAME_HOUR_SPEED_DAY = 1.0f / 60.f;
static constexpr float GAME_HOUR_SPEED_NIGHT = 1.8f / 60.f;

Game::Game()
    : m_window(sf::VideoMode({ WINDOW_W, WINDOW_H }), "Greenveil Harvest", sf::Style::Default)
    , m_camera(WINDOW_W, WINDOW_H)
    , m_tileMap(m_assets)
    , m_player(m_assets)
    , m_coinPickup(m_assets)
{
    UI::init(m_window.getSize().x, m_window.getSize().y);
    m_window.setVerticalSyncEnabled(true);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    m_wallet.gold = 2;
    m_wallet.silver = 50;

#ifdef _WIN32
    HICON hIcon = (HICON)LoadImage(NULL,
        "assets/icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if (hIcon) {
        HWND hwnd = m_window.getNativeHandle();
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    }
#endif
    m_seed = static_cast<unsigned>(std::time(nullptr));
    m_tileMap.generate(256, 256, m_seed);
    m_landmarks.assignRoles(m_seed, 3);
    {
        std::vector<sf::Vector2i> entries;
        for (const auto& slot : m_landmarks.getSlots())
            entries.push_back({
                slot.tilePos.x + LandmarkSystem::SLOT_WIDTH / 2,
                slot.tilePos.y + LandmarkSystem::SLOT_HEIGHT });
        sf::Vector2i mCenter = {
            m_landmarks.getMarketPos().x + m_landmarks.getMarketSize().x / 2,
            m_landmarks.getMarketPos().y + m_landmarks.getMarketSize().y / 2 };
        m_tileMap.connectLandmarks(entries, mCenter, m_seed);
    }
    {
        sf::Vector2i slotPos = m_landmarks.getPlayerSlotPos();
        m_playerHouse.setTilePos(slotPos);
        m_player.setPosition({
            static_cast<float>((slotPos.x + LandmarkSystem::SLOT_WIDTH / 2) * TileMap::TILE_SIZE),
            static_cast<float>((slotPos.y + LandmarkSystem::SLOT_HEIGHT + 2) * TileMap::TILE_SIZE) });
    }

    m_bushes.generate(m_tileMap, m_seed, m_playerHouse.getOccupiedTileSet());

    std::unordered_set<int> treeBlocked = m_bushes.getTileSet();
    auto houseSet = m_playerHouse.getOccupiedTileSet();
    treeBlocked.insert(houseSet.begin(), houseSet.end());
    m_trees.generate(m_tileMap, m_seed, treeBlocked);

    m_grassDecor.generate(m_tileMap, m_seed, m_trees.getTileSet());
    m_mapSystem.bake(m_tileMap, m_trees, m_bushes, m_landmarks);
    m_camera.snapTo(m_player.getPosition());

#ifdef _WIN32
    HWND hwnd = m_window.getNativeHandle();
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~WS_SIZEBOX;
    style |= WS_MAXIMIZEBOX;
    SetWindowLong(hwnd, GWL_STYLE, style);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#endif
}

void Game::applyProgressionStats() {}

SaveData Game::buildSaveData() {
    SaveData sd;
    sd.hasData = true;
    sd.mapSeed = m_seed;
    sd.playerLevel = m_progression.getLevel();
    sd.playMinutes = m_slotBaseMinutes + static_cast<int>(m_playSeconds / 60.f);
    sd.playerX = m_player.getPosition().x;
    sd.playerY = m_player.getPosition().y;
    sd.playerHp = m_player.getHp();
    sd.gameHour = m_gameHour;
    sd.houseHasData = m_playerHouse.hasHouse();
    sd.houseTileX = m_playerHouse.getTilePos().x;
    sd.houseTileY = m_playerHouse.getTilePos().y;
    sd.level = m_progression.getLevel();
    sd.xp = m_progression.getXP();
    sd.skillPoints = m_progression.getSkillPoints();
    sd.coins = m_wallet.toSilver();
    const auto& skills = m_progression.getSkills();
    for (int i = 0; i < static_cast<int>(SkillType::COUNT); ++i)
        sd.skillLevels[i] = skills[i].level;
    const auto& slots = m_inventory.slots;
    for (int i = 0; i < Inventory::SLOTS; ++i) {
        sd.itemTypes[i] = static_cast<int>(slots[i].type);
        sd.itemCounts[i] = slots[i].count;
    }
    return sd;
}

void Game::beginLoading(std::vector<std::function<void()>> steps) {
    m_loadSteps = std::move(steps);
    m_loadStepIndex = 0;
    m_loadingScreen.show();
    m_loadingScreen.setProgress(0.f);
    m_gameState = GameState::Loading;
}

void Game::updateLoading(float dt) {
    if (m_loadStepIndex < m_loadSteps.size()) {
        m_loadSteps[m_loadStepIndex]();
        ++m_loadStepIndex;
        m_loadingScreen.setProgress(
            static_cast<float>(m_loadStepIndex) / static_cast<float>(m_loadSteps.size()));
    }

    m_loadingScreen.update(dt);

    if (m_loadStepIndex >= m_loadSteps.size() && m_loadingScreen.isFinished()) {
        m_loadSteps.clear();
        m_gameState = GameState::Playing;
    }
}

void Game::applyLoadedData(const SaveData& sd, int slot) {
    m_farming = FarmingSystem{};
    m_interactTiles = InteractTileManager{};
    m_npcManager = NPCManager{};
    m_coinPickup.reset();

    m_slotBaseMinutes = sd.playMinutes;
    m_playSeconds = 0.f;

    m_player.setPosition({ sd.playerX, sd.playerY });

    if (sd.houseHasData)
        m_playerHouse.setTilePos({ sd.houseTileX, sd.houseTileY });
    else
        m_playerHouse.setTilePos(m_landmarks.getPlayerSlotPos());

    m_gameHour = sd.gameHour;
    m_wallet = Wallet{};
    m_wallet.addSilver(sd.coins);
    for (int i = 0; i < Inventory::SLOTS; ++i) {
        m_inventory.slots[i].type = static_cast<ItemType>(sd.itemTypes[i]);
        m_inventory.slots[i].count = sd.itemCounts[i];
    }

    spawnVillage();
    SaveSystem::loadTiles(slot, m_tileMap);

    if (m_inventory.countOf(ItemType::Map) == 0)
        m_inventory.addItem(ItemType::Map, 1);

    m_camera.snapTo(m_player.getPosition());
}

void Game::saveGame() {
    SaveData sd = buildSaveData();
    SaveSystem::save(m_currentSlot, sd);
    SaveSystem::saveTiles(m_currentSlot, m_tileMap);
}

void Game::autosave() {
    if (m_gameState == GameState::MainMenu ||
        m_gameState == GameState::SaveSelect ||
        m_gameState == GameState::Loading) return;

    SaveData sd = buildSaveData();
    SaveSystem::save(0, sd);
    SaveSystem::saveTiles(0, m_tileMap);
}

void Game::loadGame() {
    SaveData sd;
    if (!SaveSystem::load(0, sd)) return;
    applyLoadedData(sd);
}

void Game::spawnVillage() {
    sf::Vector2f base = m_player.getPosition();

    auto npcWorldPos = [&](int npcIndex) {
        sf::Vector2i tp = m_landmarks.getNPCSlotPos(npcIndex);
        return sf::Vector2f{
            static_cast<float>(tp.x * TileMap::TILE_SIZE),
            static_cast<float>(tp.y * TileMap::TILE_SIZE) };
        };

    m_npcManager.spawnNPC(NPCType::Farmer, npcWorldPos(0), "Marko");
    m_npcManager.spawnNPC(NPCType::Merchant, npcWorldPos(1), "Ana");
    m_npcManager.spawnNPC(NPCType::Elder, npcWorldPos(2), "Elder Vid");
    m_coinPickup.spawnCoins(base, 25, m_seed, m_tileMap);
    m_interactTiles.add({ InteractTileType::Sign, {129, 128},
        "Village Entrance", "Welcome to Greenveil.", false, false });

    for (const auto& slot : m_landmarks.getSlots()) {
        if (slot.role != LandmarkRole::ForSale) continue;
        m_interactTiles.add({ InteractTileType::Sign,
            { slot.tilePos.x + LandmarkSystem::SLOT_WIDTH / 2,
              slot.tilePos.y + LandmarkSystem::SLOT_HEIGHT / 2 },
            "For Sale", "This ranch parcel is available for purchase.", false, false });
    }
}

void Game::run() {
    float accumulator = 0.f;
    while (m_window.isOpen()) {
        float frameTime = m_clock.restart().asSeconds();
        if (frameTime > 0.05f) frameTime = 0.05f;
        processEvents();

        if (m_gameState == GameState::Loading) {
            updateLoading(frameTime);
        }
        else {
            accumulator += frameTime;
            while (accumulator >= FIXED_DT) {
                update(FIXED_DT);
                accumulator -= FIXED_DT;
            }
        }
        render();
    }
}

void Game::processEvents() {
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            autosave();
            m_window.close();
        }

        if (m_gameState == GameState::Loading)
            continue;

        if (m_gameState == GameState::MainMenu) {
            auto action = m_mainMenu.handleEvent(*event, m_window.getSize());
            if (action == MenuAction::NewGame) {
                beginLoading({
                    [this] {
                        SaveSystem::deleteSlot(0);
                        SaveSystem::deleteSlot(1);
                        SaveSystem::deleteSlot(2);
                        m_seed = static_cast<unsigned>(std::time(nullptr));
                        m_wallet = Wallet{}; m_wallet.gold = 2; m_wallet.silver = 50;
                        m_inventory = Inventory{};
                        m_inventory.addItem(ItemType::Map, 1);
                        m_farming = FarmingSystem{};
                        m_npcManager = NPCManager{};
                        m_interactTiles = InteractTileManager{};
                        m_coinPickup.reset();
                        m_playSeconds = 0.f;
                        m_gameHour = 8.f;
                        m_slotBaseMinutes = 0;
                    },
                    [this] { m_tileMap.generate(256, 256, m_seed); },
                    [this] {
                        m_landmarks.assignRoles(m_seed, 3);
                        std::vector<sf::Vector2i> entries;
                        for (const auto& slot : m_landmarks.getSlots())
                            entries.push_back({
                                slot.tilePos.x + LandmarkSystem::SLOT_WIDTH / 2,
                                slot.tilePos.y + LandmarkSystem::SLOT_HEIGHT });
                        sf::Vector2i mCenter = {
                            m_landmarks.getMarketPos().x + m_landmarks.getMarketSize().x / 2,
                            m_landmarks.getMarketPos().y + m_landmarks.getMarketSize().y / 2 };
                        m_tileMap.connectLandmarks(entries, mCenter, m_seed);
                    },
                    [this] {
                        sf::Vector2i slotPos = m_landmarks.getPlayerSlotPos();
                        m_playerHouse.setTilePos(slotPos);
                        m_player.setPosition({
                            static_cast<float>((slotPos.x + LandmarkSystem::SLOT_WIDTH / 2) * TileMap::TILE_SIZE),
                            static_cast<float>((slotPos.y + LandmarkSystem::SLOT_HEIGHT + 2) * TileMap::TILE_SIZE) });
                    },
                    [this] { m_bushes.generate(m_tileMap, m_seed, m_playerHouse.getOccupiedTileSet()); },
                    [this] {
                        std::unordered_set<int> treeBlocked = m_bushes.getTileSet();
                        auto houseSet = m_playerHouse.getOccupiedTileSet();
                        treeBlocked.insert(houseSet.begin(), houseSet.end());
                        m_trees.generate(m_tileMap, m_seed, treeBlocked);
                    },
                    [this] { m_grassDecor.generate(m_tileMap, m_seed, m_trees.getTileSet()); },
                    [this] { m_mapSystem.bake(m_tileMap, m_trees, m_bushes, m_landmarks); },
                    [this] { spawnVillage(); },
                                        [this] { m_camera.snapTo(m_player.getPosition()); }
                    });
            }
            if (action == MenuAction::Continue) {
                int bestSlot = -1;
                int bestMinutes = -1;
                SaveData bestData{};
                for (int i = 0; i < 3; ++i) {
                    SaveData sd{};
                    if (SaveSystem::load(i, sd) && sd.hasData && sd.playMinutes > bestMinutes) {
                        bestMinutes = sd.playMinutes;
                        bestSlot = i;
                        bestData = sd;
                    }
                }
                if (bestSlot >= 0) {
                    m_pendingLoadData = bestData;
                    beginLoading({
                        [this, bestSlot] {
                            m_currentSlot = bestSlot;
                            m_seed = m_pendingLoadData.mapSeed;
                        },
                        [this] { m_tileMap.generate(256, 256, m_seed); },
                        [this] {
                            m_landmarks.assignRoles(m_seed, 3);
                            std::vector<sf::Vector2i> entries;
                            for (const auto& slot : m_landmarks.getSlots())
                                entries.push_back({
                                    slot.tilePos.x + LandmarkSystem::SLOT_WIDTH / 2,
                                    slot.tilePos.y + LandmarkSystem::SLOT_HEIGHT });
                            sf::Vector2i mCenter = {
                                m_landmarks.getMarketPos().x + m_landmarks.getMarketSize().x / 2,
                                m_landmarks.getMarketPos().y + m_landmarks.getMarketSize().y / 2 };
                            m_tileMap.connectLandmarks(entries, mCenter, m_seed);
                        },
                        [this, bestSlot] { applyLoadedData(m_pendingLoadData, bestSlot); },
                        [this] { m_bushes.generate(m_tileMap, m_seed, m_playerHouse.getOccupiedTileSet()); },
                        [this] {
                            std::unordered_set<int> treeBlocked = m_bushes.getTileSet();
                            auto houseSet = m_playerHouse.getOccupiedTileSet();
                            treeBlocked.insert(houseSet.begin(), houseSet.end());
                            m_trees.generate(m_tileMap, m_seed, treeBlocked);
                        },
                        [this] { m_grassDecor.generate(m_tileMap, m_seed, m_trees.getTileSet()); },
                        [this] { m_mapSystem.bake(m_tileMap, m_trees, m_bushes, m_landmarks); }
                        });
                }
                else {
                    beginLoading({
                        [this] {
                            m_seed = static_cast<unsigned>(std::time(nullptr));
                            m_wallet = Wallet{}; m_wallet.gold = 2; m_wallet.silver = 50;
                            m_inventory = Inventory{};
                            m_inventory.addItem(ItemType::Map, 1);
                            m_farming = FarmingSystem{};
                            m_npcManager = NPCManager{};
                            m_interactTiles = InteractTileManager{};
                            m_coinPickup.reset();
                            m_playSeconds = 0.f;
                            m_gameHour = 8.f;
                        },
                        [this] { m_tileMap.generate(256, 256, m_seed); },
                        [this] {
                            m_landmarks.assignRoles(m_seed, 3);
                            std::vector<sf::Vector2i> entries;
                            for (const auto& slot : m_landmarks.getSlots())
                                entries.push_back({
                                    slot.tilePos.x + LandmarkSystem::SLOT_WIDTH / 2,
                                    slot.tilePos.y + LandmarkSystem::SLOT_HEIGHT });
                            sf::Vector2i mCenter = {
                                m_landmarks.getMarketPos().x + m_landmarks.getMarketSize().x / 2,
                                m_landmarks.getMarketPos().y + m_landmarks.getMarketSize().y / 2 };
                            m_tileMap.connectLandmarks(entries, mCenter, m_seed);
                        },
                        [this] {
                            sf::Vector2i slotPos = m_landmarks.getPlayerSlotPos();
                            m_playerHouse.setTilePos(slotPos);
                            m_player.setPosition({
                                static_cast<float>((slotPos.x + LandmarkSystem::SLOT_WIDTH / 2) * TileMap::TILE_SIZE),
                                static_cast<float>((slotPos.y + LandmarkSystem::SLOT_HEIGHT + 2) * TileMap::TILE_SIZE) });
                        },
                        [this] { m_bushes.generate(m_tileMap, m_seed, m_playerHouse.getOccupiedTileSet()); },
                        [this] {
                            std::unordered_set<int> treeBlocked = m_bushes.getTileSet();
                            auto houseSet = m_playerHouse.getOccupiedTileSet();
                            treeBlocked.insert(houseSet.begin(), houseSet.end());
                            m_trees.generate(m_tileMap, m_seed, treeBlocked);
                        },
                        [this] { m_grassDecor.generate(m_tileMap, m_seed, m_trees.getTileSet()); },
                        [this] { m_mapSystem.bake(m_tileMap, m_trees, m_bushes, m_landmarks); },
                        [this] { spawnVillage(); },
                        [this] { m_camera.snapTo(m_player.getPosition()); }
                        });
                }
            }
            if (action == MenuAction::Exit) m_window.close();
            continue;
        }

        if (m_gameState == GameState::SaveSelect) {
            if (const auto* k = event->getIf<sf::Event::KeyPressed>())
                if (k->code == sf::Keyboard::Key::Escape)
                    m_gameState = GameState::MainMenu;

            if (const auto* c = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mp((float)c->position.x, (float)c->position.y);
                SaveAction action = m_saveUI.handleClick(mp, m_slotPreviews);

                auto doNew = [&](int slot) {
                    SaveSystem::deleteSlot(slot);
                    m_seed = static_cast<unsigned>(std::time(nullptr));
                    m_tileMap.generate(256, 256, m_seed);
                    m_landmarks.assignRoles(m_seed, 3);
                    {
                        std::vector<sf::Vector2i> entries;
                        for (const auto& slot : m_landmarks.getSlots())
                            entries.push_back({
                                slot.tilePos.x + LandmarkSystem::SLOT_WIDTH / 2,
                                slot.tilePos.y + LandmarkSystem::SLOT_HEIGHT });
                        sf::Vector2i mCenter = {
                            m_landmarks.getMarketPos().x + m_landmarks.getMarketSize().x / 2,
                            m_landmarks.getMarketPos().y + m_landmarks.getMarketSize().y / 2 };
                        m_tileMap.connectLandmarks(entries, mCenter, m_seed);
                    }
                    {
                        sf::Vector2i slotPos = m_landmarks.getPlayerSlotPos();
                        m_playerHouse.setTilePos(slotPos);
                        m_player.setPosition({
                            static_cast<float>((slotPos.x + LandmarkSystem::SLOT_WIDTH / 2) * TileMap::TILE_SIZE),
                            static_cast<float>((slotPos.y + LandmarkSystem::SLOT_HEIGHT + 2) * TileMap::TILE_SIZE) });
                    }

                    m_bushes.generate(m_tileMap, m_seed, m_playerHouse.getOccupiedTileSet());

                    std::unordered_set<int> treeBlocked4 = m_bushes.getTileSet();
                    {
                        auto houseSet = m_playerHouse.getOccupiedTileSet();
                        treeBlocked4.insert(houseSet.begin(), houseSet.end());
                    }
                    m_trees.generate(m_tileMap, m_seed, treeBlocked4);

                    m_grassDecor.generate(m_tileMap, m_seed, m_trees.getTileSet());
                    m_mapSystem.bake(m_tileMap, m_trees, m_bushes, m_landmarks);

                    m_wallet = Wallet{}; m_wallet.gold = 2; m_wallet.silver = 50;
                    m_inventory = Inventory{};
                    m_inventory.addItem(ItemType::Map, 1);
                    spawnVillage();
                    m_gameState = GameState::Playing;
                    };
                auto doLoad = [&](int slot) {
                    m_currentSlot = slot;
                    m_seed = m_slotPreviews[slot].mapSeed;
                    m_tileMap.generate(256, 256, m_seed);
                    m_landmarks.assignRoles(m_seed, 3);
                    {
                        std::vector<sf::Vector2i> entries;
                        for (const auto& s : m_landmarks.getSlots())
                            entries.push_back({
                                s.tilePos.x + LandmarkSystem::SLOT_WIDTH / 2,
                                s.tilePos.y + LandmarkSystem::SLOT_HEIGHT });
                        sf::Vector2i mCenter = {
                            m_landmarks.getMarketPos().x + m_landmarks.getMarketSize().x / 2,
                            m_landmarks.getMarketPos().y + m_landmarks.getMarketSize().y / 2 };
                        m_tileMap.connectLandmarks(entries, mCenter, m_seed);
                    }

                    applyLoadedData(m_slotPreviews[slot], slot);

                    m_bushes.generate(m_tileMap, m_seed, m_playerHouse.getOccupiedTileSet());

                    std::unordered_set<int> treeBlocked5 = m_bushes.getTileSet();
                    {
                        auto houseSet = m_playerHouse.getOccupiedTileSet();
                        treeBlocked5.insert(houseSet.begin(), houseSet.end());
                    }
                    m_trees.generate(m_tileMap, m_seed, treeBlocked5);

                    m_grassDecor.generate(m_tileMap, m_seed, m_trees.getTileSet());
                    m_mapSystem.bake(m_tileMap, m_trees, m_bushes, m_landmarks);
                    m_gameState = GameState::Playing;
                    };
                auto doDelete = [&](int slot) {
                    SaveSystem::deleteSlot(slot);
                    m_slotPreviews[slot] = {};
                    };
                auto doSave = [&](int slot) {
                    m_currentSlot = slot;
                    SaveData sd = buildSaveData();
                    SaveSystem::save(slot, sd);
                    SaveSystem::saveTiles(slot, m_tileMap);
                    m_slotPreviews[slot] = sd;
                    };

                if (m_saveMode == SaveMode::NewGame) {
                    if (action == SaveAction::Save0) doNew(0);
                    if (action == SaveAction::Save1) doNew(1);
                    if (action == SaveAction::Save2) doNew(2);
                }
                else {
                    switch (action) {
                    case SaveAction::Load0: doLoad(0); break;
                    case SaveAction::Load1: doLoad(1); break;
                    case SaveAction::Load2: doLoad(2); break;
                    case SaveAction::Save0: doSave(0); break;
                    case SaveAction::Save1: doSave(1); break;
                    case SaveAction::Save2: doSave(2); break;
                    case SaveAction::Delete0: doDelete(0); break;
                    case SaveAction::Delete1: doDelete(1); break;
                    case SaveAction::Delete2: doDelete(2); break;
                    default: break;
                    }
                }
            }
        continue;
        }

        if (const auto* k = event->getIf<sf::Event::KeyPressed>()) {

            if (k->scancode == sf::Keyboard::Scancode::Unknown) break;

            if (k->code == sf::Keyboard::Key::N && !m_nKeyHeld) {
                m_nKeyHeld = true;
                printf("N PRESSED, active=%p\n", m_npcManager.getMissions().getActiveMission());
                const Mission* active = m_npcManager.getMissions().getActiveMission();
                if (active && m_npcManager.canTurnIn(m_player.getPosition())) {
                    int reward = active->rewardCoins;
                    int missionId = active->id;
                    int requireCrops = active->requireCrops;
                    int fishCount = m_inventory.countOf(ItemType::Carp) +
                        m_inventory.countOf(ItemType::Trout) +
                        m_inventory.countOf(ItemType::Salmon) +
                        m_inventory.countOf(ItemType::Catfish) +
                        m_inventory.countOf(ItemType::GoldenFish);
                    if (m_npcManager.tryTurnIn(m_player.getPosition(), fishCount)) {
                        printf("TURNIN SUCCESS\n");
                        m_fKeyHeld = true;
                        if (requireCrops > 0) {
                            if (missionId == 3) {
                                int toRemove = requireCrops;
                                for (auto fishType : { ItemType::Carp, ItemType::Trout,
                                    ItemType::Salmon, ItemType::Catfish, ItemType::GoldenFish }) {
                                    int has = m_inventory.countOf(fishType);
                                    int remove = std::min(has, toRemove);
                                    if (remove > 0) m_inventory.removeItem(fishType, remove);
                                    toRemove -= remove;
                                    if (toRemove <= 0) break;
                                }
                            }
                            else {
                                m_inventory.removeItem(ItemType::Crop, requireCrops);
                            }
                        }
                        m_wallet.addSilver(reward);
                    }
                }
            }

            if (m_marketUI.isOpen()) {
                if (k->code == sf::Keyboard::Key::M ||
                    k->code == sf::Keyboard::Key::Escape)
                    m_marketUI.close();
            }
            else if (m_skillUI.isVisible()) {
                if (k->code == sf::Keyboard::Key::C ||
                    k->code == sf::Keyboard::Key::Escape)
                    m_skillUI.toggle();
            }
            else {
                switch (k->code) {
            case sf::Keyboard::Key::Escape:
                if (m_gameState == GameState::HouseInterior) {
                    m_gameState = GameState::Playing;
                    m_player.setPosition(m_returnPos);
                    m_camera.snapTo(m_player.getPosition());
                    break;
                }
                if (m_interactPrompt.isVisible()) { m_interactPrompt.hide(); break; }
                if (m_mapUI.isOpen()) { m_mapUI.close(); break; }
                if (m_npcManager.isDialogueOpen()) { m_npcManager.closeDialogue(); break; }
                if (m_buildingUI.isOpen()) { m_buildingUI.close(); break; }
                if (m_buildingUI.hasSelected()) { m_buildingUI.clearSelected(); break; }
                if (m_gameState == GameState::SaveMenu) { m_gameState = GameState::Playing; break; }
                if (m_gameState == GameState::PauseMenu) { m_gameState = GameState::Playing; break; }
                m_gameState = GameState::PauseMenu;
                break;

            case sf::Keyboard::Key::C:
                if (m_cKeyHeld) break;
                m_cKeyHeld = true;
                if (m_marketUI.isOpen()) m_marketUI.close();
                if (m_inventory.isBackpackOpen()) m_inventory.toggleBackpack();
                m_skillUI.toggle();
                break;

            case sf::Keyboard::Key::M:
                if (m_mKeyHeld) break;
                m_mKeyHeld = true;
                if (!m_npcManager.isDialogueOpen()) {
                    if (m_marketUI.isOpen()) {
                        m_marketUI.close();
                    }
                    else {
                        if (m_skillUI.isVisible()) m_skillUI.toggle();
                        if (m_inventory.isBackpackOpen()) m_inventory.toggleBackpack();
                        m_marketUI.open(m_gameHour);
                    }
                }
                break;

            case sf::Keyboard::Key::F5: saveGame(); break;
            case sf::Keyboard::Key::F9: loadGame(); break;

            case sf::Keyboard::Key::F:
                if (m_fKeyHeld) break;
                m_fKeyHeld = true;

                if (m_gameState == GameState::HouseInterior) {
                    m_gameState = GameState::Playing;
                    m_player.setPosition(m_returnPos);
                    m_camera.snapTo(m_player.getPosition());
                    break;
                }
                if (m_interactPrompt.isVisible()) {
                    m_interactPrompt.hide(); break;
                }
                if (m_npcManager.isDialogueOpen()) {
                    m_npcManager.advanceDialogue(
                        m_inventory.countOf(ItemType::Crop), 0);
                    break;
                }
                {
                    std::string t, b;
                    if (m_interactTiles.tryInteract(m_player.getPosition(), t, b)) {
                        m_interactPrompt.show(t, b); break;
                    }
                }
                if (m_inventory.selectedItem() == ItemType::FishingRod) {
                    m_fishing.onActionPressed(m_player.getPosition(), m_player.getFacing());
                    break;
                }
                if (!m_npcManager.tryInteract(m_player.getPosition(), m_gameHour)) {
                    if (m_inventory.countOf(ItemType::Seed) > 0)
                        if (m_farming.tryPlant(m_player.getPosition(), m_tileMap))
                            m_inventory.removeItem(ItemType::Seed);
                }
                break;

            case sf::Keyboard::Key::E:
                if (m_eKeyHeld) break;
                m_eKeyHeld = true;
                if (!m_npcManager.isDialogueOpen())  
                    m_farming.tryTill(
                    m_player.getPosition(), m_tileMap,
                    m_inventory.selectedItem() == ItemType::Hoe);
                break;

            case sf::Keyboard::Key::R:
                if (m_rKeyHeld) break;
                m_rKeyHeld = true;
                if (!m_npcManager.isDialogueOpen()) {
                    int seeds = 0;
                    if (m_farming.tryHarvest(
                        m_player.getPosition(), m_tileMap, seeds)) {
                        int bonus = static_cast<int>(m_progression.getFarmYield());
                        if (seeds + bonus > 0)
                            m_inventory.addItem(ItemType::Crop, seeds + bonus);
                    }
                }
                break;

            case sf::Keyboard::Key::Space:
                if (m_spaceKeyHeld) break;
                m_spaceKeyHeld = true;
                if (!m_player.isJumping()) {
                    m_player.startJump();
                }
                break;

            case sf::Keyboard::Key::B:
                if (m_bKeyHeld) break;
                m_bKeyHeld = true;
                if (!m_npcManager.isDialogueOpen()) {
                    if (m_buildingUI.isOpen()) {
                        m_buildingUI.close();
                    }
                    else {
                        if (m_marketUI.isOpen())  m_marketUI.close();
                        if (m_skillUI.isVisible()) m_skillUI.toggle();
                        if (m_inventory.isBackpackOpen()) m_inventory.toggleBackpack();
                        m_buildingUI.toggle();
                    }
                }
                break;

                if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                    UI::init(resized->size.x, resized->size.y);
                    m_camera = Camera(resized->size.x, resized->size.y);
                }

            case sf::Keyboard::Key::Tab:
                if (m_tabKeyHeld) break;
                m_tabKeyHeld = true;
                if (!m_npcManager.isDialogueOpen() && !m_skillUI.isVisible())
                    m_inventory.toggleBackpack();
                break;

            case sf::Keyboard::Key::Num1: m_inventory.selectedSlot = 0; break;
            case sf::Keyboard::Key::Num2: m_inventory.selectedSlot = 1; break;
            case sf::Keyboard::Key::Num3: m_inventory.selectedSlot = 2; break;
            case sf::Keyboard::Key::Num4: m_inventory.selectedSlot = 3; break;
            case sf::Keyboard::Key::Num5: m_inventory.selectedSlot = 4; break;
            default: break;
            }
        }
    }

        
        if (const auto* k = event->getIf<sf::Event::KeyReleased>()) {
            switch (k->code) {
            case sf::Keyboard::Key::F:     m_fKeyHeld = false; break;
            case sf::Keyboard::Key::E:     m_eKeyHeld = false; break;
            case sf::Keyboard::Key::R:     m_rKeyHeld = false; break;
            case sf::Keyboard::Key::B:     m_bKeyHeld = false; break;
            case sf::Keyboard::Key::M:     m_mKeyHeld = false; break;
            case sf::Keyboard::Key::C:     m_cKeyHeld = false; break;
            case sf::Keyboard::Key::Tab:   m_tabKeyHeld = false; break;
            case sf::Keyboard::Key::Space: m_spaceKeyHeld = false; break;
            case sf::Keyboard::Key::N: m_nKeyHeld = false; break;
            default: break;
            }
        }
        
        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            sf::Vector2f mpos = {
                static_cast<float>(click->position.x),
                static_cast<float>(click->position.y) };

            if (m_gameState == GameState::PauseMenu) {
                auto action = m_pauseMenu.handleClick(mpos, m_window.getSize());
                if (action == PauseAction::Resume)   m_gameState = GameState::Playing;
                if (action == PauseAction::SaveLoad) {
                    for (int i = 0; i < 3; ++i) {
                        m_slotPreviews[i] = {};
                        SaveSystem::load(i, m_slotPreviews[i]);
                    }
                    m_gameState = GameState::SaveMenu;
                }
                if (action == PauseAction::Quit) { autosave(); m_window.close(); }
                return;
            }

            if (m_gameState == GameState::SaveMenu) {
                SaveAction action = m_saveUI.handleClick(mpos, m_slotPreviews);

                auto doSave = [&](int slot) {
                    SaveData sd = buildSaveData();
                    SaveSystem::save(slot, sd);
                    SaveSystem::saveTiles(slot, m_tileMap);
                    m_slotPreviews[slot] = sd;
                    };
                auto doDelete = [&](int slot) {
                    SaveSystem::deleteSlot(slot);
                    m_slotPreviews[slot] = {};
                    };
                auto doLoad = [&](int slot) {
                    m_pendingLoadData = m_slotPreviews[slot];
                    beginLoading({
                        [this, slot] {
                            m_currentSlot = slot;
                            m_seed = m_pendingLoadData.mapSeed;
                        },
                        [this] { m_tileMap.generate(256, 256, m_seed); },
                        [this] {
                            m_landmarks.assignRoles(m_seed, 3);
                            std::vector<sf::Vector2i> entries;
                            for (const auto& s : m_landmarks.getSlots())
                                entries.push_back({
                                    s.tilePos.x + LandmarkSystem::SLOT_WIDTH / 2,
                                    s.tilePos.y + LandmarkSystem::SLOT_HEIGHT });
                            sf::Vector2i mCenter = {
                                m_landmarks.getMarketPos().x + m_landmarks.getMarketSize().x / 2,
                                m_landmarks.getMarketPos().y + m_landmarks.getMarketSize().y / 2 };
                            m_tileMap.connectLandmarks(entries, mCenter, m_seed);
                        },
                        [this, slot] { applyLoadedData(m_pendingLoadData, slot); },
                        [this] { m_bushes.generate(m_tileMap, m_seed, m_playerHouse.getOccupiedTileSet()); },
                        [this] {
                            std::unordered_set<int> treeBlocked = m_bushes.getTileSet();
                            auto houseSet = m_playerHouse.getOccupiedTileSet();
                            treeBlocked.insert(houseSet.begin(), houseSet.end());
                            m_trees.generate(m_tileMap, m_seed, treeBlocked);
                        },
                        [this] { m_grassDecor.generate(m_tileMap, m_seed, m_trees.getTileSet()); },
                        [this] { m_mapSystem.bake(m_tileMap, m_trees, m_bushes, m_landmarks); }
                        });
                    };

                switch (action) {
                case SaveAction::Save0:   doSave(0);   break;
                case SaveAction::Save1:   doSave(1);   break;
                case SaveAction::Save2:   doSave(2);   break;
                case SaveAction::Load0:   doLoad(0);   break;
                case SaveAction::Load1:   doLoad(1);   break;
                case SaveAction::Load2:   doLoad(2);   break;
                case SaveAction::Delete0: doDelete(0); break;
                case SaveAction::Delete1: doDelete(1); break;
                case SaveAction::Delete2: doDelete(2); break;
                default: break;
                }
                return;
            }

            if (m_buildingUI.isOpen()) {
                m_buildingUI.handleClick(mpos, m_building, m_inventory);
                break;
            }

            if (m_marketUI.isOpen()) {
                m_marketUI.handleClick(mpos, m_market, m_inventory, m_wallet);
                break;
            }

            if (m_skillUI.isVisible()) {
                int skill = m_skillUI.handleClick(mpos, m_progression);
                if (skill >= 0)
                    m_progression.upgradeSkill(static_cast<SkillType>(skill));
                break;
            }

            if (m_mapUI.isOpen()) {
                if (click->button == sf::Mouse::Button::Right)
                    m_mapUI.close();
                break;
            }

            if (click->button == sf::Mouse::Button::Right &&
                m_inventory.selectedItem() == ItemType::Map) {
                m_mapUI.toggle();
                break;
            }

            if (click->button == sf::Mouse::Button::Right) {
                if (m_npcManager.isDialogueOpen()) {
                    m_npcManager.advanceDialogue(
                        m_inventory.countOf(ItemType::Crop), 0);
                }
                else if (m_buildingUI.hasSelected()) {
                    m_buildingUI.clearSelected();
                }
                else {
                    sf::Vector2f mouseWorld = m_window.mapPixelToCoords(
                        sf::Mouse::getPosition(m_window), m_camera.getView());
                    m_building.tryDestroy(mouseWorld, m_inventory);
                }
            }
            if (click->button == sf::Mouse::Button::Left) {
                sf::Vector2f mposUI = {
                    static_cast<float>(click->position.x),
                    static_cast<float>(click->position.y) };
                if (!m_marketUI.isOpen() && !m_skillUI.isVisible())
                    m_inventory.onMousePressed(mposUI);
            }
            if (click->button == sf::Mouse::Button::Left &&
                !m_npcManager.isDialogueOpen()) {
                if (m_buildingUI.hasSelected() && !m_buildingUI.isOpen()) {
                    sf::Vector2f mouseWorld = m_window.mapPixelToCoords(
                        sf::Mouse::getPosition(m_window), m_camera.getView());
                    int tx = static_cast<int>(mouseWorld.x) / TileMap::TILE_SIZE;
                    int ty = static_cast<int>(mouseWorld.y) / TileMap::TILE_SIZE;
                    m_building.tryPlace(m_buildingUI.getSelected(),
                        { tx, ty }, m_inventory, m_tileMap);
                    const auto& def = m_building.getDef(m_buildingUI.getSelected());
                    if (m_inventory.countOf(def.material) < def.cost)
                        m_buildingUI.clearSelected();
                }
            }
        }

        if (const auto* rel = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (rel->button == sf::Mouse::Button::Left) {
                sf::Vector2f mposUI = {
                    static_cast<float>(rel->position.x),
                    static_cast<float>(rel->position.y) };
                m_inventory.onMouseReleased(mposUI);
            }
        }

        if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
            if (m_marketUI.isOpen()) {
                m_marketUI.handleScroll(scroll->delta, m_market);
            }
            else if (!m_npcManager.isDialogueOpen() && !m_skillUI.isVisible()) {
                if (scroll->delta < 0)
                    m_inventory.selectedSlot =
                    (m_inventory.selectedSlot + 1) % Inventory::HOTBAR_SLOTS;
                else
                    m_inventory.selectedSlot =
                    (m_inventory.selectedSlot - 1 + Inventory::HOTBAR_SLOTS) %
                    Inventory::HOTBAR_SLOTS;
            }
        }
        if (const auto* moved = event->getIf<sf::Event::MouseMoved>()) {
            sf::Vector2f mposUI = {
                static_cast<float>(moved->position.x),
                static_cast<float>(moved->position.y) };
            m_inventory.onMouseMoved(mposUI);
        }
    }
}

void Game::update(float dt) {
    if (m_gameState == GameState::PauseMenu) return;
    if (m_gameState == GameState::MainMenu) return;
    if (m_gameOver) return;

    if (m_gameState == GameState::Playing)
        m_playSeconds += dt;

    if (m_gameState == GameState::HouseInterior) {
        sf::Vector2f vel{ 0.f, 0.f };
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) vel.y -= Player::SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) vel.y += Player::SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) vel.x -= Player::SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) vel.x += Player::SPEED;
        if (vel.x != 0.f && vel.y != 0.f) { vel.x *= 0.7071f; vel.y *= 0.7071f; }

        sf::Vector2f cur = m_player.getPosition();
        sf::Vector2f next = cur + vel * dt;
        if (m_playerHouse.isWithinInteriorBounds(next))
            m_player.setPosition(next);

        m_camera.snapTo(m_player.getPosition());
        return;
    }

    m_prevHour = m_gameHour;
    bool  isNightTime = (m_gameHour >= 21.f || m_gameHour < 6.f);
    float hourSpeed = isNightTime ? GAME_HOUR_SPEED_NIGHT : GAME_HOUR_SPEED_DAY;
    m_gameHour += hourSpeed * dt;

    if (m_gameHour >= 24.f) {
        m_gameHour -= 24.f;
    }

    auto ws = m_window.getSize();
    if (ws.x != m_windowW || ws.y != m_windowH) {
        m_windowW = ws.x;
        m_windowH = ws.y;
        UI::init(ws.x, ws.y);
        m_camera = Camera(ws.x, ws.y);
        m_camera.snapTo(m_player.getPosition());
    }

    while (m_progression.levelUp()) {}

    if (!m_npcManager.isDialogueOpen())
        m_player.update(dt, m_tileMap, m_building, m_bushes, m_trees, m_playerHouse);

    bool nowAtDoor = m_playerHouse.isAtDoorTrigger(m_player.getPosition());
    if (nowAtDoor && !m_wasAtDoorTrigger) {
        m_returnPos = m_player.getPosition();
        m_gameState = GameState::HouseInterior;
        m_player.setPosition(m_playerHouse.getInteriorSpawnPos());
        m_camera.snapTo(m_player.getPosition());
        m_wasAtDoorTrigger = true;
        return;
    }
    m_wasAtDoorTrigger = nowAtDoor;

    {
        ItemType held = m_inventory.selectedItem();
        WeaponType wt = WeaponType::Bare;
        if (held == ItemType::FishingRod) wt = WeaponType::Rod;
        m_player.setWeaponType(wt);
    }

    m_camera.follow(m_player.getPosition(), dt);
    m_farming.update(dt, m_tileMap);
    m_building.update(dt);
    m_market.update(dt, m_gameHour);
    m_marketUI.update(dt, m_gameHour, m_market);

    m_fishing.update(dt, m_player.getPosition(), m_tileMap);

    m_coinPickup.update(m_player.getPosition());
    int coins = m_coinPickup.pollCollected();
    if (coins > 0) m_wallet.addSilver(coins);


    const FishDef* caught = m_fishing.pollCatch();
    if (caught) {
        m_inventory.addItem(caught->type, 1);
        m_progression.addXP(static_cast<int>(caught->rarity * 80.f) + 10);
    }

    m_npcManager.update(dt, m_gameHour, m_tileMap);
    m_interactTiles.update(m_player.getPosition());

    const Mission* active = m_npcManager.getMissions().getActiveMission();
    int cropCount = (active && active->id == 3)
        ? m_inventory.countOf(ItemType::Carp) +
        m_inventory.countOf(ItemType::Trout) +
        m_inventory.countOf(ItemType::Salmon) +
        m_inventory.countOf(ItemType::Catfish) +
        m_inventory.countOf(ItemType::GoldenFish)
        : m_inventory.countOf(ItemType::Crop);

    m_npcManager.getMissions().update(
        m_inventory.countOf(ItemType::Crop),
        m_inventory.countOf(ItemType::Carp) +
        m_inventory.countOf(ItemType::Trout) +
        m_inventory.countOf(ItemType::Salmon) +
        m_inventory.countOf(ItemType::Catfish) +
        m_inventory.countOf(ItemType::GoldenFish));

    if (!m_player.isAlive()) m_gameOver = true;
}

void Game::render() {
    if (m_gameState == GameState::MainMenu) {
        m_window.clear();
        m_mainMenu.draw(m_window);
        m_window.display();
        return;
    }

    if (m_gameState == GameState::Loading) {
        m_window.clear();
        m_loadingScreen.draw(m_window);
        m_window.display();
        return;
    }

    if (m_gameState == GameState::HouseInterior) {
        m_window.clear(sf::Color::Black);

        m_camera.apply(m_window);
        m_playerHouse.drawInterior(m_window);
        m_player.draw(m_window);

        m_camera.applyUI(m_window);
        sf::Text hint(m_hud.getFont(), "[ ESC / F ] Exit House", 16);
        hint.setFillColor(sf::Color(220, 210, 180));
        sf::Vector2u ws = m_window.getSize();
        auto tb = hint.getLocalBounds();
        hint.setPosition({ (ws.x - tb.size.x) * 0.5f, ws.y - 40.f });
        m_window.draw(hint);

        m_window.display();
        return;
    }

    m_window.clear(sf::Color(20, 20, 20));

    m_camera.apply(m_window);
    m_tileMap.draw(m_window);
    m_grassDecor.draw(m_window);
    m_farming.draw(m_window);

    // Y-sort pass — Stardew-style depth sorting po Y anchor tacki (baza/noge)
    std::vector<RenderItem> renderQueue;
    m_bushes.collectRenderItems(renderQueue);
    m_trees.collectRenderItems(renderQueue);
    m_building.collectRenderItems(renderQueue);
    m_npcManager.collectRenderItems(renderQueue);
    if (m_playerHouse.hasHouse()) {
        float houseSortY = static_cast<float>(
            (m_playerHouse.getTilePos().y + m_playerHouse.getSize().y) * TileMap::TILE_SIZE);
        renderQueue.push_back({ houseSortY,
            [this](sf::RenderWindow& w) { m_playerHouse.draw(w); } });
    }
    renderQueue.push_back({ m_player.getPosition().y + 12.f,
        [this](sf::RenderWindow& w) { m_player.draw(w); } });

    std::sort(renderQueue.begin(), renderQueue.end(),
        [](const RenderItem& a, const RenderItem& b) { return a.sortY < b.sortY; });
    for (auto& item : renderQueue)
        item.draw(m_window);

    m_npcManager.drawDebugRanges(m_window);
    m_coinPickup.draw(m_window);

    if (m_buildingUI.hasSelected() && !m_buildingUI.isOpen()) {
        sf::Vector2f mouseWorld = m_window.mapPixelToCoords(
            sf::Mouse::getPosition(m_window), m_camera.getView());
        int tx = static_cast<int>(mouseWorld.x) / TileMap::TILE_SIZE;
        int ty = static_cast<int>(mouseWorld.y) / TileMap::TILE_SIZE;
        bool valid = m_building.canPlace(m_buildingUI.getSelected(),
            { tx, ty }, m_inventory, m_tileMap);
        m_building.drawPreview(m_window, m_buildingUI.getSelected(),
            { tx, ty }, valid);
    }

    if (m_fishing.isActive())
        m_fishingUI.drawBobber(m_window,
            m_fishing.getBobber(),
            m_fishing.getCastOrigin(),
            m_fishing.getState());

    m_interactTiles.draw(m_window);

    m_camera.applyUI(m_window);
    m_dayNight.draw(m_window, m_gameHour);

    m_interactPrompt.draw(m_window);
    m_npcManager.drawUI(m_window);

    if (!m_anyUIOpen()) {
        m_inventory.draw(m_window);
        m_hud.draw(m_window, m_player.getHp(),
            m_player.getMaxHp(), m_gameHour, m_wallet, m_player.getLives());
        bool canFish = m_fishing.canFish(m_player.getPosition(),
            m_tileMap, m_inventory);
        m_fishingUI.draw(m_window, m_fishing, canFish);
        m_objectiveUI.draw(m_window, m_npcManager.getMissions(),
            m_npcManager.canTurnIn(m_player.getPosition()));
        m_skillUI.draw(m_window, m_progression);
        m_buildingUI.draw(m_window, m_building, m_inventory);
        m_marketUI.draw(m_window, m_market, m_inventory, m_wallet, m_gameHour);
    }

    if (m_gameOver) {
        sf::RectangleShape overlay({
            static_cast<float>(m_windowW),
            static_cast<float>(m_windowH) });
        overlay.setFillColor(sf::Color(0, 0, 0, 160));
        m_window.draw(overlay);
    }

    if (m_gameState == GameState::SaveMenu) {
        m_camera.applyUI(m_window);
        m_saveUI.draw(m_window, m_slotPreviews);
    }

    if (m_mapUI.isOpen()) {
        m_camera.applyUI(m_window);
        sf::Vector2i mp = sf::Mouse::getPosition(m_window);
        m_mapUI.draw(m_window, m_mapSystem, m_player.getPosition(),
            sf::Vector2f{ static_cast<float>(mp.x), static_cast<float>(mp.y) });
    }

    if (m_gameState == GameState::PauseMenu)
        m_pauseMenu.draw(m_window);

    m_window.display();
}

void Game::deleteSave() { std::remove("savegame.bin"); }