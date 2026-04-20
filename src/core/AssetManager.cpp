#include "core/AssetManager.h"

sf::Texture& AssetManager::getTexture(const std::string& path) {
    auto it = m_textures.find(path);
    if (it != m_textures.end()) return it->second;
    sf::Texture tex(path);
    return m_textures.emplace(path, std::move(tex)).first->second;
}

sf::Font& AssetManager::getFont(const std::string& path) {
    auto it = m_fonts.find(path);
    if (it != m_fonts.end()) return it->second;
    sf::Font font(path);
    return m_fonts.emplace(path, std::move(font)).first->second;
}
