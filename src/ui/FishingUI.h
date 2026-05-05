#pragma once
#include "pch.h"
#include "systems/FishingSystem.h"

class FishingUI {
public:
    FishingUI();

    // UI space
    void draw(sf::RenderWindow& window, const FishingSystem& fs,
        bool canFish);

    // World space — zovemo iz Game::render prije applyUI
    void drawBobber(sf::RenderWindow& w,
        const BobberAnim& bobber,
        sf::Vector2f castOrigin,
        FishingState state);

private:
    void drawMinigame(sf::RenderWindow& w, const FishingMinigame& mg);
    void drawStateMsg(sf::RenderWindow& w, const FishingSystem& fs);

    sf::Font m_font;
    float    m_blinkTimer{ 0.f };
};