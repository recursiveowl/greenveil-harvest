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

void NPCManager::draw(sf::RenderWindow& window) {
    for (auto& npc : m_npcs)
        npc.draw(window);

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
    if (!m_activeSpeaker || !m_dialogueBox.isVisible()) return;

    const auto& line = m_activeSpeaker->getCurrentDialogue();

    // Zapamti misiju — aktivirat će se tek kad se dijalog zatvori
    if (line.isMission && line.missionId >= 0) {
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
    if (m_activeSpeaker) {
        m_activeSpeaker->resetDialogue();
        m_activeSpeaker = nullptr;
    }
    m_dialogueBox.hide();

    // Aktiviraj misiju tek sada — dijalog je završen
    if (m_pendingMissionId >= 0) {
        m_missions.activateMission(m_pendingMissionId, m_pendingMissionNpcIdx);
        m_pendingMissionId = -1;
        m_pendingMissionNpcIdx = -1;
    }
}

bool NPCManager::canTurnIn(sf::Vector2f playerPos) const {
    const Mission* active = m_missions.getActiveMission();
    if (!active) return false;
    if (active->status != MissionStatus::ReadyToTurn) return false;

    int npcIdx = active->npcId;
    if (npcIdx < 0 || npcIdx >= static_cast<int>(m_npcs.size())) return false;
    return dist(m_npcs[npcIdx].getPosition(), playerPos) <= NPC::INTERACT_RANGE;
}

bool NPCManager::tryTurnIn(sf::Vector2f playerPos) {
    const Mission* active = m_missions.getActiveMission();
    if (!active) return false;
    if (!canTurnIn(playerPos)) return false;
    return m_missions.tryTurnIn(active->id);
}

void NPCManager::drawUI(sf::RenderWindow& window) {
    m_dialogueBox.draw(window);
}