#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <memory>
#include <glm/glm.hpp>
#include "texture.h"
#include "sprite_renderer.h"

/*
* GameObject���Կ����Ǿ���ͼsprite��������, �������������Ի�ȫ����������ͼ�е�����, ����֮�����Ӷ���ͨ�õ�GameObject����.
* GameObject������Game������object�Ļ���, ������Ϸ�е�objectҪô��GameObjectҪô����������.
* �����е�game object, ���Ƕ���ͳһ�Ļ��Ƹ�GameObject�Ľӿ�. �ýӿڽ���һ����Ⱦ��, �ڽӿ��ڲ������ø���Ⱦ��������GameObject.
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
	void Draw(SpriteRenderer& sprite_renderer);			// ʹ������ģʽ, GameObject����ά�ֻ��������һ������, Ȼ������Ⱦ��ʹ�õ�һְ�����Ⱦ�����ж�GameObject�Ļ���. 

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
	std::shared_ptr<Texture2D> texture;		// 2D GameObject, ������������һ��2D image.

	glm::vec2 position;
	glm::vec2 size;
	float rotation;

	glm::vec2 velocity;
	glm::vec3 color;

	bool isDestoryed;		// �ж�GameObject�Ƿ�"�ݻ�"��, �ݻ��˾Ͳ����ٱ���Ⱦ��ͼ����ȥ.
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


