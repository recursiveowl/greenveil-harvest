#include "entities/NPC.h"
#include <cmath>
#include <cstdlib>

static float dist(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x-b.x, dy = a.y-b.y;
    return std::sqrt(dx*dx+dy*dy);
}
static sf::Vector2f normalize(sf::Vector2f v) {
    float l = std::sqrt(v.x*v.x+v.y*v.y);
    if (l < 0.001f) return {0.f,0.f};
    return {v.x/l, v.y/l};
}

NPC::NPC(NPCType type, sf::Vector2f pos, const std::string& name)
    : m_type(type), m_pos(pos), m_name(name)
{
    m_schedule.homePos = pos;
    m_schedule.workPos = pos + sf::Vector2f{
        static_cast<float>((std::rand()%5-2) * TileMap::TILE_SIZE),
        static_cast<float>((std::rand()%5-2) * TileMap::TILE_SIZE)
    };
    switch (type) {
        case NPCType::Farmer:
            m_color = sf::Color(210, 170, 90);
            m_schedule.workStartHour = 6.f;
            m_schedule.workEndHour   = 17.f;
            break;
        case NPCType::Merchant:
            m_color = sf::Color(90, 160, 210);
            m_schedule.workStartHour = 9.f;
            m_schedule.workEndHour   = 19.f;
            break;
        case NPCType::Elder:
            m_color = sf::Color(200, 160, 200);
            m_schedule.workStartHour = 10.f;
            m_schedule.workEndHour   = 16.f;
            break;
        default: break;
    }
    loadDialogue();
}

void NPC::loadDialogue() {
    m_dialogues.clear();
    switch (m_type) {
        case NPCType::Farmer:
            m_dialogues.push_back({"Good day! The soil looks fertile today."});
            m_dialogues.push_back({"Could you bring me 3 crops? I am not feeling well.", true, 0});
            m_dialogues.push_back({"Thank you in advance, friend!"});
            break;
        case NPCType::Merchant:
            m_dialogues.push_back({"Welcome to my shop, traveller!"});
            m_dialogues.push_back({"I have rare goods for trade."});
            m_dialogues.push_back({"Bring me 5 crops and I will give you something special.", true, 1});
            break;
        case NPCType::Elder:
            m_dialogues.push_back({ "My old bones ache... I could use some fresh fish." });
            m_dialogues.push_back({ "The pond nearby is full of them this time of year." });
            m_dialogues.push_back({ "Bring me 3 fish and I shall reward you well.", true, 3 });
            break;
        default: break;
    }
    m_dialogueIdx = 0;
}

void NPC::completeMission(int id, bool alreadyHasFish) {
    m_activeMission = -1;
    m_dialogueIdx = 0;
    m_dialogues.clear();
    switch (id) {
    case 0:
        m_dialogues.push_back({ "Here you are, Marko!" });
        m_dialogues.push_back({ "Wonderful! These crops will keep me going. Take this, friend." });
        break;
    case 1:
        m_dialogues.push_back({ "Here are your crops, Ana!" });
        m_dialogues.push_back({ "Perfect timing! The traders will be pleased. Here is your cut." });
        break;
    case 3:
        if (alreadyHasFish) {
            m_dialogues.push_back({ "I had some spare fish anyway!", false, -1 });
            m_dialogues.push_back({ "Ahh, fresh fish... My bones thank you, child. Bless you.", false, -1 });
        }
        else {
            m_dialogues.push_back({ "Fresh fish for you, Elder Vid!", false, -1 });
            m_dialogues.push_back({ "Ahh, fresh fish... My bones thank you, child. Bless you.", false, -1 });
        }
        break;
    }
}

