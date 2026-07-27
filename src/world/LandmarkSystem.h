#pragma once
#include "pch.h"

enum class LandmarkRole : uint8_t { Empty, Player, NPC, ForSale };

struct RanchSlot {
    sf::Vector2i tilePos;          // gornji-lijevi ugao slota
    LandmarkRole role{ LandmarkRole::Empty };
    int          npcIndex{ -1 };   // valjano samo ako role == NPC
};

class LandmarkSystem {
public:
    static constexpr int SLOT_WIDTH = 12;
    static constexpr int SLOT_HEIGHT = 10;
    static constexpr int SLOT_COUNT = 10;

    LandmarkSystem();

    // Random dodjeljuje uloge (Player/NPC/ForSale/Empty) fiksnim pozicijama slotova.
    // Pozicije slotova se NIKAD ne mijenjaju - samo uloge.
    void assignRoles(unsigned seed, int npcCount);

    const std::vector<RanchSlot>& getSlots() const { return m_slots; }

    sf::Vector2i getMarketPos()  const { return m_marketPos; }
    sf::Vector2i getMarketSize() const { return { 8, 8 }; }

    // Pronadje slot sa role==Player. Ako ne postoji (nije pozvan assignRoles), vraca {0,0}.
    sf::Vector2i getPlayerSlotPos() const;

    // Pronadje slot za dati NPC indeks (0,1,2,...). Ako ne postoji, vraca {0,0}.
    sf::Vector2i getNPCSlotPos(int npcIndex) const;

    void drawMarketPlaceholder(sf::RenderWindow& window) const;

private:
    std::vector<RanchSlot> m_slots;
    sf::Vector2i            m_marketPos{ 130, 118 };
};