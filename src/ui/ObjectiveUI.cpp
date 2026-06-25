#include "ui/ObjectiveUI.h"

ObjectiveUI::ObjectiveUI() {
    (void)m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

void ObjectiveUI::draw(sf::RenderWindow& window,
    const MissionSystem& missions,
    bool turnInAvailable) {
    sf::Vector2u ws = window.getSize();

    const Mission* active = missions.getActiveMission();
    if (!active) return;

    float boxW = 340.f;
    float boxH = 140.f;
    float extraH = (turnInAvailable &&
        active->status == MissionStatus::ReadyToTurn) ? 32.f : 0.f;
    float boxX = ws.x - boxW - 16.f;
    float boxY = ws.y - boxH - extraH - 16.f;

    sf::RectangleShape bg({ boxW, boxH });
    bg.setPosition({ boxX, boxY });
    bg.setFillColor(sf::Color(15, 15, 25, 215));
    bg.setOutlineThickness(2.f);
    bg.setOutlineColor(active->status == MissionStatus::ReadyToTurn
        ? sf::Color(80, 220, 80, 220)
        : sf::Color(100, 140, 200, 200));
    window.draw(bg);

    sf::Text header(m_font, "OBJECTIVE", 11);
    header.setFillColor(active->status == MissionStatus::ReadyToTurn
        ? sf::Color(80, 220, 80) : sf::Color(120, 160, 220));
    header.setPosition({ boxX + 10.f, boxY + 8.f });
    window.draw(header);

    sf::Text title(m_font, active->title, 15);
    title.setFillColor(sf::Color(255, 220, 100));
    title.setPosition({ boxX + 10.f, boxY + 24.f });
    window.draw(title);

    sf::Text obj(m_font, active->objective, 12);
    obj.setFillColor(sf::Color(210, 210, 210));
    obj.setPosition({ boxX + 10.f, boxY + 44.f });
    window.draw(obj);

    float barY = boxY + 66.f;
    float barW = boxW - 20.f;
    float barH = 8.f;

    auto drawBar = [&](const std::string& label, float ratio,
        sf::Color col, sf::Color labelCol) {
            sf::Text lbl(m_font, label, 11);
            lbl.setFillColor(labelCol);
            lbl.setPosition({ boxX + 10.f, barY });
            window.draw(lbl);

            sf::RectangleShape bbg({ barW, barH });
            bbg.setPosition({ boxX + 10.f, barY + 14.f });
            bbg.setFillColor(sf::Color(40, 40, 40));
            bbg.setOutlineThickness(1.f);
            bbg.setOutlineColor(sf::Color(80, 80, 80));
            window.draw(bbg);

            if (ratio > 0.f) {
                sf::RectangleShape fill({ barW * std::min(ratio, 1.f), barH });
                fill.setPosition({ boxX + 10.f, barY + 14.f });
                fill.setFillColor(ratio >= 1.f ? sf::Color(80, 220, 80) : col);
                window.draw(fill);
            }
            barY += 30.f;
        };

    // Crops progress � od aktivacije questa
    if (active->requireCrops > 0) {
        int cur = std::min(missions.getQuestCrops(), active->requireCrops);
        std::string label = (active->id == 3)
            ? "Fish: " + std::to_string(cur) + " / " + std::to_string(active->requireCrops)
            : "Crops: " + std::to_string(cur) + " / " + std::to_string(active->requireCrops);
        drawBar(label,
            static_cast<float>(cur) / active->requireCrops,
            sf::Color(80, 160, 220), sf::Color(180, 220, 180));
    }

    // Nagrada
    // Format nagrade u silver/gold
    int rs = active->rewardCoins;
    std::string rewardStr = "Reward: ";
    if (rs >= 100) rewardStr += std::to_string(rs / 100) + "g ";
    rewardStr += std::to_string(rs % 100 == 0 && rs >= 100 ? 0 : rs >= 100 ? rs % 100 : rs) + "s";
    sf::Text reward(m_font, rewardStr, 11);
    reward.setFillColor(sf::Color(220, 180, 60));
    reward.setPosition({ boxX + 10.f, boxY + boxH - 16.f });
    window.draw(reward);

    // Turn-in hint
    if (turnInAvailable && active->status == MissionStatus::ReadyToTurn) {
        sf::RectangleShape hint({ boxW, 28.f });
        hint.setPosition({ boxX, boxY + boxH });
        hint.setFillColor(sf::Color(20, 80, 20, 220));
        hint.setOutlineThickness(2.f);
        hint.setOutlineColor(sf::Color(80, 220, 80, 220));
        window.draw(hint);

        sf::Text hintText(m_font, "[ N ] - Turn in quest!", 13);
        hintText.setFillColor(sf::Color(80, 255, 80));
        hintText.setPosition({ boxX + 10.f, boxY + boxH + 6.f });
        window.draw(hintText);
    }
}