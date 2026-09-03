#include "systems/MarketSystem.h"
#include "systems/Inventory.h"
#include <cmath>
#include <algorithm>
#include <sstream>

MarketSystem::MarketSystem()
    : m_rng(std::random_device{}())
{
    // ---- Definicija svih trzinih stavki ----
    // { item, name, desc, baseBuy, baseSell, volatility, canBuy, canSell, color }

    auto make = [](ItemType it, const char* name, const char* desc,
        int buy, int sell, float vol,
        bool canBuy, bool canSell, sf::Color col) {
            MarketEntry e;
            e.item = it;
            e.name = name;
            e.description = desc;
            e.baseBuyPrice = buy;
            e.baseSellPrice = sell;
            e.currentBuyPrice = buy;
            e.currentSellPrice = sell;
            e.volatility = vol;
            e.canBuy = canBuy;
            e.canSell = canSell;
            e.color = col;
            return e;
        };

    // ---- Farming ----
    m_entries.push_back(make(
        ItemType::Seed, "Seeds", "Plant to grow crops.",
        8, 3, 0.10f, true, true,
        sf::Color(180, 140, 60)));
    m_entries.push_back(make(
        ItemType::Crop, "Crops", "Harvested produce.",
        12, 8, 0.35f, true, true,
        sf::Color(240, 200, 0)));
    m_entries.push_back(make(
        ItemType::Wood, "Wood", "Lumber for building.",
        15, 9, 0.20f, true, true,
        sf::Color(139, 90, 43)));
    m_entries.push_back(make(
        ItemType::Stone, "Stone", "Construction material.",
        18, 11, 0.12f, true, true,
        sf::Color(160, 160, 160)));
    m_entries.push_back(make(
        ItemType::CropPoor, "Poor Crop", "Poorly grown produce.",
        1, 4, 0.15f, false, true,
        sf::Color(150, 130, 40)));

    // ---- Equipment ----
    m_entries.push_back(make(
        ItemType::FishingRod, "Fishing Rod", "Required to fish near water.",
        120, 40, 0.05f, true, true,
        sf::Color(80, 160, 220)));
    m_entries.push_back(make(
        ItemType::Hoe, "Hoe", "Required to till soil.",
        160, 60, 0.05f, true, true,
        sf::Color(140, 150, 220)));

    // ---- Fish (prodaja only) ----
    m_entries.push_back(make(
        ItemType::Carp, "Carp",
        "A common river fish.",
        8, 6, 0.08f, false, true,
        sf::Color(80, 160, 100)));
    m_entries.push_back(make(
        ItemType::Trout, "Trout",
        "A fast mountain fish.",
        15, 12, 0.06f, false, true,
        sf::Color(60, 120, 200)));
    m_entries.push_back(make(
        ItemType::Salmon, "Salmon",
        "A valuable pink fish.",
        28, 22, 0.05f, false, true,
        sf::Color(220, 100, 80)));
    m_entries.push_back(make(
        ItemType::Catfish, "Catfish",
        "A rare bottom dweller.",
        40, 32, 0.04f, false, true,
        sf::Color(100, 80, 140)));
    m_entries.push_back(make(
        ItemType::GoldenFish, "Golden Fish",
        "Legendary. Worth a fortune.",
        90, 75, 0.02f, false, true,
        sf::Color(255, 200, 0)));
}

// -------------------------------------------------------
//  Multiplikator cijene po dobu dana
//  Ujutro (7-10) i uvece (17-20) cijene usijeva su vise
//  Noc smanjuje vecinu cijena osim Stone/Wood
// -------------------------------------------------------
float MarketSystem::getDayPriceMult(float gameHour) {
    // Jutarnji rush
    if (gameHour >= 7.f && gameHour < 10.f) {
        float t = (gameHour - 7.f) / 3.f;
        float peak = std::sin(t * 3.14159f); // 0→1→0
        return 1.f + 0.25f * peak;
    }
    // Vjecernji rush
    if (gameHour >= 17.f && gameHour < 20.f) {
        float t = (gameHour - 17.f) / 3.f;
        float peak = std::sin(t * 3.14159f);
        return 1.f + 0.35f * peak;
    }
    // Noc — diskonti
    if (gameHour >= 21.f || gameHour < 5.f)
        return 0.75f;
    // Podne — blagi bonus
    if (gameHour >= 11.f && gameHour < 14.f)
        return 1.10f;

    return 1.0f;
}

