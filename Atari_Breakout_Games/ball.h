#ifndef BALL_H
#define BALL_H

#include <memory>
#include "game_object.h"
#include "texture.h"

/*
* ����Ball��һobject.
*
* ��Ϊ���а뾶, ��ʼ������paddle��, ��player��paddle��������������ƶ�, ���Զ����GameObject������.
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
	bool isSticky;		// ר��ά��һ����������sticky powerup���к�Ӧ. ��ʼʱû��sticky powerup, ����isSticky��false.
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
	// �����и�С�ط�Ҫ˼����, Move�����Ǽ�����Ϸƽ��ռ�(���½�λ��ê��)���Ǵ��ڿռ�(���Ͻ�λ��Ϊê��)������?
	// ��ȷ����, Ball��position�ǻ�����Ϸƽ�������, ������ȷê����Ball�����½Ƕ���, ����Ϸƽ��ռ����Move.
	// ò���ⲻ��һ������, ��Ϊflip֮��, ���½�ê���flip�����Ͻ�ê����. ���������ռ���������Move�������ȫһ��.
	if (!IsStuck())
	{
		this->position += this->velocity * delta_time;
		// Ball��λ��ê������߱��ϻ����½Ƕ���(��Ϸƽ��ռ�), �����ϽǶ���(���ڿռ�). ��ê����left edge��.
		if (this->position.x <= 0.0f)											// Ball left edge�����������
		{
			this->velocity.x = -this->velocity.x;
			this->position.x = 0.0f;
		}	
		else if (this->position.x + this->size.x >= window_width)				// Ball right edge���������ұ�
		{
			this->velocity.x = -this->velocity.x;
			this->position.x = window_width - this->size.x;
		}
		// ����һ��if����ж�y, �����Ballײ�����ڵĶ�����, ��ôx��y����reverse velocity direction.
		if (this->position.y <= 0.0f)
		{
			this->velocity.y = -this->velocity.y;
			this->position.y = 0.0f;
		}
	}

	return this->position;
}

#endif // !BALL_H

