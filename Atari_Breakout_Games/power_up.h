#ifndef POWER_UP_H
#define POWER_UP_H

#include <string>

#include "game_object.h"

/*
* powerup能量道具: 对游戏加入游戏玩法机制add gameplay mechanic.
* 对于这一2D game, 加入powerup object, 当ball每撞击非solid砖块时, 就让这一collision有一定的概率产生一个powerup, 这一powerup被paddle"吃"掉之后, 会带来意想不到的效果.
* 我们定义4种正面能量道具、2种负面能量道具.
		1. Speed加速: increases the velocity of the ball by 20%.
		2. Sticky黏着: when the ball collides with the paddle, the ball remains stuck to the paddle unless the spacebar is pressed again. 
		3. Pass-Through透穿: collision resolution is disabled for non-solid blocks, allowing the ball to pass through multiple blocks.
		4. Pad-Size-Increase增大: increases the width of the paddle by 50 pixels.
		5. Confuse倒转: activates the confuse postprocessing effect for a short period of time, confusing the user.
		6. Chaos混沌: activates the chaos postprocessing effect for a short period of time, heavily disorienting the user.
*
* 我们可以将 Powerup 建模为具有一些额外属性的 GameObject, PowerUp本质只是一个具有额外状态(成员变量)的GameObject.
* PowerUp类额外维持的核心state包含:
*		1. type: 定义PowerUp所属道具类型的字符串.
*		2. duration: 指定PowerUp被获取后效果的持续时间.
*		3. activated: 指定PowerUp当前是否被激活. 特别注意powerup被激活和被摧毁是两个概念.
*			3.1 state destroyed表示的是当前powerup是否还会被渲染, 任何被destroyed的object都不会再被渲染在游戏中.
*			3.2 state activated表示的是当前的powerup的效果是否被激活, 激活跟destroyed有一定关系但并不等价. 
				powerup只有在被paddle destroy时才会激活其效果, 道具的效果能够持续到道具被摧毁之后的一段时间. 不同的道具效果激活维持有效的时间也是不一样的.
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

