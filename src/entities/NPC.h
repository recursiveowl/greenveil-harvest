#pragma once
#include "pch.h"
#include "world/TileMap.h"

// ========================= ENUMS =========================

enum class NPCType : uint8_t {
    Farmer = 0,
    Merchant,
    Guard,
    Elder,
    COUNT
};

enum class NPCState : uint8_t {
    Idle,
    Walking,
    Working,
    Sleeping,
    Talking
};

// ========================= DIALOGUE =========================

struct DialogueLine {
    std::string text;
    bool        isMission{false};
    int         missionId{-1};
};

// ========================= SCHEDULE =========================

struct NPCSchedule {
    float workStartHour{8.f};
    float workEndHour{18.f};
    float sleepStartHour{22.f};
    float sleepEndHour{6.f};

    sf::Vector2f homePos{0.f, 0.f};
    sf::Vector2f workPos{0.f, 0.f};
};

// ========================= NPC CLASS =========================

class NPC {
public:
    NPC(NPCType type, sf::Vector2f pos, const std::string& name);

    // Core
    void update(float dt, float gameHour, const TileMap& map);
    void draw(sf::RenderWindow& window);

    // ================= INTERACTION =================

    bool tryInteract(sf::Vector2f playerPos);
    void exitDialogue(); // ručno zatvaranje (ESC npr.)

    // ================= DIALOGUE =================

    const DialogueLine& getCurrentDialogue() const;
    void nextDialogue();
    bool hasMoreDialogue() const;
    void resetDialogue();

    bool isTalking() const { return m_state == NPCState::Talking; }
    bool isDialogueFinished() const;

    // ================= TYPING EFFECT =================

    void updateTyping(float dt);
    std::string getVisibleText() const;
    bool isTypingFinished() const;

    // ================= MISSIONS =================

    int  getActiveMission() const { return m_activeMission; }
    void completeMission(int id);

    // ================= GETTERS =================

    sf::Vector2f getPosition() const { return m_pos; }
    NPCType      getType()     const { return m_type; }
    std::string  getName()     const { return m_name; }

    // ================= CONSTANTS =================

    static constexpr float INTERACT_RANGE = 48.f;

private:
    // Core helpers
    void updateSchedule(float gameHour);
    void moveTowards(sf::Vector2f target, float dt, const TileMap& map);
    void loadDialogue();

    // ================= BASIC DATA =================

    NPCType      m_type;
    NPCState     m_state{NPCState::Idle};

    sf::Vector2f m_pos;
    std::string  m_name;
    sf::Color    m_color;
    float        m_size{14.f};

    // ================= SCHEDULE =================

    NPCSchedule m_schedule;

    // ================= DIALOGUE =================

    std::vector<DialogueLine> m_dialogues;
    int                       m_dialogueIdx{0};
    int                       m_activeMission{-1};

    // ================= TYPING SYSTEM =================

    float m_typingSpeed{40.f};      // karaktera u sekundi
    float m_typingTimer{0.f};
    size_t m_visibleChars{0};

    // ================= WANDER =================

    sf::Vector2f m_wanderTarget{0.f, 0.f};
    float        m_wanderTimer{0.f};
    float        m_speed{60.f};

    // ================= ANIMATION =================

    float m_bobTimer{0.f};
};