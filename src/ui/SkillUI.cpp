#include "ui/SkillUI.h"

SkillUI::SkillUI() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

int SkillUI::handleClick(sf::Vector2f mouse, const ProgressionSystem& prog) {
    if (!m_visible) return -1;
    sf::Vector2f base{ 300.f, 150.f };
    const auto& skills = prog.getSkills();
    for (int i = 0; i < static_cast<int>(skills.size()); ++i) {
        sf::FloatRect btn(sf::Vector2f{ base.x + 200.f, base.y + 60.f + i * 70.f }, sf::Vector2f{ 120.f, 30.f });
        if (btn.contains(mouse) && prog.getSkillPoints() > 0 &&
            skills[i].level < skills[i].maxLevel)
            return i;
    }
    return -1;
}

void SkillUI::draw(sf::RenderWindow& window, const ProgressionSystem& prog) {
    if (!m_visible) return;

    sf::Vector2u ws = window.getSize();
    float        pw = 400.f, ph = 460.f;
    float        px = (ws.x - pw) * 0.5f;
    float        py = (ws.y - ph) * 0.5f;

    // Panel
    sf::RectangleShape panel({ pw, ph });
    panel.setPosition({ px, py });
    panel.setFillColor(sf::Color(15, 15, 30, 240));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(100, 140, 220));
    window.draw(panel);

    // Header
    sf::Text header(m_font, "CHARACTER", 18);
    header.setFillColor(sf::Color(180, 200, 255));
    header.setPosition({ px + 20.f, py + 14.f });
    window.draw(header);

    // Level + XP
    std::string lvlStr = "Level " + std::to_string(prog.getLevel());
    sf::Text lvlText(m_font, lvlStr, 15);
    lvlText.setFillColor(sf::Color(255, 220, 80));
    lvlText.setPosition({ px + pw - 120.f, py + 16.f });
    window.draw(lvlText);

    // XP bar
    float xpRatio = static_cast<float>(prog.getXP()) /
        static_cast<float>(prog.getXPForNext());
    sf::RectangleShape xpBg({ pw - 40.f, 8.f });
    xpBg.setPosition({ px + 20.f, py + 44.f });
    xpBg.setFillColor(sf::Color(40, 40, 60));
    xpBg.setOutlineThickness(1.f);
    xpBg.setOutlineColor(sf::Color(80, 80, 120));
    window.draw(xpBg);
    sf::RectangleShape xpFill({ (pw - 40.f) * xpRatio, 8.f });
    xpFill.setPosition({ px + 20.f, py + 44.f });
    xpFill.setFillColor(sf::Color(80, 120, 255));
    window.draw(xpFill);

    std::string xpStr = "XP: " + std::to_string(prog.getXP()) +
        " / " + std::to_string(prog.getXPForNext());
    sf::Text xpText(m_font, xpStr, 10);
    xpText.setFillColor(sf::Color(140, 160, 220));
    xpText.setPosition({ px + 20.f, py + 55.f });
    window.draw(xpText);

    // Skill points
    std::string spStr = "Skill Points: " + std::to_string(prog.getSkillPoints());
    sf::Text spText(m_font, spStr, 13);
    spText.setFillColor(prog.getSkillPoints() > 0
        ? sf::Color(80, 220, 80) : sf::Color(160, 160, 160));
    spText.setPosition({ px + 20.f, py + 72.f });
    window.draw(spText);

    // Skills
    const auto& skills = prog.getSkills();
    for (int i = 0; i < static_cast<int>(skills.size()); ++i) {
        float sy = py + 100.f + i * 70.f;
        const Skill& s = skills[i];

        // Naziv
        sf::Text nameText(m_font, s.name, 14);
        nameText.setFillColor(sf::Color(220, 220, 255));
        nameText.setPosition({ px + 20.f, sy });
        window.draw(nameText);

        // Opis
        sf::Text descText(m_font, s.description, 10);
        descText.setFillColor(sf::Color(140, 140, 160));
        descText.setPosition({ px + 20.f, sy + 18.f });
        window.draw(descText);

        // Level dots
        for (int d = 0; d < s.maxLevel; ++d) {
            sf::CircleShape dot(6.f);
            dot.setOrigin({ 6.f, 6.f });
            dot.setPosition({ px + 20.f + d * 18.f, sy + 42.f });
            dot.setFillColor(d < s.level
                ? sf::Color(80, 160, 255)
                : sf::Color(40, 40, 60));
            dot.setOutlineThickness(1.f);
            dot.setOutlineColor(sf::Color(80, 80, 120));
            window.draw(dot);
        }

        // Upgrade button
        bool canUpgrade = prog.getSkillPoints() > 0 && s.level < s.maxLevel;
        sf::RectangleShape btn({ 80.f, 24.f });
        btn.setPosition({ px + pw - 110.f, sy + 30.f });
        btn.setFillColor(canUpgrade
            ? sf::Color(40, 80, 40) : sf::Color(30, 30, 40));
        btn.setOutlineThickness(1.f);
        btn.setOutlineColor(canUpgrade
            ? sf::Color(80, 200, 80) : sf::Color(60, 60, 80));
        window.draw(btn);

        sf::Text btnText(m_font, canUpgrade ? "Upgrade" : "---", 11);
        btnText.setFillColor(canUpgrade
            ? sf::Color(80, 220, 80) : sf::Color(80, 80, 80));
        btnText.setPosition({ px + pw - 104.f, sy + 34.f });
        window.draw(btnText);
    }

    sf::Text hint(m_font, "[ C ] Close", 11);
    hint.setFillColor(sf::Color(100, 100, 120));
    hint.setPosition({ px + pw - 100.f, py + ph - 22.f });
    window.draw(hint);
}