#pragma once
#include "pch.h"

enum class ItemType : uint8_t {
    None = 0,
    Seed,
    Crop,
    Wood,
    Stone
};

struct Item {
    ItemType type  = ItemType::None;
    int      count = 0;
};

class Inventory {
public:
    static constexpr int SLOTS = 10;

    Inventory();
    bool addItem(ItemType type, int count = 1);
    bool removeItem(ItemType type, int count = 1);
    int  countOf(ItemType type) const;

    void draw(sf::RenderWindow& window);

    int          selectedSlot = 0;
    std::array<Item, SLOTS> slots{};
};
