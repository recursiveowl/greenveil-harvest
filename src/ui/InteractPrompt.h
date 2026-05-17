#pragma once
#include "pch.h";

class InteractPrompt {
public:
    InteractPrompt();
    void show(const std::string& title, const std::string& body);
    void hide();
    bool isVisible() const { return m_visible; }
    void draw(sf::RenderWindow& w);
private:
    bool        m_visible{ false };
    std::string m_title, m_body;
    sf::Font    m_font;
};