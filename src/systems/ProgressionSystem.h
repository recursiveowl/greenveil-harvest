#pragma once
#include "pch.h"

enum class SkillType : uint8_t {
    AttackDamage = 0,
    AttackSpeed,
    MaxHealth,
    MoveSpeed,
    FarmYield,
    COUNT
};

struct Skill {
    SkillType   type;
    std::string name;
    std::string description;
    int         level{0};
    int         maxLevel{5};
    float       valuePerLevel{0.f};
};

class ProgressionSystem {
public:
    ProgressionSystem();

    void addXP(int amount);
    bool levelUp();
    bool upgradeSkill(SkillType type);

    int   getLevel()        const { return m_level; }
    int   getXP()           const { return m_xp; }
    int   getXPForNext()    const;
    int   getSkillPoints()  const { return m_skillPoints; }
    float getSkillValue(SkillType type) const;

    const std::vector<Skill>& getSkills() const { return m_skills; }

    // Derived stats
    float getAttackDamage()  const;
    float getAttackSpeed()   const;
    float getMaxHealth()     const;
    float getMoveSpeed()     const;
    float getFarmYield()     const;

private:
    int                 m_level{1};
    int                 m_xp{0};
    int                 m_skillPoints{0};
    std::vector<Skill>  m_skills;
};
