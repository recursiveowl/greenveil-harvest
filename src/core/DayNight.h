#pragma once
#include "pch.h"

enum class TimeOfDay : uint8_t {
    Dawn,    // 5-7
    Morning, // 7-12
    Noon,    // 12-15
    Evening, // 15-19
    Dusk,    // 19-21
    Night,   // 21-5
};

class DayNight {
public:
    // Vrati overlay boju za cijeli ekran (tamni/svijetli filter)
    sf::Color getOverlayColor(float gameHour) const;

    // Vrati trenutno doba dana
    TimeOfDay getTimeOfDay(float gameHour) const;

    // Vrati naziv doba dana
    std::string getTimeLabel(float gameHour) const;

    // Vrati boju ambijentalnog svjetla za tile rendering
    sf::Color getAmbientColor(float gameHour) const;

    void draw(sf::RenderWindow& window, float gameHour);

private:
    // Interpoliraj izmedju dvije boje
    sf::Color lerpColor(sf::Color a, sf::Color b, float t) const;
};
