#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <memory>
#include <glm/glm.hpp>
#include "texture.h"
#include "sprite_renderer.h"

/*
* GameObject可以看做是精灵图sprite的升级版, 其所包含的属性会全部囊括精灵图有的属性, 并在之上增加额外通用的GameObject属性.
* GameObject将会是Game中所有object的基类, 所有游戏中的object要么是GameObject要么是它的子类.
* 对所有的game object, 我们定义统一的绘制该GameObject的接口. 该接口接受一个渲染器, 在接口内部将调用该渲染器来绘制GameObject.
* Game object attribute:
*		1. texture.
*		2. position.
*		3. size.
*		4. rotation.
*		5. velocity.
*		6. color.
*		7. isDestroyed.
*		8. ...customized attributes
*/
class GameObject		// checked, no need to check again!
{
public:
	GameObject();
	GameObject(std::shared_ptr<Texture2D> tex, glm::vec2 pos, glm::vec2 sz, float rot = 0.0f, glm::vec2 vel = glm::vec2(0.0f), glm::vec3 clr = glm::vec3(1.0f));
public:
	void Draw(SpriteRenderer& sprite_renderer);			// 使用命令模式, GameObject本身维持绘制所需的一切数据, 然后传入渲染器使用单一职责的渲染器进行对GameObject的绘制. 

	void Destroy();
	bool IsDestroyed() const;

public:
	std::shared_ptr<Texture2D>& Texture() { return this->texture; }
	glm::vec2& Position() { return this->position; }
	glm::vec2& Size() { return this->size; }
	glm::vec2& Velocity() { return this->velocity; }
	glm::vec3& Color() { return this->color; }

	const std::shared_ptr<Texture2D>& Texture() const { return this->texture; }
	const glm::vec2& Position() const { return this->position; }
	const glm::vec2& Size() const { return this->size; }
	const glm::vec2& Velocity() const { return this->velocity; }
	const glm::vec3& Color() const { return this->color; }
protected:
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ general attribute ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	std::shared_ptr<Texture2D> texture;		// 2D GameObject, 所以其纹理是一张2D image.

	glm::vec2 position;
	glm::vec2 size;
	float rotation;

	glm::vec2 velocity;
	glm::vec3 color;

	bool isDestoryed;		// 判断GameObject是否被"摧毁"了, 摧毁了就不会再被渲染到图像中去.
};

GameObject::GameObject() : texture{ nullptr }, position{ 0.0f }, size{ 0.0f }, rotation{ 0.0f }, velocity{ 0.0f }, color{ 0.0f }, isDestoryed{ false } { }

GameObject::GameObject(std::shared_ptr<Texture2D> tex, glm::vec2 pos, glm::vec2 sz, float rot, glm::vec2 vel, glm::vec3 clr)
	: texture{ tex }, position{ pos }, size{ sz }, rotation{ rot }, velocity{ vel }, color{ clr }, isDestoryed{ false } { }

void GameObject::Draw(SpriteRenderer& sprite_renderer)
{
	sprite_renderer.DrawSprite(this->texture, this->position, this->size, this->rotation, this->color);
}

void GameObject::Destroy()
{
	this->isDestoryed = true;
}

bool GameObject::IsDestroyed() const
{
	return this->isDestoryed;
}

#endif // !GAME_OBJECT_H


