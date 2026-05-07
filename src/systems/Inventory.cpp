#include "systems/Inventory.h"
#include <cmath>
#include <algorithm>

// -------------------------------------------------------
//  Static helpers
// -------------------------------------------------------
sf::Color Inventory::itemColor(ItemType t) {
    switch (t) {
    case ItemType::Seed:       return sf::Color(200, 160, 60);
    case ItemType::Crop:       return sf::Color(240, 210, 40);
    case ItemType::Wood:       return sf::Color(160, 100, 45);
    case ItemType::Stone:      return sf::Color(150, 150, 160);
    case ItemType::FishingRod: return sf::Color(80, 160, 220);
    case ItemType::Sword:      return sf::Color(180, 180, 220);
    case ItemType::Hoe:        return sf::Color(180, 170, 210);
    case ItemType::Carp:       return sf::Color(80, 160, 100);
    case ItemType::Trout:      return sf::Color(60, 120, 200);
    case ItemType::Salmon:     return sf::Color(220, 100, 80);
    case ItemType::Catfish:    return sf::Color(100, 80, 140);
    case ItemType::GoldenFish: return sf::Color(255, 200, 0);
    default:                   return sf::Color(60, 60, 60);
    }
}
std::string Inventory::itemName(ItemType t) {
    switch (t) {
    case ItemType::Seed:       return "Seed";
    case ItemType::Crop:       return "Crop";
    case ItemType::Wood:       return "Wood";
    case ItemType::Stone:      return "Stone";
    case ItemType::FishingRod: return "Fishing Rod";
    case ItemType::Sword:      return "Sword";
    case ItemType::Hoe:        return "Hoe";
    case ItemType::Carp:       return "Carp";
    case ItemType::Trout:      return "Trout";
    case ItemType::Salmon:     return "Salmon";
    case ItemType::Catfish:    return "Catfish";
    case ItemType::GoldenFish: return "Golden Fish";
    default:                   return "";
    }
}
std::string Inventory::itemDesc(ItemType t) {
    switch (t) {
    case ItemType::Seed:       return "Plant in tilled soil.";
    case ItemType::Crop:       return "Harvested produce.";
    case ItemType::Wood:       return "Lumber for crafting.";
    case ItemType::Stone:      return "Construction material.";
    case ItemType::FishingRod: return "Equip to fish near water.";
    case ItemType::Sword:      return "Equip to attack enemies.";
    case ItemType::Hoe:        return "Equip to dig to till soil.";
    case ItemType::Carp:       return "A common river fish.";
    case ItemType::Trout:      return "A fast mountain fish.";
    case ItemType::Salmon:     return "A valuable pink fish.";
    case ItemType::Catfish:    return "A rare bottom dweller.";
    case ItemType::GoldenFish: return "Legendary. Worth a fortune.";
    default:                   return "";
    }
}

// -------------------------------------------------------
//  Ctor
// -------------------------------------------------------
Inventory::Inventory() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
    slots[0] = { ItemType::Seed, 5 };
    slots[1] = { ItemType::Sword, 1 };
    slots[2] = { ItemType::Hoe, 1 };
}

// -------------------------------------------------------
//  Slot rect helpers
// -------------------------------------------------------
sf::FloatRect Inventory::hotbarSlotRect(int i) const {
    float totalW = HOTBAR_SLOTS * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD;
    float startX = (m_ws.x - totalW) * 0.5f;
    float startY = m_ws.y - SLOT_SIZE - 14.f;
    return sf::FloatRect(
        sf::Vector2f{ startX + i * (SLOT_SIZE + SLOT_PAD), startY },
        sf::Vector2f{ SLOT_SIZE, SLOT_SIZE });
}

sf::FloatRect Inventory::backpackSlotRect(int i) const {
    int col = i % BP_COLS;
    int row = i / BP_COLS;
    float x = m_bpPx + SLOT_PAD + col * (SLOT_SIZE + SLOT_PAD);
    float y = m_bpPy + HEADER_H + row * (SLOT_SIZE + SLOT_PAD);
    return sf::FloatRect(sf::Vector2f{ x, y }, sf::Vector2f{ SLOT_SIZE, SLOT_SIZE });
}

