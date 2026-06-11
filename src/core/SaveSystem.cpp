#include "core/SaveSystem.h"
#include <fstream>
#include <cstdio>

std::string SaveSystem::slotPath(int slot) {
    return "save_" + std::to_string(slot) + ".bin";
}

bool SaveSystem::save(int slot, const SaveData& data) {
    std::ofstream f(slotPath(slot), std::ios::binary);
    if (!f) return false;
    f.write(reinterpret_cast<const char*>(&data), sizeof(SaveData));
    return f.good();
}

bool SaveSystem::load(int slot, SaveData& data) {
    std::ifstream f(slotPath(slot), std::ios::binary);
    if (!f) return false;
    f.read(reinterpret_cast<char*>(&data), sizeof(SaveData));
    if (f.good()) { data.hasData = true; return true; }
    return false;
}

bool SaveSystem::saveTiles(int slot, const TileMap& map) {
    std::ofstream f("tiles_" + std::to_string(slot) + ".bin", std::ios::binary);
    if (!f) return false;
    int w = map.getWidth(), h = map.getHeight();
    f.write(reinterpret_cast<const char*>(&w), sizeof(int));
    f.write(reinterpret_cast<const char*>(&h), sizeof(int));
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            const Tile& t = map.getTile(x, y);
            uint8_t type = static_cast<uint8_t>(t.type);
            f.write(reinterpret_cast<const char*>(&type), 1);
        }
    return f.good();
}

bool SaveSystem::loadTiles(int slot, TileMap& map) {
    std::ifstream f("tiles_" + std::to_string(slot) + ".bin", std::ios::binary);
    if (!f) return false;
    int w, h;
    f.read(reinterpret_cast<char*>(&w), sizeof(int));
    f.read(reinterpret_cast<char*>(&h), sizeof(int));
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            uint8_t type;
            f.read(reinterpret_cast<char*>(&type), 1);
            map.setTileType(x, y, static_cast<TileType>(type));
        }
    return f.good();
}

void SaveSystem::deleteSlot(int slot) {
    std::remove(slotPath(slot).c_str());
    std::remove(("tiles_" + std::to_string(slot) + ".bin").c_str());
}