#include "core/Game.h"
#include <ctime>
#include <cstdlib>
#include <cmath>

static constexpr float FIXED_DT = 1.f / 60.f;
static constexpr float ENEMY_SPAWN_CD = 15.f;
static constexpr float GAME_HOUR_SPEED_DAY = 1.0f / 60.f;
static constexpr float GAME_HOUR_SPEED_NIGHT = 1.8f / 60.f;

Game::Game()
    : m_window(sf::VideoMode({ WINDOW_W, WINDOW_H }), "Greenveil Harvest")
    , m_camera(WINDOW_W, WINDOW_H)
    , m_tileMap(m_assets)
    , m_player(m_assets)
{
    m_window.setVerticalSyncEnabled(true);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    unsigned seed = static_cast<unsigned>(std::time(nullptr));
    m_tileMap.generate(256, 256, seed);
    m_player.setPosition({
        static_cast<float>(128 * TileMap::TILE_SIZE),
        static_cast<float>(128 * TileMap::TILE_SIZE)
        });

    // Pocetni novac
    m_wallet.gold = 2;
    m_wallet.silver = 50;

    // Pocetni equipment
    m_inventory.addItem(ItemType::FishingRod, 1);
    m_inventory.addItem(ItemType::Sword, 1);

    spawnVillage();
    loadGame();
}

void Game::applyProgressionStats() {}

void Game::saveGame() {
    SaveData sd;
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

    const auto& missions = m_npcManager.getMissions().getAllMissions();
    for (int i = 0; i < std::min(4, static_cast<int>(missions.size())); ++i)
        sd.missionStatuses[i] = static_cast<int>(missions[i].status);

    SaveSystem::save("savegame.bin", sd);
}

void Game::loadGame() {
    SaveData sd;
    if (!SaveSystem::load("savegame.bin", sd)) {
        // Nema save fajla ili je format promijenjen — koristi defaulte
        return;
    }

    m_player.setPosition({ sd.playerX, sd.playerY });
    m_gameHour = sd.gameHour;

    m_wallet = Wallet{};
    m_wallet.addSilver(sd.coins);

    for (int i = 0; i < Inventory::SLOTS; ++i) {
        m_inventory.slots[i].type = static_cast<ItemType>(sd.itemTypes[i]);
        m_inventory.slots[i].count = sd.itemCounts[i];
    }
}

