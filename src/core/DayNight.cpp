#include "core/DayNight.h"
#include <cmath>

sf::Color DayNight::lerpColor(sf::Color a, sf::Color b, float t) const {
    t = std::max(0.f, std::min(1.f, t));
    return sf::Color(
        static_cast<uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<uint8_t>(a.b + (b.b - a.b) * t),
        static_cast<uint8_t>(a.a + (b.a - a.a) * t));
}

TimeOfDay DayNight::getTimeOfDay(float h) const {
    if (h >= 5.f  && h < 7.f)  return TimeOfDay::Dawn;
    if (h >= 7.f  && h < 12.f) return TimeOfDay::Morning;
    if (h >= 12.f && h < 15.f) return TimeOfDay::Noon;
    if (h >= 15.f && h < 19.f) return TimeOfDay::Evening;
    if (h >= 19.f && h < 21.f) return TimeOfDay::Dusk;
    return TimeOfDay::Night;
}

std::string DayNight::getTimeLabel(float h) const {
    switch (getTimeOfDay(h)) {
        case TimeOfDay::Dawn:    return "Dawn";
        case TimeOfDay::Morning: return "Morning";
        case TimeOfDay::Noon:    return "Noon";
        case TimeOfDay::Evening: return "Evening";
        case TimeOfDay::Dusk:    return "Dusk";
        case TimeOfDay::Night:   return "Night";
    }
    return "";
}

sf::Color DayNight::getOverlayColor(float h) const {
    // Svako doba dana ima svoju overlay boju (alpha kanal tamni ekran)
    // Potpuna noc = tamno plava, podne = prozirno
    struct KeyFrame { float hour; sf::Color color; };

    static const KeyFrame frames[] = {
        { 0.f,  sf::Color(10,  10,  40,  200) }, // Ponocno — tamno plavo
        { 5.f,  sf::Color(80,  40,  20,  140) }, // Zora — narandzasto
        { 7.f,  sf::Color(255, 200, 100, 30)  }, // Jutro — toplo
        { 12.f, sf::Color(255, 255, 255, 0)   }, // Podne — bez filtera
        { 15.f, sf::Color(255, 220, 180, 20)  }, // Popodne — toplo
        { 19.f, sf::Color(200, 100, 40,  80)  }, // Vece — narandzasto
        { 21.f, sf::Color(40,  20,  80,  150) }, // Sumrak — ljubicasto
        { 24.f, sf::Color(10,  10,  40,  200) }, // Noc — tamno plavo
    };

    static const int N = sizeof(frames) / sizeof(frames[0]);

    for (int i = 0; i < N - 1; ++i) {
        if (h >= frames[i].hour && h < frames[i+1].hour) {
            float t = (h - frames[i].hour) /
                      (frames[i+1].hour - frames[i].hour);
            return lerpColor(frames[i].color, frames[i+1].color, t);
        }
    }
    return sf::Color(10, 10, 40, 200);
}

sf::Color DayNight::getAmbientColor(float h) const {
    // Boja koja se miješa sa tile bojama
    struct KeyFrame { float hour; sf::Color color; };
    static const KeyFrame frames[] = {
        { 0.f,  sf::Color(60,  60,  120) }, // Noc — plavo
        { 5.f,  sf::Color(180, 120, 80)  }, // Zora — narandzasto
        { 7.f,  sf::Color(255, 220, 180) }, // Jutro — toplo bijelo
        { 12.f, sf::Color(255, 255, 255) }, // Podne — bijelo
        { 17.f, sf::Color(255, 200, 150) }, // Kasno popodne
        { 19.f, sf::Color(200, 120, 60)  }, // Vece
        { 21.f, sf::Color(80,  60,  140) }, // Sumrak
        { 24.f, sf::Color(60,  60,  120) }, // Noc
    };
    static const int N = sizeof(frames) / sizeof(frames[0]);

    for (int i = 0; i < N - 1; ++i) {
        if (h >= frames[i].hour && h < frames[i+1].hour) {
            float t = (h - frames[i].hour) /
                      (frames[i+1].hour - frames[i].hour);
            return lerpColor(frames[i].color, frames[i+1].color, t);
        }
    }
    return sf::Color(60, 60, 120);
}

float DayNight::getEnemyStrengthMult(float h) const {
    // Nocu neprijatelji su jaci
    switch (getTimeOfDay(h)) {
        case TimeOfDay::Night:   return 2.0f;  // 2x jaci nocu
        case TimeOfDay::Dusk:    return 1.5f;  // 1.5x u sumrak
        case TimeOfDay::Dawn:    return 1.3f;  // 1.3x u zoru
        case TimeOfDay::Evening: return 1.1f;  // malo jaci uvece
        default:                 return 1.0f;  // normalno danju
    }
}

void DayNight::draw(sf::RenderWindow& window, float gameHour) {
    sf::Vector2u ws = window.getSize();

    // Overlay — tamni/svjetlosni filter preko cijelog ekrana
    sf::Color overlay = getOverlayColor(gameHour);
    if (overlay.a > 0) {
        sf::RectangleShape rect({
            static_cast<float>(ws.x),
            static_cast<float>(ws.y)});
        rect.setFillColor(overlay);
        window.draw(rect);
    }
}
