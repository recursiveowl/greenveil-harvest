#pragma once
#include "pch.h"
#include <functional>

struct RenderItem {
    float sortY;
    std::function<void(sf::RenderWindow&)> draw;
};