#include "ui/DialogueBox.h"

DialogueBox::DialogueBox() {
    m_font.openFromFile("assets/fonts/DejaVuSans.ttf");
}

void DialogueBox::show(const std::string& speaker, const std::string& text) {
    m_speaker = speaker;
    m_text    = text;
    m_visible = true;
}

void DialogueBox::hide() {
    m_visible = false;
}

void DialogueBox::draw(sf::RenderWindow& window) {
    if (!m_visible) return;

    sf::Vector2u ws   = window.getSize();
    float        boxW = ws.x * 0.72f;
    float        boxH = 160.f;
    float        boxX = (ws.x - boxW) * 0.5f;
    float        boxY = ws.y - boxH - 24.f;

    // Background
    sf::RectangleShape bg({boxW, boxH});
    bg.setPosition({boxX, boxY});
    bg.setFillColor(sf::Color(15, 15, 25, 235));
    bg.setOutlineThickness(2.f);
    bg.setOutlineColor(sf::Color(180, 160, 100, 220));
    window.draw(bg);

    // Speaker name bar
    sf::RectangleShape nameBar({220.f, 34.f});
    nameBar.setPosition({boxX + 12.f, boxY - 34.f});
    nameBar.setFillColor(sf::Color(15, 15, 25, 235));
    nameBar.setOutlineThickness(2.f);
    nameBar.setOutlineColor(sf::Color(180, 160, 100, 220));
    window.draw(nameBar);

    // Speaker ime — veci font
    sf::Text speakerText(m_font, m_speaker, 22);
    speakerText.setFillColor(sf::Color(255, 220, 100));
    speakerText.setPosition({boxX + 18.f, boxY - 32.f});
    window.draw(speakerText);

    // Tekst dijaloga — veci font + word wrap rucno
    // Podijeli tekst u linije od max ~60 znakova
    std::string wrapped;
    std::string word;
    int         lineLen = 0;
    for (char c : m_text) {
        if (c == ' ') {
            if (lineLen + static_cast<int>(word.size()) > 58) {
                wrapped += '\n';
                lineLen = 0;
            } else if (!wrapped.empty()) {
                wrapped += ' ';
                lineLen++;
            }
            wrapped += word;
            lineLen += static_cast<int>(word.size());
            word.clear();
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        if (lineLen + static_cast<int>(word.size()) > 58) {
            wrapped += '\n';
        } else if (!wrapped.empty()) {
            wrapped += ' ';
        }
        wrapped += word;
    }

    sf::Text dialogueText(m_font, wrapped, 20);
    dialogueText.setFillColor(sf::Color(235, 235, 235));
    dialogueText.setPosition({boxX + 22.f, boxY + 14.f});
    window.draw(dialogueText);

    // Hint
    sf::Text hint(m_font, "[ Desni klik / F - dalje  |  Esc - zatvori ]", 13);
    hint.setFillColor(sf::Color(140, 140, 140));
    hint.setPosition({boxX + boxW - 370.f, boxY + boxH - 24.f});
    window.draw(hint);
}
