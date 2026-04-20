#pragma once
#include "pch.h"

// Generiše sve placeholder teksture u memoriji
// Zamijeni sa pravim spritesheetom kada budu gotovi assets
namespace PlaceholderAssets {

    // Napravi solid-color teksturu sa border-om
    inline sf::Texture makeTile(sf::Color fill, sf::Color border, unsigned size = 32) {
        sf::Image img({size, size}, fill);
        // Nacrtaj border
        for (unsigned i = 0; i < size; ++i) {
            img.setPixel({i, 0},      border);
            img.setPixel({i, size-1}, border);
            img.setPixel({0, i},      border);
            img.setPixel({size-1, i}, border);
        }
        sf::Texture tex(img);
        return tex;
    }

    // Napravi teksturu za igraca (covjek silueta)
    inline sf::Texture makePlayer(unsigned size = 32) {
        sf::Image img({size, size}, sf::Color::Transparent);
        sf::Color body(220, 120, 60);
        sf::Color head(255, 200, 150);
        sf::Color legs(80, 60, 140);

        // Noge
        for (unsigned y = 20; y < 30; ++y)
            for (unsigned x = 10; x < 22; ++x)
                img.setPixel({x, y}, legs);

        // Tijelo
        for (unsigned y = 12; y < 21; ++y)
            for (unsigned x = 10; x < 22; ++x)
                img.setPixel({x, y}, body);

        // Glava
        for (unsigned y = 4; y < 13; ++y)
            for (unsigned x = 11; x < 21; ++x)
                img.setPixel({x, y}, head);

        sf::Texture tex(img);
        return tex;
    }

    // Napravi teksturu za travu
    inline sf::Texture makeGrass() {
        return makeTile(sf::Color(34, 139, 34), sf::Color(20, 100, 20));
    }

    // Napravi teksturu za zemlju
    inline sf::Texture makeDirt() {
        return makeTile(sf::Color(139, 90, 43), sf::Color(100, 60, 20));
    }

    // Napravi teksturu za vodu
    inline sf::Texture makeWater() {
        return makeTile(sf::Color(64, 164, 223), sf::Color(30, 120, 180));
    }

    // Napravi teksturu za kamen
    inline sf::Texture makeStone() {
        return makeTile(sf::Color(128, 128, 128), sf::Color(80, 80, 80));
    }
}
