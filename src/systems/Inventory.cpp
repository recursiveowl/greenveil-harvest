#include "systems/Inventory.h"

Inventory::Inventory() {
    // Pocni sa 5 sjemena
    slots[0] = {ItemType::Seed, 5};
}

bool Inventory::addItem(ItemType type, int count) {
    // Pokušaj stack na postojeci slot
    for (auto& slot : slots) {
        if (slot.type == type) {
            slot.count += count;
            return true;
        }
    }
    // Novi slot
    for (auto& slot : slots) {
        if (slot.type == ItemType::None) {
            slot = {type, count};
            return true;
        }
    }
    return false; // Inventar pun
}

bool Inventory::removeItem(ItemType type, int count) {
    for (auto& slot : slots) {
        if (slot.type == type && slot.count >= count) {
            slot.count -= count;
            if (slot.count == 0) slot = {};
            return true;
        }
    }
    return false;
}

int Inventory::countOf(ItemType type) const {
    for (auto& slot : slots)
        if (slot.type == type) return slot.count;
    return 0;
}

void Inventory::draw(sf::RenderWindow& window) {
    const float slotSize = 48.f;
    const float padding  = 6.f;
    sf::Vector2u winSize = window.getSize();

    float totalW = SLOTS * (slotSize + padding) - padding;
    float startX = (winSize.x - totalW) * 0.5f;
    float startY = winSize.y - slotSize - 16.f;

    for (int i = 0; i < SLOTS; ++i) {
        float x = startX + i * (slotSize + padding);

        // Slot background
        sf::RectangleShape bg({slotSize, slotSize});
        bg.setPosition({x, startY});
        bg.setFillColor(i == selectedSlot
            ? sf::Color(80, 80, 80, 220)
            : sf::Color(40, 40, 40, 180));
        bg.setOutlineThickness(2.f);
        bg.setOutlineColor(i == selectedSlot
            ? sf::Color(255, 200, 0)
            : sf::Color(100, 100, 100));
        window.draw(bg);

        // Item u slotu
        if (slots[i].type != ItemType::None) {
            sf::Color itemColor;
            switch (slots[i].type) {
                case ItemType::Seed:  itemColor = sf::Color(180, 140, 60);  break;
                case ItemType::Crop:  itemColor = sf::Color(240, 200,  0);  break;
                case ItemType::Wood:  itemColor = sf::Color(139,  90, 43);  break;
                case ItemType::Stone: itemColor = sf::Color(128, 128, 128); break;
                default:              itemColor = sf::Color::White;
            }
            sf::RectangleShape item({slotSize - 12.f, slotSize - 12.f});
            item.setPosition({x + 6.f, startY + 6.f});
            item.setFillColor(itemColor);
            window.draw(item);
        }
    }
}
