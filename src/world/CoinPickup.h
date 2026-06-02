#pragma once
#include "pch.h";
#include "core/AssetManager.h";
#include "world/TileMap.h";

struct Coin {
	sf::Vector2f pos;
	int value{ 1 };
	bool collected{ false };
};

class CoinPickup {
public:
	explicit CoinPickup(AssetManager& assets);
	void spawnCoins(sf::Vector2f center, int count,
		unsigned seed, const TileMap& map);
	void reset() { m_coins.clear(); m_accum = 0; }
	void update(sf::Vector2f playerPos);
	int pollCollected();
	void draw(sf::RenderWindow& window);

private:
	std::vector<Coin> m_coins;
	sf::Texture& m_tex;
	int m_accum{ 0 };

	static constexpr float PICKUP_RANGE = 28.f;
};