int Inventory::hitTestHotbar(sf::Vector2f p) const {
    for (int i = 0; i < HOTBAR_SLOTS; ++i)
        if (hotbarSlotRect(i).contains(p)) return i;
    return -1;
}

int Inventory::hitTestBackpack(sf::Vector2f p) const {
    if (!m_backpackOpen) return -1;
    for (int i = 0; i < BACKPACK_SLOTS; ++i)
        if (backpackSlotRect(i).contains(p)) return HOTBAR_SLOTS + i;
    return -1;
}

int Inventory::hitTestAll(sf::Vector2f p) const {
    int h = hitTestHotbar(p);
    if (h >= 0) return h;
    return hitTestBackpack(p);
}

// -------------------------------------------------------
//  Item management
// -------------------------------------------------------
bool Inventory::addItem(ItemType type, int count) {
    for (auto& s : slots)
        if (s.type == type) { s.count += count; return true; }
    for (auto& s : slots)
        if (s.type == ItemType::None) { s = { type, count }; return true; }
    return false;
}

bool Inventory::removeItem(ItemType type, int count) {
    for (auto& s : slots)
        if (s.type == type && s.count >= count) {
            s.count -= count;
            if (s.count == 0) s = {};
            return true;
        }
    return false;
}

int Inventory::countOf(ItemType type) const {
    int total = 0;
    for (auto& s : slots)
        if (s.type == type) total += s.count;
    return total;
}

ItemType Inventory::selectedItem() const {
    if (selectedSlot >= 0 && selectedSlot < HOTBAR_SLOTS)
        return slots[selectedSlot].type;
    return ItemType::None;
}

// -------------------------------------------------------
//  Mouse events — Drag & Drop
// -------------------------------------------------------
void Inventory::onMousePressed(sf::Vector2f mouse) {
    int idx = hitTestAll(mouse);
    if (idx < 0 || slots[idx].type == ItemType::None) return;

    m_dragging = true;
    m_dragSrcIdx = idx;
    m_dragItem = slots[idx];
    m_dragPos = mouse;
    // Vizuelno sakrij iz source slota
    slots[idx] = {};
}

void Inventory::onMouseReleased(sf::Vector2f mouse) {
    if (!m_dragging) return;
    m_dragging = false;

    int dst = hitTestAll(mouse);

    if (dst < 0 || dst == m_dragSrcIdx) {
        // Dropovan izvan slotova ili isti slot — vrati
        slots[m_dragSrcIdx] = m_dragItem;
    }
    else {
        // Swap ili stack
        if (slots[dst].type == m_dragItem.type) {
            // Stack
            slots[dst].count += m_dragItem.count;
        }
        else if (slots[dst].type == ItemType::None) {
            // Prazan slot — postavi
            slots[dst] = m_dragItem;
        }
        else {
            // Swap
            slots[m_dragSrcIdx] = slots[dst];
            slots[dst] = m_dragItem;
        }
    }

    m_dragSrcIdx = -1;
    m_dragItem = {};
}

void Inventory::onMouseMoved(sf::Vector2f mouse) {
    if (m_dragging) m_dragPos = mouse;
}

// -------------------------------------------------------
//  Draw
// -------------------------------------------------------
void Inventory::draw(sf::RenderWindow& window) {
    m_ws = window.getSize();

    // Backpack panel bounds — racunaj uvijek
    float bpW = BP_COLS * (SLOT_SIZE + SLOT_PAD) + SLOT_PAD;
    float bpH = BP_ROWS * (SLOT_SIZE + SLOT_PAD) + HEADER_H + FOOTER_H;
    m_bpPx = (m_ws.x - bpW) * 0.5f;
    // Backpack sjedni tik iznad hotbara
    float hotbarY = m_ws.y - SLOT_SIZE - 14.f;
    m_bpPy = hotbarY - bpH - 12.f;
    m_bpW = bpW;
    m_bpH = bpH;

    if (m_backpackOpen) drawBackpack(window);
    drawHotbar(window);
    if (m_dragging)     drawDragItem(window);
}

