#include "systems/ProgressionSystem.h"
#include <cmath>

ProgressionSystem::ProgressionSystem() {
    auto make = [](SkillType t, std::string name,
                   std::string desc, float val) {
        Skill s;
        s.type           = t;
        s.name           = std::move(name);
        s.description    = std::move(desc);
        s.valuePerLevel  = val;
        return s;
    };
    m_skills.push_back(make(SkillType::AttackDamage, "Strength",
        "+5 attack damage per level", 5.f));
    m_skills.push_back(make(SkillType::AttackSpeed,  "Agility",
        "-0.05s attack cooldown per level", 0.05f));
    m_skills.push_back(make(SkillType::MaxHealth,    "Vitality",
        "+20 max HP per level", 20.f));
    m_skills.push_back(make(SkillType::MoveSpeed,    "Swiftness",
        "+10 move speed per level", 10.f));
    m_skills.push_back(make(SkillType::FarmYield,    "Farming",
        "+1 extra crop per harvest per level", 1.f));
}

int ProgressionSystem::getXPForNext() const {
    // XP kriva: 100 * level^1.5
    return static_cast<int>(100.f * std::pow(static_cast<float>(m_level), 1.5f));
}

void ProgressionSystem::addXP(int amount) {
    m_xp += amount;
}

bool ProgressionSystem::levelUp() {
    if (m_xp < getXPForNext()) return false;
    m_xp        -= getXPForNext();
    m_level     += 1;
    m_skillPoints += 2; // 2 skill poena po levelu
    return true;
}

bool ProgressionSystem::upgradeSkill(SkillType type) {
    if (m_skillPoints <= 0) return false;
    for (auto& s : m_skills) {
        if (s.type == type && s.level < s.maxLevel) {
            ++s.level;
            --m_skillPoints;
            return true;
        }
    }
    return false;
}

float ProgressionSystem::getSkillValue(SkillType type) const {
    for (auto& s : m_skills)
        if (s.type == type) return s.level * s.valuePerLevel;
    return 0.f;
}

float ProgressionSystem::getAttackDamage() const {
    return 20.f + getSkillValue(SkillType::AttackDamage);
}
float ProgressionSystem::getAttackSpeed() const {
    return std::max(0.15f, 0.4f - getSkillValue(SkillType::AttackSpeed));
}
float ProgressionSystem::getMaxHealth() const {
    return 100.f + getSkillValue(SkillType::MaxHealth);
}
float ProgressionSystem::getMoveSpeed() const {
    return 160.f + getSkillValue(SkillType::MoveSpeed);
}
float ProgressionSystem::getFarmYield() const {
    return getSkillValue(SkillType::FarmYield);
}
