#pragma once
#include "pch.h"

class AssetManager {
public:
    sf::Texture& getTexture(const std::string& path);
    sf::Font&    getFont(const std::string& path);

private:
    std::unordered_map<std::string, sf::Texture> m_textures;
    std::unordered_map<std::string, sf::Font>    m_fonts;
};