// -------------------------------------------------------
//  Draw single slot
// -------------------------------------------------------
void Inventory::drawSlot(sf::RenderWindow& w, sf::FloatRect r,
    const Item& item, bool selected,
    bool highlighted, bool isDragSource) {
    // Pozadina
    sf::RectangleShape bg(r.size);
    bg.setPosition(r.position);

    if (isDragSource)
        bg.setFillColor(sf::Color(20, 20, 30, 120));
    else if (selected)
        bg.setFillColor(sf::Color(80, 70, 30, 240));
    else if (highlighted)
        bg.setFillColor(sf::Color(50, 60, 80, 220));
    else
        bg.setFillColor(sf::Color(28, 32, 48, 220));

    bg.setOutlineThickness(2.f);
    if (isDragSource)
        bg.setOutlineColor(sf::Color(100, 100, 120, 80));
    else if (selected)
        bg.setOutlineColor(sf::Color(255, 210, 60));
    else if (highlighted)
        bg.setOutlineColor(sf::Color(120, 160, 220, 200));
    else
        bg.setOutlineColor(sf::Color(55, 65, 95));
    w.draw(bg);

    if (item.type == ItemType::None || isDragSource) return;

    // Ikona — uvijek kvadrat (min od w/h), centrirano u slotu
    float iconPad = 7.f;
    float iconSize = std::min(r.size.x, r.size.y) - iconPad * 2.f;
    float iconX = r.position.x + (r.size.x - iconSize) * 0.5f;
    float iconY = r.position.y + (r.size.y - iconSize) * 0.5f;

    sf::RectangleShape icon(sf::Vector2f{ iconSize, iconSize });
    icon.setPosition({ iconX, iconY });
    icon.setFillColor(itemColor(item.type));
    icon.setOutlineThickness(1.5f);
    icon.setOutlineColor(sf::Color(0, 0, 0, 80));
    w.draw(icon);

    // Count — donji desni ugao (bez naziva)
    if (item.count > 1) {
        sf::Text countT(m_font, std::to_string(item.count), 11);
        countT.setFillColor(sf::Color::White);
        auto cb = countT.getLocalBounds();
        countT.setPosition({
            r.position.x + r.size.x - cb.size.x - 3.f,
            r.position.y + r.size.y - 15.f });
        w.draw(countT);
    }
}

// -------------------------------------------------------
//  Hotbar
// -------------------------------------------------------
void Inventory::drawHotbar(sf::RenderWindow& w) {
    float totalW = HOTBAR_SLOTS * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD;
    float startX = (m_ws.x - totalW) * 0.5f - SLOT_PAD;
    float startY = m_ws.y - SLOT_SIZE - 14.f - SLOT_PAD;
    float panW = totalW + SLOT_PAD * 2.f;
    float panH = SLOT_SIZE + SLOT_PAD * 2.f;

    // Panel pozadina
    sf::RectangleShape hotbarBg({ panW, panH });
    hotbarBg.setPosition({ startX, startY });
    hotbarBg.setFillColor(sf::Color(14, 16, 28, 210));
    hotbarBg.setOutlineThickness(2.f);
    hotbarBg.setOutlineColor(sf::Color(55, 65, 95));
    w.draw(hotbarBg);

    for (int i = 0; i < HOTBAR_SLOTS; ++i) {
        sf::FloatRect r = hotbarSlotRect(i);
        bool isDragSrc = m_dragging && m_dragSrcIdx == i;
        bool selected = (i == selectedSlot);

        // Slot bg
        sf::RectangleShape bg(r.size);
        bg.setPosition(r.position);
        if (isDragSrc)
            bg.setFillColor(sf::Color(20, 20, 30, 120));
        else if (selected)
            bg.setFillColor(sf::Color(80, 70, 30, 240));
        else
            bg.setFillColor(sf::Color(28, 32, 48, 220));
        bg.setOutlineThickness(2.f);
        bg.setOutlineColor(isDragSrc
            ? sf::Color(100, 100, 120, 80)
            : selected
            ? sf::Color(255, 210, 60)
            : sf::Color(55, 65, 95));
        w.draw(bg);

        // Item — kvadratna kocka, centrirana, bez teksta
        if (!isDragSrc && slots[i].type != ItemType::None) {
            float pad = 7.f;
            float cubeSize = std::min(r.size.x, r.size.y) - pad * 2.f;
            float cubeX = r.position.x + (r.size.x - cubeSize) * 0.5f;
            float cubeY = r.position.y + (r.size.y - cubeSize) * 0.5f;

            sf::RectangleShape cube(sf::Vector2f{ cubeSize, cubeSize });
            cube.setPosition({ cubeX, cubeY });
            cube.setFillColor(itemColor(slots[i].type));
            cube.setOutlineThickness(1.f);
            cube.setOutlineColor(sf::Color(0, 0, 0, 80));
            w.draw(cube);

            // Count samo ako > 1
            if (slots[i].count > 1) {
                sf::Text countT(m_font, std::to_string(slots[i].count), 10);
                countT.setFillColor(sf::Color::White);
                auto cb = countT.getLocalBounds();
                countT.setPosition({
                    r.position.x + r.size.x - cb.size.x - 3.f,
                    r.position.y + r.size.y - 13.f });
                w.draw(countT);
            }
        }

        // Broj slota (1-5) ispod slota
        sf::Text numT(m_font, std::to_string(i + 1), 9);
        numT.setFillColor(sf::Color(80, 80, 110));
        auto nb = numT.getLocalBounds();
        numT.setPosition({
            r.position.x + (r.size.x - nb.size.x) * 0.5f,
            r.position.y + r.size.y + 2.f });
        w.draw(numT);
    }

    // Tab hint — ispod hotbar panela, ne iznad (ne preklapa backpack)
    sf::Text hint(m_font, "[ Tab ]", 10);
    hint.setFillColor(sf::Color(60, 60, 80));
    auto hb = hint.getLocalBounds();
    hint.setPosition({
        (m_ws.x - hb.size.x) * 0.5f,
        startY + panH + 3.f });
    w.draw(hint);
}

