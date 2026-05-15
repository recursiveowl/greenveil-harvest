#include "core/Camera.h"

Camera::Camera(unsigned windowW, unsigned windowH) {
    m_view  = sf::View(sf::FloatRect({0.f, 0.f},
                       {static_cast<float>(windowW),
                        static_cast<float>(windowH)}));
    m_uiView = m_view;
}

void Camera::follow(sf::Vector2f target, float dt) {
    sf::Vector2f current = m_view.getCenter();
    sf::Vector2f delta   = target - current;
    m_view.setCenter(current + delta * m_smoothSpeed * dt);
}

void Camera::apply(sf::RenderWindow& window) {
    window.setView(m_view);
}

void Camera::snapTo(sf::Vector2f target) {
    m_view.setCenter(target);
}

void Camera::applyUI(sf::RenderWindow& window) {
    window.setView(m_uiView);
}
