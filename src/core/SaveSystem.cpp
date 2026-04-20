#include "core/SaveSystem.h"
#include <fstream>

bool SaveSystem::save(const std::string& path, const SaveData& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f.write(reinterpret_cast<const char*>(&data), sizeof(SaveData));
    return f.good();
}

bool SaveSystem::load(const std::string& path, SaveData& data) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    f.read(reinterpret_cast<char*>(&data), sizeof(SaveData));
    return f.good();
}
