#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "shader.h"
#include "texture.h"
#include "gl_object.h"
#include "game_object.h"

/*
* Particle System:
* Wiki: ����ϵͳ��ʾ3D�����ͼ��ѧ��ģ��һЩ�ض���ģ������ļ���, ����Щ������������ͳ����Ⱦ��������ʵ�ֵ���ʵ�е���Ϸͼ��.
*		����ʹ������ϵͳģ��������л𡢱�ը���̡�ˮ�����𻨡���Ҷ���ơ���ѩ����������β�������󷢹�켣�����ĳ����Ӿ�Ч���ȵ�.
* ����ʵ��classical implementation: 
		1. ͨ������ϵͳ��3D�ռ��е�λ�����˶����ɷ�����particle emitter���Ƶ�. 
		2. particle emitter��Ҫ��һ��������Ϊ����behavior parameter�Լ�����ά�ռ��е�λ������ʾ.
		3. ������Ϊ�������԰���:
				3.1 ���������ٶ�(����λʱ���������ɵ���Ŀ);
				3.2 ���ӳ�ʼ�ٶ�����(����ʲôʱ����ʲô�����˶�);
				3.3 ��������(�����೤ʱ����������);
				3.4 ������ɫ;
				3.5 ���������������еı仯�Լ����������ȵ�; 
		   ʹ�ô��ֵ�����Ǿ���ֵ��ģ������ռ��ȫ�����߾��󲿷��Ǻ�������, һЩ��������������ֵ�Լ�����ı仯.

*** ���͵�����ϵͳ����ѭ��update loop���Ի���Ϊ������ͬ�Ľ׶�: ��������/ģ��׶� �Լ� ��Ⱦ�׶�. ÿ��ѭ��ִ��ÿһ֡����. ***
* ģ��׶�Simulation phase:
*		1. ���������ٶ��Լ����¼�����������ӵ���Ŀ; 
		2. ÿ�����Ӹ��ݷ�������λ�ü������������������ض�����ά�ռ�λ������, ���Ҹ��ݷ������Ĳ�����ʼ��ÿ�����ӵ��ٶȡ���ɫ���������ڵȵȲ���.
		3. Ȼ����ÿ�������Ƿ��Ѿ���������������, һ�������ͽ���Щ�����޳�ģ�����, ����͸�������ģ��������ӵ�λ��������, 
				3.1 ��Щ����ģ��������ٶȼӵ���ǰλ�û��ߵ����ٶȵ���Ħ��������;
				3.2 Ҳ�������������ǽ�ȡ������ȷ����������켣��������;
				3.3 ����, ������Ҫ�����������ά�������ײ��ʹ���Ӵ��ϰ��ﵯ��; 
				3.4 ��������֮�����ײ�������ܴ��Ҷ��ڴ����ģ����˵û�б�Ҫ, ���Ժ���ʹ������֮�����ײ.
* ��Ⱦ�׶�rendering phase:
*		1. �ڸ������֮��, ͨ��ÿ�������þ�������ӳ����ı���sprite������Ⱦ, Ҳ����˵�ı�����������۲���;
		2. ����������̲��Ǳ����, ��һЩ�ͷֱ��ʻ��ߴ����������޵ĳ������ӿ��ܽ�����Ⱦ��һ������, ��������Ⱦ��������Ⱦ��һ��Ԫ��, ������Ԫ�������ĵ�ֵ����Եõ��൱�õ�Һ�����;
		3. ���⣬Ҳ��������ά������Ⱦ����.
*/

/*
* Particle:
*	���ӱ����Ͼ���һ��size��С�� 2D ����ͼ(����������ͼ���һС����), ���Ҿ���ͼ�Ĵ󲿷�������͸����, Ҳ������ͼ��ʽΪRGBA����alpha channel.
*	����ϵͳ���ǰ�����������ǧ����Щ������һ����Ϊ��ʽ�ۼ���һ��, �ɴ˿��Դ�������˵�Ч��.
*			1. particle��Ȼ�Ǿ���ͼ, ����Ȼ�Ǵ�������һ��ۺ�ʹ��, ����ͬһ���͵����ӱ�Ȼʹ�õ���ͬһ�ž���ͼ��, ������ǿ��԰�texture object�����particle emitter��.
*			2. particle��active sprite, �������Լ�������ֵ, ����������Ҫ���ٶ�velocity��life attribute. ��ô��particle������GameObject��.
*/
struct Particle
{
	glm::vec2 position;
	glm::vec2 size;				// ��������ʹ�õ�����particle��size������ȫ��ͬ, �������attribute�������洢Ҳ��.
	//float rotation:			// ���attribute����������particle���õ�?
	glm::vec2 velocity;
	glm::vec4 color;			// ע��particle��color��4D����, ��alpha channel, ����ͨ�����channel��������ʱ������˥�������Ӿ���ͼ��ɫԽ��Խ����ֱ����ʧ.
	float life;

