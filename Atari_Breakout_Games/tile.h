#ifndef TILE_H
#define TILE_H

#include "game_object.h"

class Tile : public GameObject
{
public:
	Tile();
	Tile(std::shared_ptr<Texture2D> tex, glm::vec2 pos, glm::vec2 s, float r = 0.0f, glm::vec2 v = glm::vec2(0.0f), glm::vec3 c = glm::vec3(1.0f));

	void Solid();
	bool IsSolid() const;

private:
	bool isSolid;			// ≈–∂œTile «∑Ò «solid
};

Tile::Tile() : GameObject(), isSolid{ false } { }
Tile::Tile(std::shared_ptr<Texture2D> tex, glm::vec2 pos, glm::vec2 s, float r, glm::vec2 v, glm::vec3 c) : GameObject(tex, pos, s, r, v, c), isSolid{ false } { }

void Tile::Solid()
{
	this->isSolid = true;
}

bool Tile::IsSolid() const
{
	return this->isSolid;
}

#endif // !TILE_H