void MarketSystem::recalcPrices(float gameHour) {
    float dayMult = getDayPriceMult(gameHour);
    std::uniform_real_distribution<float> noise(-1.f, 1.f);

    for (auto& e : m_entries) {
        // Random fluktuacija ogranicena volatilnoscu
        float rnd = noise(m_rng) * e.volatility;

        // Demand efekt: vise prodaje = niza cijena, vise kupnje = visa cijena
        float demandShift = 0.f;
        if (e.recentBuys + e.recentSells > 0) {
            float ratio = static_cast<float>(e.recentBuys - e.recentSells) /
                static_cast<float>(e.recentBuys + e.recentSells + 1);
            demandShift = ratio * 0.15f; // max ±15% od demanda
        }

        float mult = dayMult * (1.f + rnd + demandShift);
        mult = std::max(0.5f, std::min(2.5f, mult)); // hard cap 50%–250%

        e.currentBuyPrice = std::max(1, static_cast<int>(e.baseBuyPrice * mult));
        e.currentSellPrice = std::max(1, static_cast<int>(e.baseSellPrice * mult * 0.85f));

        // Sell uvijek manji od buy (spread)
        if (e.currentSellPrice >= e.currentBuyPrice)
            e.currentSellPrice = e.currentBuyPrice - 1;

        // Polako decay demand statistike
        e.recentBuys = static_cast<int>(e.recentBuys * 0.8f);
        e.recentSells = static_cast<int>(e.recentSells * 0.8f);
    }
}

void MarketSystem::recordHistory(ItemType item, float gameHour) {
    int key = static_cast<int>(item);
    auto& hist = m_history[key];

    const MarketEntry* e = getEntry(item);
    if (!e) return;

    hist.push_back({ gameHour, e->currentBuyPrice });

    // Cuvaj samo zadnjih 24 tocaka (jedna po satu igre otprilike)
    if (hist.size() > 24)
        hist.erase(hist.begin());
}

void MarketSystem::forceRecalc(float gameHour) {
    recalcPrices(gameHour);
    for (auto& e : m_entries)
        recordHistory(e.item, gameHour);
}

void MarketSystem::update(float dt, float gameHour) {
    m_fluctTimer += dt;
    if (m_fluctTimer >= m_fluctInterval) {
        m_fluctTimer = 0.f;
        recalcPrices(gameHour);
        // Zapisi historiju za sve stavke
        for (auto& e : m_entries)
            recordHistory(e.item, gameHour);
    }
}

// -------------------------------------------------------
//  Kupovina
// -------------------------------------------------------
bool MarketSystem::buyItem(ItemType item, int count, Wallet& wallet, Inventory& inv) {
    MarketEntry* e = nullptr;
    for (auto& entry : m_entries)
        if (entry.item == item) { e = &entry; break; }

    if (!e || !e->canBuy) return false;

    int totalCost = e->currentBuyPrice * count;
    if (!wallet.canAfford(totalCost)) return false;
    if (!inv.addItem(item, count)) return false;

    wallet.addSilver(-totalCost);
    e->recentBuys += count;
    return true;
}

// -------------------------------------------------------
//  Prodaja
// -------------------------------------------------------
bool MarketSystem::sellItem(ItemType item, int count, Wallet& wallet, Inventory& inv) {
    MarketEntry* e = nullptr;
    for (auto& entry : m_entries)
        if (entry.item == item) { e = &entry; break; }

    if (!e || !e->canSell) return false;
    if (inv.countOf(item) < count) return false;
    if (!inv.removeItem(item, count)) return false;

    int totalGain = e->currentSellPrice * count;
    wallet.addSilver(totalGain);
    e->recentSells += count;
    return true;
}

// -------------------------------------------------------
//  Getteri
// -------------------------------------------------------
const MarketEntry* MarketSystem::getEntry(ItemType item) const {
    for (auto& e : m_entries)
        if (e.item == item) return &e;
    return nullptr;
}

const std::vector<MarketSystem::PricePoint>&
MarketSystem::getHistory(ItemType item) const {
    static std::vector<PricePoint> empty;
    auto it = m_history.find(static_cast<int>(item));
    if (it == m_history.end()) return empty;
    return it->second;
}

// -------------------------------------------------------
//  Formatiranje
// -------------------------------------------------------
std::string MarketSystem::formatPrice(int silver) {
    if (silver <= 0) return "0s";
    if (silver < 100) return std::to_string(silver) + "s";
    int g = silver / 100;
    int s = silver % 100;
    std::string out = std::to_string(g) + "g";
    if (s > 0) out += " " + std::to_string(s) + "s";
    return out;
}

std::string MarketSystem::formatWallet(const Wallet& w) {
    std::string out;
    if (w.emerald > 0) out += std::to_string(w.emerald) + "em  ";
    if (w.gold > 0) out += std::to_string(w.gold) + "g ";
    out += std::to_string(w.silver) + "s";
    return out;
}