	Particle() : position{ 0.0f }, size{ 0.0f }, velocity{ 0.0f }, color{ 1.0f }, life{ 0.0f } { }
};

/*
* Particle Emitter:
* �ڴ�������ʱ, ͨ����һ����Ϊ���ӷ�����particle emitter�Ķ���, �����������λ�ò��ϲ�����ʱ��˥����������.
* ���磬������������ӷ������������������״�����΢С����, ���������������particle emitter����ԽԶ, �����ǵ����Ⱦ�Խ����, ��˾Ϳ��Ի�����ƻ����Ч��.
* ��������ͨ����һ����������, ����һ�����������, ���ͻ���ʱ������˥��. һ����������С��ĳ����ֵ(ͨ��Ϊ 0), �����Ӿͻᱻɱ��, �Ա�����һ�����Ӳ���ʱ���������滻��.
* 
*** ���͵����ӷ���������ѭ��update loop���Ի���Ϊ������ͬ�Ľ׶�: ��������/ģ��׶� �Լ� ��Ⱦ�׶�. ÿ��ѭ��ִ��ÿһ֡����. ***
* ��������/ģ��׶�Update() phase:
*		1. ���������ٶ��Լ����¼�����������ӵ���Ŀ;
		2. ÿ�����Ӹ��ݷ�������λ�ü������������������ض�����ά�ռ�λ������, ���Ҹ��ݷ������Ĳ�����ʼ��ÿ�����ӵ��ٶȡ���ɫ���������ڵȵȲ���.
		3. Ȼ����ÿ�������Ƿ��Ѿ���������������, һ�������ͽ���Щ�����޳�ģ�����, ����͸�������ģ��������ӵ�λ��������,
				3.1 ��Щ����ģ��������ٶȼӵ���ǰλ�û��ߵ����ٶȵ���Ħ��������;
				3.2 Ҳ�������������ǽ�ȡ������ȷ����������켣��������;
				3.3 ����, ������Ҫ�����������ά�������ײ��ʹ���Ӵ��ϰ��ﵯ��;
				3.4 ��������֮�����ײ�������ܴ��Ҷ��ڴ����ģ����˵û�б�Ҫ, ���Ժ���ʹ������֮�����ײ.
* ��Ⱦ�׶�Draw() phase:
*		1. �ڸ������֮��, ͨ��ÿ�������þ�������ӳ����ı���sprite������Ⱦ, Ҳ����˵�ı�����������۲���;
		2. ����������̲��Ǳ����, ��һЩ�ͷֱ��ʻ��ߴ����������޵ĳ������ӿ��ܽ�����Ⱦ��һ������, ��������Ⱦ��������Ⱦ��һ��Ԫ��, ������Ԫ�������ĵ�ֵ����Եõ��൱�õ�Һ�����;
		3. ���⣬Ҳ��������ά������Ⱦ����.
*/
class ParticleEmitter
{
public:
	ParticleEmitter(std::shared_ptr<Shader> sd, std::shared_ptr<Texture2D> tex, unsigned int total);
public:
	// ���͵����ӷ���������ѭ��update loop���Ի���Ϊ������ͬ�Ľ׶�: ��������/ģ��׶� �Լ� ��Ⱦ�׶�. ÿ��ѭ��ִ��ÿһ֡����.
	void Update(float delta_time, const GameObject& object, glm::vec2 particle_pos_offset, unsigned int new_spawn_particle = 2);
	// ������Ҫposition��Ϣ, ����position�ɴ��ݽ�����GameObject����. �������ϵͳ��ģ�����ǵ���β�����ǻ������β. "����"���Ǵ��ݽ�����GameObject.
	void Draw();
private:
	std::vector<Particle> particles;
	unsigned int lastUsedParticle;

	// ���ӷ�������Ϊһ��ϵͳ, ����������ⲿ��Ⱦ�����������ӻ��ƹ���, �����������������ӻ��ƹ���.
	std::shared_ptr<Shader> shader;
	std::shared_ptr<Texture2D> texture;	

	VAO vao;
	VBO vbo;

	unsigned int FirstUnUsedParticle();
	void ReSpawnParticle(Particle& particle, const GameObject& object, glm::vec2 offset);
};

