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
    ItemType type = ItemType::None;
    int      count = 0;
};

class Inventory {
public:
    static constexpr int HOTBAR_SLOTS = 5;
    static constexpr int BACKPACK_SLOTS = 36;  // 6x6 grid, kao SDV
    static constexpr int SLOTS = HOTBAR_SLOTS + BACKPACK_SLOTS;
    static constexpr int TOTAL_SLOTS = SLOTS;

    Inventory();

    bool addItem(ItemType type, int count = 1);
    bool removeItem(ItemType type, int count = 1);
    int  countOf(ItemType type) const;

    void toggleBackpack() { m_backpackOpen = !m_backpackOpen; }
    bool isBackpackOpen() const { return m_backpackOpen; }

    // Drag & Drop
    void onMousePressed(sf::Vector2f mouse);
    void onMouseReleased(sf::Vector2f mouse);
    void onMouseMoved(sf::Vector2f mouse);

    void draw(sf::RenderWindow& window);

    int selectedSlot{ 0 };
    std::array<Item, SLOTS> slots{};

private:
    // Layout helpers
    sf::FloatRect hotbarSlotRect(int i) const;
    sf::FloatRect backpackSlotRect(int i) const;
    int  hitTestHotbar(sf::Vector2f p) const; // -1 = miss
    int  hitTestBackpack(sf::Vector2f p) const; // -1 = miss
    // -1 = none, 0..HOTBAR_SLOTS-1 = hotbar, HOTBAR_SLOTS.. = backpack
    int  hitTestAll(sf::Vector2f p) const;

    void drawHotbar(sf::RenderWindow& w);
    void drawBackpack(sf::RenderWindow& w);
    void drawDragItem(sf::RenderWindow& w);

    static sf::Color     itemColor(ItemType t);
    static std::string   itemName(ItemType t);
    static std::string   itemDesc(ItemType t);
    void drawSlot(sf::RenderWindow& w, sf::FloatRect r,
        const Item& item, bool selected,
        bool highlighted, bool isDragSource);

    bool        m_backpackOpen{ false };
    sf::Font    m_font;

    // Drag state
    bool         m_dragging{ false };
    int          m_dragSrcIdx{ -1 };    // slot index
    sf::Vector2f m_dragPos;           // current mouse pos
    Item         m_dragItem;          // item being dragged

    // Backpack panel bounds (computed each frame)
    mutable float m_bpPx{ 0.f }, m_bpPy{ 0.f };
    mutable float m_bpW{ 0.f }, m_bpH{ 0.f };

    // Window size cache
    mutable sf::Vector2u m_ws{ 1280u, 720u };

    // Constants
    static constexpr float SLOT_SIZE = 44.f;
    static constexpr float SLOT_PAD = 5.f;
    static constexpr int   BP_COLS = 6;
    static constexpr int   BP_ROWS = 6;
    static constexpr float HEADER_H = 40.f;
    static constexpr float FOOTER_H = 20.f;
};