void Game::spawnVillage() {
    sf::Vector2f base = m_player.getPosition();
    m_npcManager.spawnNPC(NPCType::Farmer, base + sf::Vector2f{ 160.f,  80.f }, "Marko");
    m_npcManager.spawnNPC(NPCType::Merchant, base + sf::Vector2f{ -120.f,  60.f }, "Ana");
    m_npcManager.spawnNPC(NPCType::Guard, base + sf::Vector2f{ 200.f, -90.f }, "Boris");
    m_npcManager.spawnNPC(NPCType::Elder, base + sf::Vector2f{ -80.f,-140.f }, "Elder Vid");
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

        if (const auto* k = event->getIf<sf::Event::KeyPressed>()) {

            if (m_marketUI.isOpen()) {
                if (k->code == sf::Keyboard::Key::M ||
                    k->code == sf::Keyboard::Key::Escape)
                    m_marketUI.close();
                break;
            }

            if (m_skillUI.isVisible()) {
                if (k->code == sf::Keyboard::Key::C ||
                    k->code == sf::Keyboard::Key::Escape)
                    m_skillUI.toggle();
                break;
            }

            switch (k->code) {
            case sf::Keyboard::Key::Escape:
                if (m_npcManager.isDialogueOpen())
                    m_npcManager.closeDialogue();
                else if (m_buildingUI.isOpen())
                    m_buildingUI.close();
                else if (m_buildingUI.hasSelected())
                    m_buildingUI.clearSelected();
                else
                    m_window.close();
                break;

            case sf::Keyboard::Key::C:
                if (m_marketUI.isOpen()) m_marketUI.close();
                if (m_inventory.isBackpackOpen()) m_inventory.toggleBackpack();
                m_skillUI.toggle();
                break;

            case sf::Keyboard::Key::M:
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
                if (m_npcManager.isDialogueOpen()) {
                    m_npcManager.advanceDialogue(
                        m_inventory.countOf(ItemType::Crop),
                        m_npcManager.getMissions().getKillCount());
                }
                else if (m_inventory.selectedItem() == ItemType::FishingRod) {
                    // Fishing action
                    m_fishing.onActionPressed(m_player.getPosition(),
                        m_player.getFacing());
                }
                else if (!m_npcManager.tryInteract(
                    m_player.getPosition(), m_gameHour)) {
                    if (m_inventory.countOf(ItemType::Seed) > 0)
                        if (m_farming.tryPlant(
                            m_player.getPosition(), m_tileMap))
                            m_inventory.removeItem(ItemType::Seed);
                }
                break;

            case sf::Keyboard::Key::E:
                if (!m_npcManager.isDialogueOpen())
                    m_farming.tryTill(m_player.getPosition(), m_tileMap);
                break;

            case sf::Keyboard::Key::R:
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

            case sf::Keyboard::Key::N:
                if (!m_npcManager.isDialogueOpen()) {
                    if (m_npcManager.tryTurnIn(m_player.getPosition())) {
                        // Ukloni potrebne cropove iz inventara
                        for (auto& m : m_npcManager.getMissions().getAllMissions())
                            if (m.status == MissionStatus::Completed &&
                                m.requireCrops > 0) {
                                m_inventory.removeItem(ItemType::Crop, m.requireCrops);
                                break;
                            }
                        // Isplati nagradu direktno u wallet
                        int reward = m_npcManager.getMissions().getLastRewardSilver();
                        if (reward > 0)
                            m_wallet.addSilver(reward);
                    }
                }
                break;

            case sf::Keyboard::Key::B:
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

            case sf::Keyboard::Key::Tab:
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

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            sf::Vector2f mpos = {
                static_cast<float>(click->position.x),
                static_cast<float>(click->position.y) };

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
                m_marketUI.handleScroll(scroll->delta);
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
    if (m_gameOver) return;
    // SkillUI i MarketUI vise NE pauziraju igru

    m_prevHour = m_gameHour;
    bool  isNightTime = (m_gameHour >= 21.f || m_gameHour < 6.f);
    float hourSpeed = isNightTime ? GAME_HOUR_SPEED_NIGHT : GAME_HOUR_SPEED_DAY;
    m_gameHour += hourSpeed * dt;

    if (m_gameHour >= 24.f) {
        m_gameHour -= 24.f;
        m_bossSpawnedThisNight = false;
    }

    while (m_progression.levelUp()) {}

    if (!m_npcManager.isDialogueOpen())
        m_player.update(dt, m_tileMap, m_building);

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
    m_npcManager.getMissions().update(
        m_inventory.countOf(ItemType::Crop),
        m_npcManager.getMissions().getKillCount());
    m_npcManager.getMissions().updateExplore(m_player.getPosition());

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
}

void Game::render() {
    m_window.clear(sf::Color(20, 20, 20));

    m_camera.apply(m_window);
    m_tileMap.draw(m_window);
    m_farming.draw(m_window);
    m_building.draw(m_window);
    m_combat.draw(m_window);
    m_npcManager.draw(m_window);
    m_player.draw(m_window);

    // Building preview — world space
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

    // Bobber — world space
    if (m_fishing.isActive())
        m_fishingUI.drawBobber(m_window,
            m_fishing.getBobber(),
            m_fishing.getCastOrigin(),
            m_fishing.getState());

    m_camera.applyUI(m_window);
    m_dayNight.draw(m_window, m_gameHour);

    m_inventory.draw(m_window);
    m_hud.draw(m_window, m_player.getHp(),
        m_player.getMaxHp(), m_gameHour, m_wallet);
    // Fishing UI
    bool canFish = m_fishing.canFish(m_player.getPosition(),
        m_tileMap, m_inventory);
    m_fishingUI.draw(m_window, m_fishing, canFish);

    m_npcManager.drawUI(m_window);
    m_objectiveUI.draw(m_window, m_npcManager.getMissions(),
        m_npcManager.canTurnIn(m_player.getPosition()));
    m_skillUI.draw(m_window, m_progression);
    m_buildingUI.draw(m_window, m_building, m_inventory);
    m_marketUI.draw(m_window, m_market, m_inventory, m_wallet, m_gameHour);

    if (m_gameOver) {
        sf::RectangleShape overlay({
            static_cast<float>(WINDOW_W),
            static_cast<float>(WINDOW_H) });
        overlay.setFillColor(sf::Color(0, 0, 0, 160));
        m_window.draw(overlay);
    }

    m_window.display();
}