#pragma once
#include "pch.h"

class Camera {
public:
    Camera(unsigned windowW, unsigned windowH);

    void follow(sf::Vector2f target, float dt);
    void apply(sf::RenderWindow& window);
    void applyUI(sf::RenderWindow& window);
    void snapTo(sf::Vector2f target);

    sf::View getView() const { return m_view; }

private:
    sf::View m_view;
    sf::View m_uiView;
    float    m_smoothSpeed = 5.f;
};
