#pragma once
#include "pch.h"
#include "systems/Inventory.h"

// ========================= CURRENCY =========================

struct Wallet {
    int gold{ 0 };
    int silver{ 0 };
    int emerald{ 0 };

    int toSilver() const { return gold * 100 + silver; }

    void addSilver(int amount) {
        silver += amount;
        if (silver >= 100) { gold += silver / 100; silver %= 100; }
        if (silver < 0) {
            int borrow = (-silver + 99) / 100;
            gold -= borrow; silver += borrow * 100;
        }
    }
    void addGold(int amount) { gold += amount; }
    void addEmerald(int amount) { emerald += amount; }

    bool canAfford(int silverCost) const { return toSilver() >= silverCost; }
};

// ========================= MARKET ENTRY =========================

struct MarketEntry {
    ItemType    item;
    std::string name;
    std::string description;

    int  baseBuyPrice{ 10 };
    int  baseSellPrice{ 6 };
    int  currentBuyPrice{ 10 };
    int  currentSellPrice{ 6 };

    float volatility{ 0.2f };
    int   recentSells{ 0 };
    int   recentBuys{ 0 };

    bool canBuy{ true };
    bool canSell{ true };

    sf::Color color{ sf::Color::White };
};

// ========================= MARKET SYSTEM =========================

class MarketSystem {
public:
    MarketSystem();

    void update(float dt, float gameHour);
    void forceRecalc(float gameHour); // poziva se svake sekunde iz MarketUI

    bool buyItem(ItemType item, int count, Wallet& wallet, class Inventory& inv);
    bool sellItem(ItemType item, int count, Wallet& wallet, class Inventory& inv);

    const MarketEntry* getEntry(ItemType item) const;
    const std::vector<MarketEntry>& getEntries() const { return m_entries; }

    static std::string formatPrice(int silver);
    static float       getDayPriceMult(float gameHour);
    static std::string formatWallet(const Wallet& w);

    struct PricePoint { float time; int buyPrice; };
    const std::vector<PricePoint>& getHistory(ItemType item) const;

private:
    void recalcPrices(float gameHour);
    void recordHistory(ItemType item, float gameHour);

    std::vector<MarketEntry>                        m_entries;
    std::unordered_map<int, std::vector<PricePoint>> m_history;

    float        m_fluctTimer{ 0.f };
    float        m_fluctInterval{ 30.f };
    std::mt19937 m_rng;
};