void NPC::update(float dt, float gameHour, const TileMap& map) {
    m_bobTimer += dt * 3.f;
    if (m_state == NPCState::Talking) return;

    bool isSleepTime = gameHour >= m_schedule.sleepStartHour ||
                       gameHour < m_schedule.sleepEndHour;
    bool isWorkTime  = gameHour >= m_schedule.workStartHour &&
                       gameHour < m_schedule.workEndHour;

    sf::Vector2f target = m_schedule.homePos;

    if (isSleepTime) {
        m_state = NPCState::Sleeping;
        target  = m_schedule.homePos;
    } else if (isWorkTime) {
        m_state = NPCState::Working;
        m_wanderTimer -= dt;
        if (m_wanderTimer <= 0.f) {
            m_wanderTimer  = 3.f + static_cast<float>(std::rand()%4);
            float angle    = static_cast<float>(std::rand()%360) * 3.14159f/180.f;
            float radius   = 32.f + std::rand()%64;
            m_wanderTarget = m_schedule.workPos + sf::Vector2f{
                std::cos(angle)*radius, std::sin(angle)*radius};
        }
        target = m_wanderTarget;
    } else {
        m_state = NPCState::Idle;
        m_wanderTimer -= dt;
        if (m_wanderTimer <= 0.f) {
            m_wanderTimer  = 4.f + static_cast<float>(std::rand()%4);
            float angle    = static_cast<float>(std::rand()%360) * 3.14159f/180.f;
            float radius   = 48.f + std::rand()%48;
            m_wanderTarget = m_schedule.homePos + sf::Vector2f{
                std::cos(angle)*radius, std::sin(angle)*radius};
        }
        target = m_wanderTarget;
    }

    float d = dist(m_pos, target);
    if (d > 8.f) {
        sf::Vector2f dir    = normalize(target - m_pos);
        sf::Vector2f newPos = m_pos + dir * m_speed * dt;
        int tx = static_cast<int>(newPos.x) / TileMap::TILE_SIZE;
        int ty = static_cast<int>(newPos.y) / TileMap::TILE_SIZE;
        if (!map.getTile(tx, ty).solid)
            m_pos = newPos;
    }
}

bool NPC::tryInteract(sf::Vector2f playerPos) {
    if (dist(m_pos, playerPos) > INTERACT_RANGE) return false;
    m_state       = NPCState::Talking;
    m_dialogueIdx = 0;
    return true;
}

const DialogueLine& NPC::getCurrentDialogue() const {
    static DialogueLine dummy{"...", false, -1};
    if (m_dialogueIdx < static_cast<int>(m_dialogues.size()))
        return m_dialogues[m_dialogueIdx];
    return dummy;
}

void NPC::nextDialogue() {
    ++m_dialogueIdx;
}

bool NPC::hasMoreDialogue() const {
    return m_dialogueIdx < static_cast<int>(m_dialogues.size());
}

void NPC::resetDialogue() {
    m_dialogueIdx = 0;
    m_state       = NPCState::Idle;
}

void NPC::draw(sf::RenderWindow& window) {
    float bob = m_state == NPCState::Sleeping ? 0.f :
                std::sin(m_bobTimer) * 2.f;
    sf::Vector2f drawPos = m_pos + sf::Vector2f{0.f, bob};

    sf::CircleShape body(m_size);
    body.setOrigin({m_size, m_size});
    body.setPosition(drawPos);
    body.setFillColor(m_state == NPCState::Sleeping
        ? sf::Color(m_color.r/2, m_color.g/2, m_color.b/2)
        : m_color);
    body.setOutlineThickness(2.f);
    body.setOutlineColor(sf::Color(0,0,0,160));
    window.draw(body);

    sf::CircleShape head(m_size * 0.6f);
    head.setOrigin({m_size*0.6f, m_size*0.6f});
    head.setPosition(drawPos + sf::Vector2f{0.f, -m_size*1.4f});
    head.setFillColor(sf::Color(255, 220, 170));
    head.setOutlineThickness(1.5f);
    head.setOutlineColor(sf::Color(0,0,0,120));
    window.draw(head);

    if (!m_dialogues.empty() && m_activeMission == -1 &&
        m_state != NPCState::Sleeping && m_state != NPCState::Talking) {
        sf::RectangleShape excl({4.f, 12.f});
        excl.setOrigin({2.f, 12.f});
        excl.setPosition(drawPos + sf::Vector2f{0.f, -m_size*2.4f});
        excl.setFillColor(sf::Color(255, 220, 0));
        window.draw(excl);
        sf::CircleShape dot(2.5f);
        dot.setOrigin({2.5f, 2.5f});
        dot.setPosition(drawPos + sf::Vector2f{0.f, -m_size*2.4f + 14.f});
        dot.setFillColor(sf::Color(255, 220, 0));
        window.draw(dot);
    }
}
