#include "ui/BuildingUI.h"
#include <cmath>

BuildingUI::BuildingUI() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

void BuildingUI::toggle() {
    m_open = !m_open;
    if (!m_open) m_hasSelected = false;
}

void BuildingUI::close() {
    m_open = false;
    m_hasSelected = false;
}

// -------------------------------------------------------
//  Handle click
// -------------------------------------------------------
bool BuildingUI::handleClick(sf::Vector2f mouse,
    const BuildingSystem& bs,
    const Inventory& inv) {
    if (!m_open) return false;

    sf::Vector2u ws = m_windowSize;
    float px = (ws.x - PANEL_W) * 0.5f;
    float py = (ws.y - PANEL_H) * 0.5f + 20.f;

    const auto& defs = bs.getDefs();
    int cols = 4;

    for (int i = 0; i < static_cast<int>(defs.size()); ++i) {
        int col = i % cols;
        int row = i / cols;
        float cx = px + 12.f + col * (CARD_W + CARD_PAD);
        float cy = py + 70.f + row * (CARD_H + CARD_PAD);

        sf::FloatRect r(sf::Vector2f{ cx, cy }, sf::Vector2f{ CARD_W, CARD_H });
        if (r.contains(mouse)) {
            m_selected = defs[i].type;
            m_hasSelected = true;
            m_open = false; // zatvori meni, pocni gradnju
            return true;
        }
    }
    return false;
}

// -------------------------------------------------------
//  Draw
// -------------------------------------------------------
void BuildingUI::draw(sf::RenderWindow& window,
    const BuildingSystem& bs,
    const Inventory& inv) {
    m_windowSize = window.getSize();
    if (!m_open) return;

    sf::Vector2u ws = window.getSize();
    float px = (ws.x - PANEL_W) * 0.5f;
    float py = (ws.y - PANEL_H) * 0.5f + 20.f;

    // Panel
    sf::RectangleShape panel({ PANEL_W, PANEL_H });
    panel.setPosition({ px, py });
    panel.setFillColor(sf::Color(10, 12, 22, 252));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(80, 120, 180, 220));
    window.draw(panel);

    // Naslov
    sf::Text title(m_font, "BUILD MENU", 20);
    title.setFillColor(sf::Color(160, 200, 255));
    title.setPosition({ px + 14.f, py + 12.f });
    window.draw(title);

    sf::Text hint(m_font, "Click to select  |  [ B / Esc ] Close", 11);
    hint.setFillColor(sf::Color(80, 80, 110));
    hint.setPosition({ px + PANEL_W - 240.f, py + 16.f });
    window.draw(hint);

    // Separator
    sf::RectangleShape sep({ PANEL_W, 2.f });
    sep.setPosition({ px, py + 44.f });
    sep.setFillColor(sf::Color(50, 70, 120, 160));
    window.draw(sep);

    // Materijali igraca
    int wood = inv.countOf(ItemType::Wood);
    int stone = inv.countOf(ItemType::Stone);

    sf::Text matT(m_font,
        "Wood: " + std::to_string(wood) +
        "   Stone: " + std::to_string(stone), 12);
    matT.setFillColor(sf::Color(180, 200, 160));
    matT.setPosition({ px + 14.f, py + 50.f });
    window.draw(matT);

    // Labele sekcija
    sf::Text woodLbl(m_font, "WOOD", 11);
    woodLbl.setFillColor(sf::Color(190, 140, 70));
    woodLbl.setPosition({ px + 12.f, py + 66.f });
    window.draw(woodLbl);

    sf::Text stoneLbl(m_font, "STONE", 11);
    stoneLbl.setFillColor(sf::Color(160, 160, 180));
    // stone section pocetak — 2 reda wood (6 items) = row 2
    stoneLbl.setPosition({ px + 12.f, py + 66.f + 2 * (CARD_H + CARD_PAD) + 4.f });
    window.draw(stoneLbl);

    // Kartice
    const auto& defs = bs.getDefs();
    int cols = 4;

    for (int i = 0; i < static_cast<int>(defs.size()); ++i) {
        int col = i % cols;
        int row = i / cols;
        float cx = px + 12.f + col * (CARD_W + CARD_PAD);
        float cy = py + 80.f + row * (CARD_H + CARD_PAD);

        bool affordable = inv.countOf(defs[i].material) >= defs[i].cost;
        bool sel = m_hasSelected && m_selected == defs[i].type;
        drawCard(window, defs[i], i, sel, affordable, cx, cy);
    }

    // Info o selektovanoj zgradi (desna strana) ako je hovered
    if (m_hoveredIdx >= 0 && m_hoveredIdx < static_cast<int>(defs.size())) {
        float ix = px + 12.f + cols * (CARD_W + CARD_PAD) + 10.f;
        drawPhysicsInfo(window, defs[m_hoveredIdx], ix, py + 70.f);
    }
}

