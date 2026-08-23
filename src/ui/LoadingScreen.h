#pragma once
#include "pch.h"

class LoadingScreen {
public:
    LoadingScreen();

    // Poziva se kad loading pocinje - resetuje progress i bira novi tip
    void show();

    // Poziva se nakon svakog zavrsenog load step-a (0.0 - 1.0)
    void setProgress(float t);

    // Lerp-uje prikazani progress ka target-u, rotira tip-ove
    void update(float dt);

    // true kad je i logika zavrsena I bar vizuelno stigao do 100%
    bool isFinished() const;

    void draw(sf::RenderWindow& window);

private:
    void pickNewTip();

    sf::Font     m_font;
    float        m_targetProgress{ 0.f };
    float        m_displayedProgress{ 0.f };
    std::string  m_currentTip;
    float        m_tipTimer{ 0.f };
    std::mt19937 m_rng{ std::random_device{}() };

    static const std::vector<std::string> s_tips;

    static constexpr float LERP_SPEED = 6.f;   // brzina "sustizanja" bara
    static constexpr float TIP_INTERVAL = 3.5f;  // sekunde po tipu
};