// -------------------------------------------------------
//  Backpack panel (Stardew Valley stil)
// -------------------------------------------------------
void Inventory::drawBackpack(sf::RenderWindow& w) {
    // Panel sjena
    sf::RectangleShape shadow({ m_bpW + 8.f, m_bpH + 8.f });
    shadow.setPosition({ m_bpPx - 4.f, m_bpPy + 4.f });
    shadow.setFillColor(sf::Color(0, 0, 0, 80));
    w.draw(shadow);

    // Panel
    sf::RectangleShape panel({ m_bpW, m_bpH });
    panel.setPosition({ m_bpPx, m_bpPy });
    panel.setFillColor(sf::Color(16, 20, 34, 248));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(70, 90, 140, 220));
    w.draw(panel);

    // Header traka
    sf::RectangleShape header({ m_bpW, HEADER_H });
    header.setPosition({ m_bpPx, m_bpPy });
    header.setFillColor(sf::Color(24, 30, 52, 255));
    w.draw(header);

    // Header separator
    sf::RectangleShape hSep({ m_bpW, 2.f });
    hSep.setPosition({ m_bpPx, m_bpPy + HEADER_H - 2.f });
    hSep.setFillColor(sf::Color(70, 90, 140, 180));
    w.draw(hSep);

    // Naslov
    sf::Text title(m_font, "BACKPACK", 16);
    title.setFillColor(sf::Color(180, 210, 255));
    title.setPosition({ m_bpPx + 12.f, m_bpPy + 10.f });
    w.draw(title);

    // Zauzeto / ukupno
    int used = 0;
    for (int i = HOTBAR_SLOTS; i < SLOTS; ++i)
        if (slots[i].type != ItemType::None) ++used;
    sf::Text capT(m_font,
        std::to_string(used) + " / " + std::to_string(BACKPACK_SLOTS), 11);
    capT.setFillColor(sf::Color(100, 120, 160));
    auto cb = capT.getLocalBounds();
    capT.setPosition({ m_bpPx + m_bpW - cb.size.x - 10.f, m_bpPy + 12.f });
    w.draw(capT);

    // Close hint
    sf::Text closeH(m_font, "[ Tab ]", 10);
    closeH.setFillColor(sf::Color(80, 80, 110));
    closeH.setPosition({ m_bpPx + m_bpW - 52.f, m_bpPy + m_bpH - 16.f });
    w.draw(closeH);

    // Slotovi
    for (int i = 0; i < BACKPACK_SLOTS; ++i) {
        int slotIdx = HOTBAR_SLOTS + i;
        bool isDragSrc = m_dragging && m_dragSrcIdx == slotIdx;
        // Highlighted ako drag lebdi iznad
        bool highlighted = false;
        if (m_dragging && !isDragSrc) {
            sf::FloatRect r = backpackSlotRect(i);
            highlighted = r.contains(m_dragPos);
        }
        drawSlot(w, backpackSlotRect(i), slots[slotIdx],
            false, highlighted, isDragSrc);
    }

    // Highlight hotbar slotova dok drag
    if (m_dragging) {
        for (int i = 0; i < HOTBAR_SLOTS; ++i) {
            if (m_dragSrcIdx == i) continue;
            sf::FloatRect r = hotbarSlotRect(i);
            if (r.contains(m_dragPos)) {
                sf::RectangleShape hl(r.size);
                hl.setPosition(r.position);
                hl.setFillColor(sf::Color(0, 0, 0, 0));
                hl.setOutlineThickness(2.f);
                hl.setOutlineColor(sf::Color(120, 180, 255, 200));
                w.draw(hl);
            }
        }
    }
}

