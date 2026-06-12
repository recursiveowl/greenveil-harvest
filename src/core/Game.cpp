#include "core/Game.h"
#include <ctime>
#include <cstdlib>
#include <cmath>
#ifdef _WIN32
#include <windows.h>
#endif

static constexpr float FIXED_DT = 1.f / 60.f;
static constexpr float ENEMY_SPAWN_CD = 15.f;
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

    // Generiši privremeni world — MainMenu se prikazuje odmah
    m_seed = static_cast<unsigned>(std::time(nullptr));
    m_tileMap.generate(256, 256, m_seed);
    m_player.setPosition({
        static_cast<float>(128 * TileMap::TILE_SIZE),
        static_cast<float>(128 * TileMap::TILE_SIZE)
        });
    m_bushes.generate(m_tileMap, m_seed);
    m_grassDecor.generate(m_tileMap, m_seed);
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
    sd.level = m_progression.getLevel();
    sd.xp = m_progression.getXP();
    sd.skillPoints = m_progression.getSkillPoints();
    sd.coins = m_wallet.toSilver();
    sd.totalKills = m_npcManager.getMissions().getKillCount();
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

void Game::applyLoadedData(const SaveData& sd, int slot) {
    m_combat = CombatSystem{};
    m_farming = FarmingSystem{};
    m_interactTiles = InteractTileManager{};
    m_npcManager = NPCManager{};
    m_coinPickup.reset();

    m_slotBaseMinutes = sd.playMinutes;
    m_playSeconds = 0.f;

    m_player.setPosition({ sd.playerX, sd.playerY });
    m_gameHour = sd.gameHour;
    m_wallet = Wallet{};
    m_wallet.addSilver(sd.coins);
    for (int i = 0; i < Inventory::SLOTS; ++i) {
        m_inventory.slots[i].type = static_cast<ItemType>(sd.itemTypes[i]);
        m_inventory.slots[i].count = sd.itemCounts[i];
    }

    spawnVillage();
    SaveSystem::loadTiles(slot, m_tileMap);
    m_camera.snapTo(m_player.getPosition());
}

void Game::saveGame() {
    SaveData sd = buildSaveData();
    SaveSystem::save(m_currentSlot, sd);
    SaveSystem::saveTiles(m_currentSlot, m_tileMap);
}

void Game::loadGame() {
    SaveData sd;
    if (!SaveSystem::load(0, sd)) return;
    applyLoadedData(sd);
}

void Game::spawnVillage() {
    sf::Vector2f base = m_player.getPosition();

    m_npcManager.spawnNPC(NPCType::Farmer, base + sf::Vector2f{ 160.f,  80.f }, "Marko");
    m_npcManager.spawnNPC(NPCType::Merchant, base + sf::Vector2f{ -120.f,  60.f }, "Ana");
    m_npcManager.spawnNPC(NPCType::Guard, base + sf::Vector2f{ 200.f, -90.f }, "Boris");
    m_npcManager.spawnNPC(NPCType::Elder, base + sf::Vector2f{ -80.f,-140.f }, "Elder Vid");
    m_coinPickup.spawnCoins(base, 25, m_seed, m_tileMap);
    m_interactTiles.add({ InteractTileType::Sign, {129, 128},
        "Village Entrance", "Welcome to Greenveil.", false, false });

    int baseTX = static_cast<int>(base.x) / TileMap::TILE_SIZE;
    int baseTY = static_cast<int>(base.y) / TileMap::TILE_SIZE;
    float angle = static_cast<float>(std::rand() % 360) * 3.14159f / 180.f;
    int dist = 50 + std::rand() % 26;
    int lakeX = baseTX + static_cast<int>(std::cos(angle) * dist);
    int lakeY = baseTY + static_cast<int>(std::sin(angle) * dist);
    m_tileMap.forceWaterRegion(lakeX, lakeY, 4 + std::rand() % 5);
}

void Game::run() {
    float accumulator = 0.f;
    while (m_window.isOpen()) {
        float frameTime = m_clock.restart().asSeconds();
        if (frameTime > 0.05f) frameTime = 0.05f;
        accumulator += frameTime;
        processEvents();
        while (accumulator >= FIXED_DT) {
            update(FIXED_DT);
            accumulator -= FIXED_DT;
        }
        render();
    }
}

