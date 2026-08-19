#pragma once
#include "pch.h"
#include "entities/NPC.h"
#include "systems/MissionSystem.h"
#include "ui/DialogueBox.h"
#include "core/RenderQueue.h"

class NPCManager {
public:
    NPCManager();

    void spawnNPC(NPCType type, sf::Vector2f pos, const std::string& name);
    void update(float dt, float gameHour, const TileMap& map);
    void collectRenderItems(std::vector<RenderItem>& out);
    void drawDebugRanges(sf::RenderWindow& window) const;
    void drawUI(sf::RenderWindow& window);

    bool tryInteract(sf::Vector2f playerPos, float gameHour);
    void advanceDialogue(int playerCrops, int playerKills);
    void closeDialogue();

    bool canTurnIn(sf::Vector2f playerPos) const;
    bool tryTurnIn(sf::Vector2f playerPos, int playerFishCount = 0);

    bool           isDialogueOpen() const { return m_dialogueBox.isVisible(); }
    MissionSystem& getMissions() { return m_missions; }
    const MissionSystem& getMissions()    const { return m_missions; }

    int getNPCCount() const { return static_cast<int>(m_npcs.size()); }
    sf::Vector2f getElderPos() const {
        if (m_npcs.size() > 2) return m_npcs[2].getPosition();
        return { 0.f, 0.f };
    }

private:
    std::vector<NPC> m_npcs;
    NPC* m_activeSpeaker{ nullptr };
    DialogueBox      m_dialogueBox;
    MissionSystem    m_missions;

    // Misija se aktivira tek kad se dijalog u potpunosti zavrsi
    int              m_pendingMissionId{ -1 };
    int              m_pendingMissionNpcIdx{ -1 };

    // true kada dijalog sadrzi samo poruku "vec imas misiju" — sljedeci klik ga zatvara
    bool             m_pendingCloseOnly{ false };

    bool m_turnInJustHappened{ false };
};