// -------------------------------------------------------
//  Drag ghost item
// -------------------------------------------------------
void Inventory::drawDragItem(sf::RenderWindow& w) {
    if (!m_dragging) return;
    float half = SLOT_SIZE * 0.5f;

    // Sjena
    sf::RectangleShape shadow({ SLOT_SIZE, SLOT_SIZE });
    shadow.setOrigin({ half, half });
    shadow.setPosition({ m_dragPos.x + 4.f, m_dragPos.y + 4.f });
    shadow.setFillColor(sf::Color(0, 0, 0, 60));
    w.draw(shadow);

    // Ghost slot
    sf::RectangleShape ghost({ SLOT_SIZE, SLOT_SIZE });
    ghost.setOrigin({ half, half });
    ghost.setPosition(m_dragPos);
    ghost.setFillColor(sf::Color(30, 36, 55, 200));
    ghost.setOutlineThickness(2.f);
    ghost.setOutlineColor(sf::Color(180, 220, 255, 200));
    w.draw(ghost);

    // Item ikona
    float iconPad = 8.f;
    sf::RectangleShape icon({ SLOT_SIZE - iconPad * 2.f, SLOT_SIZE - iconPad * 2.f - 12.f });
    icon.setOrigin({ (SLOT_SIZE - iconPad * 2.f) * 0.5f, (SLOT_SIZE - iconPad * 2.f - 12.f) * 0.5f });
    icon.setPosition(m_dragPos);
    icon.setFillColor(itemColor(m_dragItem.type));
    icon.setOutlineThickness(1.5f);
    icon.setOutlineColor(sf::Color(255, 255, 255, 60));
    w.draw(icon);

    // Count
    if (m_dragItem.count > 1) {
        sf::Text countT(m_font, std::to_string(m_dragItem.count), 12);
        countT.setFillColor(sf::Color::White);
        auto cb = countT.getLocalBounds();
        countT.setPosition({
            m_dragPos.x + half - cb.size.x - 4.f,
            m_dragPos.y + half - 16.f });
        w.draw(countT);
    }

    // Tooltip ispod kursora
    std::string tip = itemName(m_dragItem.type) + "\n" + itemDesc(m_dragItem.type);
    sf::Text tooltip(m_font, tip, 10);
    tooltip.setFillColor(sf::Color(200, 220, 255, 220));
    sf::FloatRect tb = tooltip.getLocalBounds();
    float tx = m_dragPos.x + half + 8.f;
    float ty = m_dragPos.y - half;
    // Clip da ne izadje van ekrana
    if (tx + tb.size.x + 8.f > m_ws.x) tx = m_dragPos.x - tb.size.x - half - 8.f;

    sf::RectangleShape tipBg({ tb.size.x + 10.f, tb.size.y + 10.f });
    tipBg.setPosition({ tx - 4.f, ty - 4.f });
    tipBg.setFillColor(sf::Color(14, 16, 28, 220));
    tipBg.setOutlineThickness(1.f);
    tipBg.setOutlineColor(sf::Color(70, 90, 140));
    w.draw(tipBg);
    tooltip.setPosition({ tx, ty });
    w.draw(tooltip);
}