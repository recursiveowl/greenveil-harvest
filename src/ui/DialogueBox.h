#pragma once
#include "pch.h"

class DialogueBox {
public:
    DialogueBox();
    void show(const std::string& speaker, const std::string& text);
    void hide();
    bool isVisible() const { return m_visible; }
    void draw(sf::RenderWindow& window);

private:
    bool        m_visible{false};
    std::string m_speaker;
    std::string m_text;
    sf::Font    m_font;
    bool        m_fontLoaded{false};
};
