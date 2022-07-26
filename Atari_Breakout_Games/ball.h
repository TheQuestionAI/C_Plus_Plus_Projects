#ifndef BALL_H
#define BALL_H

#include <memory>
#include "game_object.h"
#include "texture.h"

/*
* 定义Ball这一object.
*
* 因为它有半径, 初始附着在paddle上, 被player从paddle弹出后可以自由移动, 所以定义成GameObject的子类.
*/
class Ball : public GameObject
{
public:
	Ball(std::shared_ptr<Texture2D> tex, glm::vec2 pos, float rdis, glm::vec2 vel = glm::vec2(0.0f), glm::vec3 clr = glm::vec3(1.0f));
public:
	float Radius() const;
	glm::vec2 InitVelocity() const;

	bool IsStuck() const;
	bool IsSticky() const;
	bool IsPassThrough() const;
	void ShouldStuck(bool state);
	void ShouldSticky(bool state);
	void ShouldPassThrough(bool state);

	void ReSet(glm::vec2 pos, glm::vec2 v);
	
	glm::vec2 Move(float delta_time, unsigned int window_width, unsigned int window_height);
private:
	float radius;
	bool isStuck;
	bool isSticky;		// 专门维持一个变量来对sticky powerup进行呼应. 初始时没有sticky powerup, 所有isSticky是false.
	bool isPassThrough;
	glm::vec2 initVelocity;
};

Ball::Ball(std::shared_ptr<Texture2D> tex, glm::vec2 pos, float rdis, glm::vec2 vel, glm::vec3 clr)
	: GameObject(tex, pos, glm::vec2(2.0f * rdis), 0.0f, vel, clr), radius{ rdis }, isStuck{ true }, isSticky{ false }, isPassThrough{ false }, initVelocity{ vel } { }

bool Ball::IsStuck() const
{
	return this->isStuck;
}
bool Ball::IsSticky() const
{
	return this->isSticky;
}
bool Ball::IsPassThrough() const
{
	return this->isPassThrough;
}
void Ball::ShouldStuck(bool state)
{
	this->isStuck = state;
}
void Ball::ShouldSticky(bool state)
{
	this->isSticky = state;
}
void Ball::ShouldPassThrough(bool state)
{
	this->isPassThrough = state;
}
void Ball::ReSet(glm::vec2 pos, glm::vec2 v)
{
	this->position = pos;
	this->velocity = v;
	this->isStuck = true;
}

float Ball::Radius() const
{
	return this->radius;
}

glm::vec2 Ball::InitVelocity() const
{
	return this->initVelocity;
}

glm::vec2 Ball::Move(float delta_time, unsigned int window_width, unsigned int window_height)
{
	// 这里有个小地方要思考下, Move到底是继续游戏平面空间(左下角位置锚点)还是窗口空间(左上角位置为锚点)来计算?
	// 正确答案是, Ball的position是基于游戏平面给出的, 所以正确锚点是Ball的左下角顶点, 在游戏平面空间计算Move.
	// 貌似这不是一个问题, 因为flip之后, 左下角锚点就flip成左上角锚点了. 所以两个空间计算出来的Move结果是完全一样.
	if (!IsStuck())
	{
		this->position += this->velocity * delta_time;
		// Ball的位置锚点是左边边上或左下角顶点(游戏平面空间), 或左上角顶点(窗口空间). 即锚点在left edge上.
		if (this->position.x <= 0.0f)											// Ball left edge触及窗口左边
		{
			this->velocity.x = -this->velocity.x;
			this->position.x = 0.0f;
		}	
		else if (this->position.x + this->size.x >= window_width)				// Ball right edge触及窗口右边
		{
			this->velocity.x = -this->velocity.x;
			this->position.x = window_width - this->size.x;
		}
		// 另起一个if语句判断y, 即如果Ball撞到窗口的顶点上, 那么x和y都会reverse velocity direction.
		if (this->position.y <= 0.0f)
		{
			this->velocity.y = -this->velocity.y;
			this->position.y = 0.0f;
		}
	}

	return this->position;
}

#endif // !BALL_H

