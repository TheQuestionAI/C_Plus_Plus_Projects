#ifndef POWER_UP_H
#define POWER_UP_H

#include <string>

#include "game_object.h"

/*
* powerup��������: ����Ϸ������Ϸ�淨����add gameplay mechanic.
* ������һ2D game, ����powerup object, ��ballÿײ����solidש��ʱ, ������һcollision��һ���ĸ��ʲ���һ��powerup, ��һpowerup��paddle"��"��֮��, ��������벻����Ч��.
* ���Ƕ���4�������������ߡ�2�ָ�����������.
		1. Speed����: increases the velocity of the ball by 20%.
		2. Sticky���: when the ball collides with the paddle, the ball remains stuck to the paddle unless the spacebar is pressed again. 
		3. Pass-Through͸��: collision resolution is disabled for non-solid blocks, allowing the ball to pass through multiple blocks.
		4. Pad-Size-Increase����: increases the width of the paddle by 50 pixels.
		5. Confuse��ת: activates the confuse postprocessing effect for a short period of time, confusing the user.
		6. Chaos����: activates the chaos postprocessing effect for a short period of time, heavily disorienting the user.
*
* ���ǿ��Խ� Powerup ��ģΪ����һЩ�������Ե� GameObject, PowerUp����ֻ��һ�����ж���״̬(��Ա����)��GameObject.
* PowerUp�����ά�ֵĺ���state����:
*		1. type: ����PowerUp�����������͵��ַ���.
*		2. duration: ָ��PowerUp����ȡ��Ч���ĳ���ʱ��.
*		3. activated: ָ��PowerUp��ǰ�Ƿ񱻼���. �ر�ע��powerup������ͱ��ݻ�����������.
*			3.1 state destroyed��ʾ���ǵ�ǰpowerup�Ƿ񻹻ᱻ��Ⱦ, �κα�destroyed��object�������ٱ���Ⱦ����Ϸ��.
*			3.2 state activated��ʾ���ǵ�ǰ��powerup��Ч���Ƿ񱻼���, �����destroyed��һ����ϵ�������ȼ�. 
				powerupֻ���ڱ�paddle destroyʱ�Żἤ����Ч��, ���ߵ�Ч���ܹ����������߱��ݻ�֮���һ��ʱ��. ��ͬ�ĵ���Ч������ά����Ч��ʱ��Ҳ�ǲ�һ����.
*/
class PowerUp : public GameObject
{
public:
	PowerUp(std::string typ, std::shared_ptr<Texture2D> tex, glm::vec2 pos, glm::vec2 sz, glm::vec2 vel, glm::vec3 clr, float dur);
public:
	bool IsActivated() const;
	void ShouldActivate(bool state);

	std::string& Type();
	const std::string& Type() const;
	float& Duration();
	const float& Duration() const;
	
private:
	std::string type;
	float duration;

	bool isActivated;
};

PowerUp::PowerUp(std::string typ, std::shared_ptr<Texture2D> tex, glm::vec2 pos, glm::vec2 sz, glm::vec2 vel, glm::vec3 clr, float dur) 
	: GameObject(tex, pos, sz, 0.0f, vel, clr), type{ typ }, duration{ dur }, isActivated{ false } { }

bool PowerUp::IsActivated() const
{
	return this->isActivated;
}
void PowerUp::ShouldActivate(bool state)
{
	this->isActivated = state;
}
std::string& PowerUp::Type()
{
	return this->type;
}
const std::string& PowerUp::Type() const
{
	return this->type;
}
float& PowerUp::Duration()
{
	return this->duration;
}
const float& PowerUp::Duration() const
{
	return this->duration;
}
#endif // !POWER_UP_H

