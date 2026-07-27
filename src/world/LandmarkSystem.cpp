#include "world/LandmarkSystem.h"
#include <random>
#include <algorithm>
#include <numeric>

LandmarkSystem::LandmarkSystem() {
    // 10 fiksnih pozicija ocitanih sa referentne skice (tile koordinate na 256x256 mapi).
    // Redoslijed nije bitan - assignRoles() radi shuffle nad indeksima.
    static const sf::Vector2i kFixedPositions[SLOT_COUNT] = {
        { 81,  13 },
        { 159, 13 },
        { 218, 31 },
        { 26,  64 },
        { 220, 85 },
        { 26,  154 },
        { 228, 148 },
        { 51,  220 },
        { 138, 220 },
        { 210, 233 },
    };

    m_slots.reserve(SLOT_COUNT);
    for (int i = 0; i < SLOT_COUNT; ++i) {
        RanchSlot slot;
        slot.tilePos = kFixedPositions[i];
        slot.role = LandmarkRole::Empty;
        m_slots.push_back(slot);
    }
}

void LandmarkSystem::assignRoles(unsigned seed, int npcCount) {
    // Reset svih uloga prije nove dodjele
    for (auto& slot : m_slots) {
        slot.role = LandmarkRole::Empty;
        slot.npcIndex = -1;
    }

    std::vector<int> order(m_slots.size());
    std::iota(order.begin(), order.end(), 0);

    std::mt19937 rng(seed + 9001); // posebna so za landmark dodjelu
    std::shuffle(order.begin(), order.end(), rng);

    int idx = 0;

    // 1 slot -> Player
    if (idx < static_cast<int>(order.size())) {
        m_slots[order[idx]].role = LandmarkRole::Player;
        ++idx;
    }

    // sledecih npcCount slotova -> NPC (sa redom npcIndex 0,1,2,...)
    for (int n = 0; n < npcCount && idx < static_cast<int>(order.size()); ++n, ++idx) {
        m_slots[order[idx]].role = LandmarkRole::NPC;
        m_slots[order[idx]].npcIndex = n;
    }

    // ostatak -> ForSale
    for (; idx < static_cast<int>(order.size()); ++idx) {
        m_slots[order[idx]].role = LandmarkRole::ForSale;
    }
}

sf::Vector2i LandmarkSystem::getPlayerSlotPos() const {
    for (const auto& slot : m_slots)
        if (slot.role == LandmarkRole::Player) return slot.tilePos;
    return { 0, 0 };
}

sf::Vector2i LandmarkSystem::getNPCSlotPos(int npcIndex) const {
    for (const auto& slot : m_slots)
        if (slot.role == LandmarkRole::NPC && slot.npcIndex == npcIndex) return slot.tilePos;
    return { 0, 0 };
}

void LandmarkSystem::drawMarketPlaceholder(sf::RenderWindow& window) const {
    constexpr float TS = 32.f; // TileMap::TILE_SIZE, izbjegavamo include radi jednostavnosti
    sf::Vector2i size = getMarketSize();

    sf::RectangleShape marker(
        { static_cast<float>(size.x) * TS, static_cast<float>(size.y) * TS });
    marker.setPosition({ m_marketPos.x * TS, m_marketPos.y * TS });
    marker.setFillColor(sf::Color(80, 140, 220, 180));
    marker.setOutlineThickness(2.f);
    marker.setOutlineColor(sf::Color(40, 90, 170));
    window.draw(marker);
}