ParticleEmitter::ParticleEmitter(std::shared_ptr<Shader> sd, std::shared_ptr<Texture2D> tex, unsigned int total) 
{
	this->shader = sd;
	this->texture = tex;
	this->particles.resize(total);
	this->lastUsedParticle = 0;

	// ��ʵ������������������ݴ洢��GPU buffer object��.
	// sprite uses the same original square shape. ����ʹ�������ģʽ�е�ʵ��ģʽ/��Ԫģʽ. 
	float square[] = {            // ������ʵ������������½Ƕ���������ԭ���(0,0)��λ������. �������ĸ������Ӧ�ĸ���ͬ����������.
		// pos      // tex coord
		0.0f, 1.0f, 0.0f, 1.0f,     // ֮��Ҫ������ͨ��model����Ի��������ν���ʵ����, ת����һ����δ��ɫǰ�ľ���ͼ. 
		1.0f, 0.0f, 1.0f, 0.0f,     // ���ڲ�δִ��projection matrix, ��������û��flip�ɴ��ڿռ�����, �������������½Ƕ���Ϊê��ê��ԭ��(0,0)λ��.
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	this->vao.Bind();
	this->vbo.Bind();
	this->vbo.LoadVertexData(sizeof(square), square);
	this->vbo.ConfigureVertexAttribute(0, 4, GL_FLOAT, 4 * sizeof(float), 0);
	this->vbo.UnBind();
	this->vao.UnBind();
}

// ���͵����ӷ���������ѭ��update loop���Ի���Ϊ������ͬ�Ľ׶�: ��������/ģ��׶� �Լ� ��Ⱦ�׶�. ÿ��ѭ��ִ��ÿһ֡����.
// ������Ҫposition��Ϣ, ����position�ɴ��ݽ�����GameObject����. �������ϵͳ��ģ�����ǵ���β�����ǻ������β. "����"���Ǵ��ݽ�����GameObject.
void ParticleEmitter::Update(float delta_time, const GameObject& object, glm::vec2 particle_pos_offset, unsigned int new_spawn_particle)
{
	for (unsigned int idx = 0; idx < new_spawn_particle; ++idx)
	{
		ReSpawnParticle(this->particles[FirstUnUsedParticle()], object, particle_pos_offset);
	}

	for (auto& particle : this->particles)
	{
		particle.life -= delta_time;		// ����˥������ʱ��.
		if (particle.life > 0.0f)			// �ж������Ƿ���active״̬.
		{
			particle.position -= particle.velocity * delta_time;
			particle.color.a -= 2.5f * delta_time;      // alpha channel value decay.   ���Ĵ������, alpha����˥����. ��˴���shader��color������vec4.
		}
	}
}

void ParticleEmitter::Draw()
{
	// use additive blending to give it a 'glow' effect
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	this->shader->Use();
	this->shader->SetInteger("image", this->texture->UnitID());

	this->vao.Bind();
	for (auto& particle : particles)
	{
		if (particle.life > 0.0f)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(particle.position, 0.0f));
			model = glm::scale(model, glm::vec3(particle.size, 0.0f));

			this->shader->SetMat4x4("model", model);
			this->shader->SetVector4f("color", particle.color);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
	this->vao.UnBind();
	
	// don't forget to reset to default blending mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

unsigned int ParticleEmitter::FirstUnUsedParticle()
{
	for (unsigned int idx = this->lastUsedParticle; idx < this->particles.size(); ++idx)
	{
		if (this->particles[idx].life <= 0.0f)
		{
			this->lastUsedParticle = idx;
			return idx;
		}
	}

	for (unsigned int idx = 0; idx < this->lastUsedParticle; ++idx)
	{
		if(this->particles[idx].life <= 0.0f)
		{
			this->lastUsedParticle = idx;
			return idx;
		}
	}

	// all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
	this->lastUsedParticle = 0;
	return 0;
}

void ParticleEmitter::ReSpawnParticle(Particle& particle, const GameObject& object, glm::vec2 offset)
{
	// rand(): Returns a pseudo-random integral number in the range between 0 and RAND_MAX.
	float random = ((rand() % 100) - 50) / 10.0f;       // ȡ[-5,+5)�����ڵ����������.
	float color = 0.5f + ((rand() % 50) / 100.0f);      // ȡ[0.5f, 1.0f]֮�������ɫ.

	particle.position = object.Position() + offset + random;		// particleλ��Ϊobject��λ�� + ƫ��ֵ + ���ֵ
	particle.velocity = object.Velocity() * 0.1f;					// ȡobject�ٶȵ�fraction. ����.
	particle.size = object.Size() * 0.9f;							// ȡobject size��0.9��.
	particle.color = glm::vec4(color, color, color, 1.0f);			// ���ɵ����ӵ�RGB��ɫֵ�������[0.5f, 1.0f]֮��.
	particle.life = 1.0f;											// ��ʼ����ֵΪ1.0, ���Ȱ���delta time�ݼ�.	
}

#endif // !PARTICLE_EMITTER_H