void Game::processEvents() {
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            m_window.close();

        if (m_gameState == GameState::MainMenu) {
            auto action = m_mainMenu.handleEvent(*event, m_window.getSize());
            if (action == MenuAction::NewGame) {
                SaveSystem::deleteSlot(0);
                SaveSystem::deleteSlot(1);
                SaveSystem::deleteSlot(2);
                m_seed = static_cast<unsigned>(std::time(nullptr));
                m_tileMap.generate(256, 256, m_seed);
                m_bushes.generate(m_tileMap, m_seed);
                m_grassDecor.generate(m_tileMap, m_seed);
                m_player.setPosition({ 128.f * TileMap::TILE_SIZE, 128.f * TileMap::TILE_SIZE });
                m_wallet = Wallet{}; m_wallet.gold = 2; m_wallet.silver = 50;
                m_inventory = Inventory{};
                m_combat = CombatSystem{};
                m_farming = FarmingSystem{};
                m_npcManager = NPCManager{};
                m_interactTiles = InteractTileManager{};
                m_coinPickup.reset();
                m_playSeconds = 0.f;
                m_gameHour = 8.f;
                m_slotBaseMinutes = 0;
                spawnVillage();
                m_camera.snapTo(m_player.getPosition());
                m_gameState = GameState::Playing;
            }
            if (action == MenuAction::Continue) {
                int bestSlot = -1;
                int bestMinutes = -1;
                for (int i = 0; i < 3; ++i) {
                    SaveData sd{};
                    if (SaveSystem::load(i, sd) && sd.hasData && sd.playMinutes > bestMinutes) {
                        bestMinutes = sd.playMinutes;
                        bestSlot = i;
                    }
                }
                if (bestSlot >= 0) {
                    m_currentSlot = bestSlot;
                    SaveData sd{};
                    SaveSystem::load(bestSlot, sd);
                    m_seed = sd.mapSeed;
                    m_tileMap.generate(256, 256, m_seed);
                    m_bushes.generate(m_tileMap, m_seed);
                    m_grassDecor.generate(m_tileMap, m_seed);
                    applyLoadedData(sd, bestSlot);
                    m_gameState = GameState::Playing;
                }
                else {
                    m_seed = static_cast<unsigned>(std::time(nullptr));
                    m_tileMap.generate(256, 256, m_seed);
                    m_bushes.generate(m_tileMap, m_seed);
                    m_grassDecor.generate(m_tileMap, m_seed);
                    m_player.setPosition({ 128.f * TileMap::TILE_SIZE, 128.f * TileMap::TILE_SIZE });
                    m_wallet = Wallet{}; m_wallet.gold = 2; m_wallet.silver = 50;
                    m_inventory = Inventory{};
                    m_combat = CombatSystem{};
                    m_farming = FarmingSystem{};
                    m_npcManager = NPCManager{};
                    m_interactTiles = InteractTileManager{};
                    m_coinPickup.reset();
                    m_playSeconds = 0.f;
                    m_gameHour = 8.f;
                    spawnVillage();
                    m_camera.snapTo(m_player.getPosition());
                    m_gameState = GameState::Playing;
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
                    m_bushes.generate(m_tileMap, m_seed);
                    m_grassDecor.generate(m_tileMap, m_seed);
                    m_player.setPosition({ 128.f * TileMap::TILE_SIZE, 128.f * TileMap::TILE_SIZE });
                    m_wallet = Wallet{}; m_wallet.gold = 2; m_wallet.silver = 50;
                    m_inventory = Inventory{};
                    spawnVillage();
                    m_gameState = GameState::Playing;
                    };
                auto doLoad = [&](int slot) {
                    m_currentSlot = slot;
                    m_seed = m_slotPreviews[slot].mapSeed;
                    m_tileMap.generate(256, 256, m_seed);
                    m_bushes.generate(m_tileMap, m_seed);
                    m_grassDecor.generate(m_tileMap, m_seed);
                    applyLoadedData(m_slotPreviews[slot], slot);
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
                                } //doLoad
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
                if (m_interactPrompt.isVisible()) { m_interactPrompt.hide(); break; }
                if (m_npcManager.isDialogueOpen()) { m_npcManager.closeDialogue(); break; }
                if (m_buildingUI.isOpen()) { m_buildingUI.close(); break; }
                if (m_buildingUI.hasSelected()) { m_buildingUI.clearSelected(); break; }
                if (m_gameState == GameState::SaveMenu) {
                   m_gameState = GameState::Playing; break;
                }
                for (int i = 0; i < 3; ++i) {
                     m_slotPreviews[i] = {};
                     SaveSystem::load(i, m_slotPreviews[i]);
                }
                m_gameState = GameState::SaveMenu;
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
                if (m_interactPrompt.isVisible()) {
                    m_interactPrompt.hide(); break;
                }
                if (m_npcManager.isDialogueOpen()) {
                    m_npcManager.advanceDialogue(
                        m_inventory.countOf(ItemType::Crop),
                        m_npcManager.getMissions().getKillCount());
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
                        m_buildingUI.close(); //Escape
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
                // Num6-Num0 rezervisani za buduce hotbar prosirenje
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

            if (m_gameState == GameState::SaveMenu) {
                SaveAction action = m_saveUI.handleClick(mpos, m_slotPreviews);

                auto doSave = [&](int slot) {
                    SaveData sd = buildSaveData();
                    SaveSystem::save(slot, sd);
                    SaveSystem::saveTiles(slot, m_tileMap);
                    m_slotPreviews[slot] = sd;
                    };
                auto doLoad = [&](int slot) {
                    m_currentSlot = slot;
                    m_seed = m_slotPreviews[slot].mapSeed;
                    m_tileMap.generate(256, 256, m_seed);
                    m_bushes.generate(m_tileMap, m_seed);
                    m_grassDecor.generate(m_tileMap, m_seed);
                    applyLoadedData(m_slotPreviews[slot], slot);
                    m_gameState = GameState::Playing;
                    };
                auto doDelete = [&](int slot) {
                    SaveSystem::deleteSlot(slot);
                    m_slotPreviews[slot] = {};
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

            if (click->button == sf::Mouse::Button::Right) {
                if (m_npcManager.isDialogueOpen()) {
                    m_npcManager.advanceDialogue(
                        m_inventory.countOf(ItemType::Crop),
                        m_npcManager.getMissions().getKillCount());
                }
                else if (m_buildingUI.hasSelected()) {
                    // Desni klik = otkazi build mode
                    m_buildingUI.clearSelected();
                }
                else {
                    // Desni klik = unisti zgradu
                    sf::Vector2f mouseWorld = m_window.mapPixelToCoords(
                        sf::Mouse::getPosition(m_window), m_camera.getView());
                    m_building.tryDestroy(mouseWorld, m_inventory);
                }
            }
            if (click->button == sf::Mouse::Button::Left) {
                // Inventory drag — hotbar uvijek, backpack ako je otvoren
                sf::Vector2f mposUI = {
                    static_cast<float>(click->position.x),
                    static_cast<float>(click->position.y) };
                if (!m_marketUI.isOpen() && !m_skillUI.isVisible())
                    m_inventory.onMousePressed(mposUI);
            }
            if (click->button == sf::Mouse::Button::Left &&
                !m_npcManager.isDialogueOpen()) {
                // Building placement
                if (m_buildingUI.hasSelected() && !m_buildingUI.isOpen()) {
                    sf::Vector2f mouseWorld = m_window.mapPixelToCoords(
                        sf::Mouse::getPosition(m_window), m_camera.getView());
                    int tx = static_cast<int>(mouseWorld.x) / TileMap::TILE_SIZE;
                    int ty = static_cast<int>(mouseWorld.y) / TileMap::TILE_SIZE;
                    m_building.tryPlace(m_buildingUI.getSelected(),
                        { tx, ty }, m_inventory, m_tileMap);
                    // Provjeri je li ostalo materijala
                    const auto& def = m_building.getDef(m_buildingUI.getSelected());
                    if (m_inventory.countOf(def.material) < def.cost)
                        m_buildingUI.clearSelected();
                }
                if (m_player.tryAttack()) {
                    sf::Vector2f mouseWorld = m_window.mapPixelToCoords(
                        sf::Mouse::getPosition(m_window), m_camera.getView());
                    sf::Vector2f dir = mouseWorld - m_player.getPosition();
                    m_player.setFacing(dir);

                    ItemType held = m_inventory.selectedItem();
                    float dmg = m_progression.getAttackDamage();
                    float range = Player::ATTACK_RANGE;

                    if (held == ItemType::Sword) {
                        // Puni damage i range — zuti hitbox ostaje
                        dmg *= 1.0f;
                        range *= 1.0f;
                    }
                    else if (held == ItemType::FishingRod) {
                        // Malo duzi od ruke
                        dmg *= 0.4f;
                        range *= 0.65f;
                    }
                    else {
                        // Ruke/ostalo — mali sivi hitbox
                        dmg *= 0.3f;
                        range *= 0.45f;
                    }

                    bool killed = m_combat.playerAttack(
                        m_player.getPosition(),
                        m_player.getFacing(),
                        range, dmg);
                    if (killed)
                        m_npcManager.getMissions().registerKill();
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
    if (m_gameState == GameState::MainMenu) return;
    if (m_gameOver) return;

    if (m_gameState == GameState::Playing)
        m_playSeconds += dt;

    m_prevHour = m_gameHour;
    bool  isNightTime = (m_gameHour >= 21.f || m_gameHour < 6.f);
    float hourSpeed = isNightTime ? GAME_HOUR_SPEED_NIGHT : GAME_HOUR_SPEED_DAY;
    m_gameHour += hourSpeed * dt;

    if (m_gameHour >= 24.f) {
        m_gameHour -= 24.f;
        m_bossSpawnedThisNight = false;
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
        m_player.update(dt, m_tileMap, m_building, m_bushes);

    // Sinhroniziraj weapon type sa selektovanim itemom
    {
        ItemType held = m_inventory.selectedItem();
        WeaponType wt = WeaponType::Bare;
        if (held == ItemType::Sword)      wt = WeaponType::Sword;
        else if (held == ItemType::FishingRod) wt = WeaponType::Rod;
        m_player.setWeaponType(wt);
    }

    m_camera.follow(m_player.getPosition(), dt);
    m_farming.update(dt, m_tileMap);
    m_building.update(dt);
    m_market.update(dt, m_gameHour);
    m_marketUI.update(dt, m_gameHour, m_market);

    // Fishing update
    m_fishing.update(dt, m_player.getPosition(), m_tileMap);

    m_coinPickup.update(m_player.getPosition());
    int coins = m_coinPickup.pollCollected();
    if (coins > 0) m_wallet.addSilver(coins);


    // Poll uhvacene ribe
    const FishDef* caught = m_fishing.pollCatch();
    if (caught) {
        m_inventory.addItem(caught->type, 1);
        m_progression.addXP(static_cast<int>(caught->rarity * 80.f) + 10);
    }

    float strengthMult = m_dayNight.getEnemyStrengthMult(m_gameHour);
    m_combat.setStrengthMult(strengthMult);
    m_combat.update(dt, m_player.getPosition(), m_tileMap);

    int xp = m_combat.pollXPGained();
    if (xp > 0) m_progression.addXP(xp);

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
        m_npcManager.getMissions().getKillCount(),
        m_inventory.countOf(ItemType::Carp) +
        m_inventory.countOf(ItemType::Trout) +
        m_inventory.countOf(ItemType::Salmon) +
        m_inventory.countOf(ItemType::Catfish) +
        m_inventory.countOf(ItemType::GoldenFish));

    float dmg = m_combat.pollPlayerDamage();
    if (dmg > 0.f && !m_npcManager.isDialogueOpen())
        m_player.takeDamage(dmg);

    if (!m_player.isAlive()) m_gameOver = true;

    m_enemySpawnTimer += dt;
    if (m_enemySpawnTimer >= ENEMY_SPAWN_CD) {
        m_enemySpawnTimer = 0.f;
        bool isNight = m_dayNight.getTimeOfDay(m_gameHour) == TimeOfDay::Night;
        // Čudovišta se spawna SAMO noću
        if (isNight) {
            int r = std::rand() % 3;
            EnemyType type = r == 0 ? EnemyType::Goblin
                : r == 1 ? EnemyType::Skeleton
                : EnemyType::Wolf;
            m_combat.spawnWave(m_player.getPosition(), 5, type);
        }
    }
} //

void Game::render() {
    if (m_gameState == GameState::MainMenu) {
        m_window.clear();
        m_mainMenu.draw(m_window);
        m_window.display();
        return;
    }

    m_window.clear(sf::Color(20, 20, 20));

    m_camera.apply(m_window);
    m_tileMap.draw(m_window);
    m_grassDecor.draw(m_window);
    m_farming.draw(m_window);
    m_bushes.draw(m_window);
    m_building.draw(m_window);
    m_combat.draw(m_window);
    m_npcManager.draw(m_window);
    m_player.draw(m_window);
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

    m_window.display();
}

void Game::deleteSave() { std::remove("savegame.bin"); }