#include "systems/NPCManager.h"
#include <cmath>

static float dist(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

NPCManager::NPCManager() {}

void NPCManager::spawnNPC(NPCType type, sf::Vector2f pos, const std::string& name) {
    m_npcs.emplace_back(type, pos, name);
}

void NPCManager::update(float dt, float gameHour, const TileMap& map) {
    for (auto& npc : m_npcs)
        npc.update(dt, gameHour, map);
}

void NPCManager::collectRenderItems(std::vector<RenderItem>& out) {
    for (auto& npc : m_npcs) {
        float sortY = npc.getPosition().y;
        out.push_back({ sortY, [&npc](sf::RenderWindow& w) { npc.draw(w); } });
    }
}

void NPCManager::drawDebugRanges(sf::RenderWindow& window) const {
    for (auto& npc : m_npcs) {
        sf::CircleShape radius(NPC::INTERACT_RANGE);
        radius.setOrigin({ NPC::INTERACT_RANGE, NPC::INTERACT_RANGE });
        radius.setPosition(npc.getPosition());
        radius.setFillColor(sf::Color::Transparent);
        radius.setOutlineThickness(1.5f);
        radius.setOutlineColor(sf::Color(255, 255, 0, 80));
        window.draw(radius);
    }
}

bool NPCManager::tryInteract(sf::Vector2f playerPos, float gameHour) {
    if (m_activeSpeaker) return false;

    for (auto& npc : m_npcs) {
        bool isSleeping = (gameHour >= 22.f || gameHour < 6.f);
        if (isSleeping) continue;

        if (npc.tryInteract(playerPos)) {
            m_activeSpeaker = &npc;
            m_dialogueBox.show(npc.getName(), npc.getCurrentDialogue().text);
            return true;
        }
    }
    return false;
}

void NPCManager::advanceDialogue(int playerCrops, int playerKills) {
    if (m_turnInJustHappened) {
        m_turnInJustHappened = false;
        return;
    }
    printf("advanceDialogue: activeSpeaker=%p visible=%d pendingClose=%d\n",
        m_activeSpeaker, m_dialogueBox.isVisible(), m_pendingCloseOnly);
    if (!m_activeSpeaker || !m_dialogueBox.isVisible()) return;

    // Ako je ovo samo poruka "vec imas misiju" — sljedeci klik je zatvara
    if (m_pendingCloseOnly) {
        closeDialogue();
        return;
    }

    const auto& line = m_activeSpeaker->getCurrentDialogue();

    // Ako je ova linija misija — provjeri odmah moze li je uzeti
    if (line.isMission && line.missionId >= 0) {
        if (m_missions.hasActiveMission()) {
            // Igrac vec ima aktivnu misiju — prikaži poruku i zatvori dijalog
            m_dialogueBox.show(
                m_activeSpeaker->getName(),
                "Wait... You are already on another mission, complete it first!");
            m_activeSpeaker->resetDialogue();
            m_activeSpeaker = nullptr;
            m_pendingMissionId = -1;
            m_pendingMissionNpcIdx = -1;
            m_pendingCloseOnly = true;
            return;
        }

        // Zapamti misiju — aktivirat ce se tek kad se dijalog zatvori
        m_pendingMissionId = line.missionId;
        m_pendingMissionNpcIdx = -1;
        for (int i = 0; i < static_cast<int>(m_npcs.size()); ++i)
            if (&m_npcs[i] == m_activeSpeaker) { m_pendingMissionNpcIdx = i; break; }
    }

    m_activeSpeaker->nextDialogue();

    if (m_activeSpeaker->hasMoreDialogue()) {
        m_dialogueBox.show(
            m_activeSpeaker->getName(),
            m_activeSpeaker->getCurrentDialogue().text);
    }
    else {
        closeDialogue();
    }
}

void NPCManager::closeDialogue() {
    // Ako je samo poruka "vec imas misiju" — samo sakrij, ne aktiviraj nista
    if (m_pendingCloseOnly) {
        m_dialogueBox.hide();
        m_pendingCloseOnly = false;
        return;
    }

    if (m_activeSpeaker) {
        m_activeSpeaker->resetDialogue();
        m_activeSpeaker = nullptr;
    }
    m_dialogueBox.hide();

    // Aktiviraj misiju tek sada — dijalog je zavrsen
    if (m_pendingMissionId >= 0) {
        m_missions.activateMission(m_pendingMissionId, m_pendingMissionNpcIdx);
        m_pendingMissionId = -1;
        m_pendingMissionNpcIdx = -1;
    }
}

bool NPCManager::canTurnIn(sf::Vector2f playerPos) const {
    const Mission* active = m_missions.getActiveMission();
    printf("canTurnIn: active=%p status=%d\n", active, active ? (int)active->status : -1);
    if (!active) return false;
    if (active->status != MissionStatus::ReadyToTurn) return false;

    int npcIdx = active->npcId;
    if (npcIdx < 0 || npcIdx >= static_cast<int>(m_npcs.size())) return false;
    float d = dist(m_npcs[npcIdx].getPosition(), playerPos);
    printf("canTurnIn: npcIdx=%d dist=%f range=%f\n", npcIdx, d, NPC::INTERACT_RANGE);
    return d <= NPC::INTERACT_RANGE;
}

bool NPCManager::tryTurnIn(sf::Vector2f playerPos, int playerFishCount) {
    const Mission* active = m_missions.getActiveMission();
    if (!active) return false;
    if (!canTurnIn(playerPos)) return false;

    int npcIdx = active->npcId;
    int missionId = active->id;
    int requireCrops = active->requireCrops;
    bool alreadyHas = (missionId == 3) && (playerFishCount >= requireCrops);

    if (!m_missions.tryTurnIn(active->id)) return false;

    if (npcIdx >= 0 && npcIdx < (int)m_npcs.size()) {
        if (m_activeSpeaker) {
            m_activeSpeaker->resetDialogue();
            m_activeSpeaker = nullptr;
        }
        m_dialogueBox.hide();
        m_npcs[npcIdx].completeMission(missionId, alreadyHas);
        m_pendingCloseOnly = false;
        m_pendingMissionId = -1;
        m_pendingMissionNpcIdx = -1;
        m_activeSpeaker = &m_npcs[npcIdx];
        m_dialogueBox.show("You", m_activeSpeaker->getCurrentDialogue().text);
        m_turnInJustHappened = true; // ← ovdje
    }
    return true;
}

void NPCManager::drawUI(sf::RenderWindow& window) {
    m_dialogueBox.draw(window);
}