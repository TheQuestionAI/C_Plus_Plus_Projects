#ifndef PLAYER_H
#define PLAYER_H

#include "game_object.h"

class Player : public GameObject
{
public: 
	Player();
	Player(std::shared_ptr<Texture2D> tex, glm::vec2 pos, glm::vec2 sz, float vx, glm::vec3 clr = glm::vec3(1.0f));

	// 这样, 任何由user input引起的物体位置变化, 都不定义member function, 而是直接对position进行直接计算操作.
	// void Move(float delta_time, unsigned int window_width, unsigned int window_height);
};

Player::Player() : GameObject() { }
Player::Player(std::shared_ptr<Texture2D> tex, glm::vec2 pos, glm::vec2 sz, float vx, glm::vec3 clr) : GameObject(tex, pos, sz, 0.0f, glm::vec2(std::abs(vx), 0.0f), clr) { }

#endif // !PLAYER_H