// -------------------------------------------------------
//  Kartica za jedan building type
// -------------------------------------------------------
void BuildingUI::drawCard(sf::RenderWindow& w, const BuildingDef& def,
    int idx, bool selected, bool affordable,
    float cx, float cy) {
    // Pozadina
    sf::Color bgCol = selected
        ? sf::Color(40, 60, 100, 240)
        : affordable
        ? sf::Color(22, 26, 42, 230)
        : sf::Color(18, 18, 26, 200);

    sf::Color borderCol = selected
        ? sf::Color(120, 180, 255)
        : affordable
        ? sf::Color(60, 80, 130)
        : sf::Color(50, 50, 70);

    sf::RectangleShape card({ CARD_W, CARD_H });
    card.setPosition({ cx, cy });
    card.setFillColor(bgCol);
    card.setOutlineThickness(selected ? 2.f : 1.5f);
    card.setOutlineColor(borderCol);
    w.draw(card);

    // Ikona — kvadrat sa bojom materijala, velicina proporcionalna size
    float iconMax = 32.f;
    float iconW = std::min(iconMax, iconMax * def.size.x / 2.f);
    float iconH = std::min(iconMax, iconMax * def.size.y / 3.f);
    sf::RectangleShape icon({ iconW, iconH });
    icon.setPosition({ cx + (CARD_W - iconW) * 0.5f, cy + 10.f });
    icon.setFillColor(affordable
        ? def.color
        : sf::Color(def.color.r / 2, def.color.g / 2, def.color.b / 2));
    icon.setOutlineThickness(1.5f);
    icon.setOutlineColor(sf::Color(0, 0, 0, 120));
    w.draw(icon);

    // Naziv
    sf::Text nameT(m_font, def.name, 10);
    nameT.setFillColor(affordable
        ? sf::Color(220, 220, 240)
        : sf::Color(110, 110, 130));
    auto nb = nameT.getLocalBounds();
    nameT.setPosition({ cx + (CARD_W - nb.size.x) * 0.5f, cy + 48.f });
    w.draw(nameT);

    // Cijena
    std::string matName = (def.material == ItemType::Wood) ? "Wood" : "Stone";
    std::string costStr = std::to_string(def.cost) + " " + matName;
    sf::Text costT(m_font, costStr, 10);
    costT.setFillColor(affordable
        ? sf::Color(140, 210, 140)
        : sf::Color(200, 80, 80));
    auto cb = costT.getLocalBounds();
    costT.setPosition({ cx + (CARD_W - cb.size.x) * 0.5f, cy + 62.f });
    w.draw(costT);

    // Dimenzije
    std::string sizeStr = std::to_string(def.size.x) + "x" +
        std::to_string(def.size.y) + " tiles";
    sf::Text sizeT(m_font, sizeStr, 9);
    sizeT.setFillColor(sf::Color(100, 100, 130));
    auto sb = sizeT.getLocalBounds();
    sizeT.setPosition({ cx + (CARD_W - sb.size.x) * 0.5f, cy + 76.f });
    w.draw(sizeT);

    // Not affordable overlay
    if (!affordable) {
        sf::RectangleShape overlay({ CARD_W, CARD_H });
        overlay.setPosition({ cx, cy });
        overlay.setFillColor(sf::Color(0, 0, 0, 60));
        w.draw(overlay);
    }
}

// -------------------------------------------------------
//  Physics info panel
// -------------------------------------------------------
void BuildingUI::drawPhysicsInfo(sf::RenderWindow& w,
    const BuildingDef& def,
    float px, float py) {
    float panW = 180.f;
    float panH = 200.f;

    sf::RectangleShape panel({ panW, panH });
    panel.setPosition({ px, py });
    panel.setFillColor(sf::Color(14, 18, 32, 240));
    panel.setOutlineThickness(1.5f);
    panel.setOutlineColor(sf::Color(60, 80, 130));
    w.draw(panel);

    sf::Text title(m_font, "Physics Info", 12);
    title.setFillColor(sf::Color(140, 180, 255));
    title.setPosition({ px + 8.f, py + 8.f });
    w.draw(title);

    sf::RectangleShape sep({ panW, 1.f });
    sep.setPosition({ px, py + 26.f });
    sep.setFillColor(sf::Color(50, 70, 120));
    w.draw(sep);

    auto row = [&](const std::string& label, const std::string& val,
        float y, sf::Color valCol) {
            sf::Text lbl(m_font, label, 10);
            lbl.setFillColor(sf::Color(140, 140, 160));
            lbl.setPosition({ px + 8.f, y });
            w.draw(lbl);
            sf::Text vt(m_font, val, 10);
            vt.setFillColor(valCol);
            vt.setPosition({ px + panW - vt.getLocalBounds().size.x - 8.f, y });
            w.draw(vt);
        };

    float y = py + 32.f;
    row("Material:", def.material == ItemType::Wood ? "Wood" : "Stone",
        y, sf::Color(200, 180, 120)); y += 18.f;
    row("HP:", std::to_string(static_cast<int>(def.hp)),
        y, sf::Color(100, 220, 100)); y += 18.f;
    row("Mass:",
        std::to_string(static_cast<int>(def.mass)) + " kg",
        y, sf::Color(180, 200, 255)); y += 18.f;
    row("Max load:",
        std::to_string(static_cast<int>(def.strength / 1000.f)) + " kN",
        y, sf::Color(255, 200, 100)); y += 18.f;
    row("Size:",
        std::to_string(def.size.x) + "x" + std::to_string(def.size.y),
        y, sf::Color(180, 180, 200)); y += 18.f;
    row("Solid:", def.solid ? "Yes" : "No",
        y, def.solid ? sf::Color(255, 150, 80) : sf::Color(120, 200, 120));
    y += 24.f;

    // Edukativni opis fizike
    std::string physNote;
    if (def.material == ItemType::Stone)
        physNote = "Stone = high\ncompressive strength";
    else
        physNote = "Wood = flexible,\ngood tensile strength";

    sf::Text note(m_font, physNote, 9);
    note.setFillColor(sf::Color(120, 160, 200));
    note.setPosition({ px + 8.f, y });
    w.draw(note);
}