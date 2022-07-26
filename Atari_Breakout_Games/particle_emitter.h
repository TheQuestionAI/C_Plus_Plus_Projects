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
* Wiki: 粒子系统表示3D计算机图形学中模拟一些特定的模糊现象的技术, 而这些现象用其它传统的渲染技术难以实现的真实感的游戏图形.
*		经常使用粒子系统模拟的现象有火、爆炸、烟、水流、火花、落叶、云、雾、雪、尘、流星尾迹或者象发光轨迹这样的抽象视觉效果等等.
* 典型实现classical implementation: 
		1. 通常粒子系统在3D空间中的位置与运动是由发射器particle emitter控制的. 
		2. particle emitter主要由一组粒子行为参数behavior parameter以及在三维空间中的位置所表示.
		3. 粒子行为参数可以包括:
				3.1 粒子生成速度(即单位时间粒子生成的数目);
				3.2 粒子初始速度向量(例如什么时候向什么方向运动);
				3.3 粒子寿命(经过多长时间粒子湮灭);
				3.4 粒子颜色;
				3.5 在粒子生命周期中的变化以及其它参数等等; 
		   使用大概值而不是绝对值的模糊参数占据全部或者绝大部分是很正常的, 一些参数定义了中心值以及允许的变化.

*** 典型的粒子系统更新循环update loop可以划分为两个不同的阶段: 参数更新/模拟阶段 以及 渲染阶段. 每个循环执行每一帧动画. ***
* 模拟阶段Simulation phase:
*		1. 根据生成速度以及更新间隔计算新粒子的数目; 
		2. 每个粒子根据发射器的位置及给定的生成区域在特定的三维空间位置生成, 并且根据发射器的参数初始化每个粒子的速度、颜色、生命周期等等参数.
		3. 然后检查每个粒子是否已经超出了生命周期, 一旦超出就将这些粒子剔出模拟过程, 否则就根据物理模拟更改粒子的位置与特性, 
				3.1 这些物理模拟可能象将速度加到当前位置或者调整速度抵消摩擦这样简单;
				3.2 也可能象将外力考虑进取计算正确的物理抛射轨迹那样复杂;
				3.3 另外, 经常需要检查与特殊三维物体的碰撞以使粒子从障碍物弹回; 
				3.4 由于粒子之间的碰撞计算量很大并且对于大多数模拟来说没有必要, 所以很少使用粒子之间的碰撞.
* 渲染阶段rendering phase:
*		1. 在更新完成之后, 通常每个例子用经过纹理映射的四边形sprite进行渲染, 也就是说四边形总是面向观察者;
		2. 但是这个过程不是必须的, 在一些低分辨率或者处理能力有限的场合粒子可能仅仅渲染成一个像素, 在离线渲染中甚至渲染成一个元球, 从粒子元球计算出的等值面可以得到相当好的液体表面;
		3. 另外，也可以用三维网格渲染粒子.
*/

/*
* Particle:
*	粒子本质上就是一个size很小的 2D 精灵图(或者是纹理图像的一小部分), 并且精灵图的大部分区域是透明的, 也即精灵图格式为RGBA包含alpha channel.
*	粒子系统就是把数百甚至数千个这些粒子以一定行为方式聚集在一起, 由此可以创造出惊人的效果.
*			1. particle虽然是精灵图, 但必然是大批量的一起聚合使用, 并且同一类型的粒子必然使用的是同一张精灵图像, 因此我们可以把texture object存放在particle emitter中.
*			2. particle是active sprite, 并且有自己的生命值, 所以我们需要有速度velocity和life attribute. 这么看particle更像是GameObject啊.
*/
struct Particle
{
	glm::vec2 position;
	glm::vec2 size;				// 这里我们使用的所有particle的size都会完全相同, 所以这个attribute不单独存储也可.
	//float rotation:			// 这个attribute基本很难在particle中用到?
	glm::vec2 velocity;
	glm::vec4 color;			// 注意particle的color是4D向量, 带alpha channel, 我们通过这个channel粒子随着时间流逝衰减让粒子精灵图颜色越来越减弱直至消失.
	float life;

	Particle() : position{ 0.0f }, size{ 0.0f }, velocity{ 0.0f }, color{ 1.0f }, life{ 0.0f } { }
};

/*
* Particle Emitter:
* 在处理粒子时, 通常有一个称为粒子发射器particle emitter的对象, 它会从其所在位置不断产生随时间衰减的新粒子.
* 例如，如果这样的粒子发射器会产生具有烟雾状纹理的微小粒子, 则发射出来的粒子离particle emitter距离越远, 则它们的亮度就越减弱, 如此就可以获得类似火焰的效果.
* 单个粒子通常有一个生命变量, 粒子一旦被创造出来, 它就会随时间慢慢衰减. 一旦它的寿命小于某个阈值(通常为 0), 该粒子就会被杀死, 以便在下一个粒子产生时用新粒子替换它.
* 
*** 典型的粒子发射器更新循环update loop可以划分为两个不同的阶段: 参数更新/模拟阶段 以及 渲染阶段. 每个循环执行每一帧动画. ***
* 参数更新/模拟阶段Update() phase:
*		1. 根据生成速度以及更新间隔计算新粒子的数目;
		2. 每个粒子根据发射器的位置及给定的生成区域在特定的三维空间位置生成, 并且根据发射器的参数初始化每个粒子的速度、颜色、生命周期等等参数.
		3. 然后检查每个粒子是否已经超出了生命周期, 一旦超出就将这些粒子剔出模拟过程, 否则就根据物理模拟更改粒子的位置与特性,
				3.1 这些物理模拟可能象将速度加到当前位置或者调整速度抵消摩擦这样简单;
				3.2 也可能象将外力考虑进取计算正确的物理抛射轨迹那样复杂;
				3.3 另外, 经常需要检查与特殊三维物体的碰撞以使粒子从障碍物弹回;
				3.4 由于粒子之间的碰撞计算量很大并且对于大多数模拟来说没有必要, 所以很少使用粒子之间的碰撞.
* 渲染阶段Draw() phase:
*		1. 在更新完成之后, 通常每个例子用经过纹理映射的四边形sprite进行渲染, 也就是说四边形总是面向观察者;
		2. 但是这个过程不是必须的, 在一些低分辨率或者处理能力有限的场合粒子可能仅仅渲染成一个像素, 在离线渲染中甚至渲染成一个元球, 从粒子元球计算出的等值面可以得到相当好的液体表面;
		3. 另外，也可以用三维网格渲染粒子.
*/
class ParticleEmitter
{
public:
	ParticleEmitter(std::shared_ptr<Shader> sd, std::shared_ptr<Texture2D> tex, unsigned int total);
public:
	// 典型的粒子发射器更新循环update loop可以划分为两个不同的阶段: 参数更新/模拟阶段 以及 渲染阶段. 每个循环执行每一帧动画.
	void Update(float delta_time, const GameObject& object, glm::vec2 particle_pos_offset, unsigned int new_spawn_particle = 2);
	// 粒子需要position信息, 而这position由传递进来的GameObject决定. 这个粒子系统是模拟彗星的拖尾或者是火球的拖尾. "彗星"就是传递进来的GameObject.
	void Draw();
private:
	std::vector<Particle> particles;
	unsigned int lastUsedParticle;

	// 粒子发射器作为一个系统, 将不会借助外部渲染器来进行粒子绘制工作, 本身将具有完整的例子绘制功能.
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

	// 把实例顶点和纹理坐标数据存储在GPU buffer object中.
	// sprite uses the same original square shape. 这里使用了设计模式中的实例模式/享元模式. 
	float square[] = {            // 真正的实例就是这个左下角顶点在坐标原点的(0,0)单位正方形. 正方形四个顶点对应四个不同的纹理坐标.
		// pos      // tex coord
		0.0f, 1.0f, 0.0f, 1.0f,     // 之后要做的是通过model矩阵对基础正方形进行实例化, 转化成一个个未着色前的精灵图. 
		1.0f, 0.0f, 1.0f, 0.0f,     // 由于并未执行projection matrix, 所以坐标没有flip成窗口空间坐标, 仍以正方形左下角顶点为锚点锚在原点(0,0)位置.
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

// 典型的粒子发射器更新循环update loop可以划分为两个不同的阶段: 参数更新/模拟阶段 以及 渲染阶段. 每个循环执行每一帧动画.
// 粒子需要position信息, 而这position由传递进来的GameObject决定. 这个粒子系统是模拟彗星的拖尾或者是火球的拖尾. "彗星"就是传递进来的GameObject.
void ParticleEmitter::Update(float delta_time, const GameObject& object, glm::vec2 particle_pos_offset, unsigned int new_spawn_particle)
{
	for (unsigned int idx = 0; idx < new_spawn_particle; ++idx)
	{
		ReSpawnParticle(this->particles[FirstUnUsedParticle()], object, particle_pos_offset);
	}

	for (auto& particle : this->particles)
	{
		particle.life -= delta_time;		// 首先衰减生命时间.
		if (particle.life > 0.0f)			// 判断粒子是否还是active状态.
		{
			particle.position -= particle.velocity * delta_time;
			particle.color.a -= 2.5f * delta_time;      // alpha channel value decay.   核心代码语句, alpha是逐渐衰弱的. 如此传给shader的color必须是vec4.
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
	float random = ((rand() % 100) - 50) / 10.0f;       // 取[-5,+5)区间内的随机浮点数.
	float color = 0.5f + ((rand() % 50) / 100.0f);      // 取[0.5f, 1.0f]之间随机颜色.

	particle.position = object.Position() + offset + random;		// particle位置为object的位置 + 偏移值 + 随机值
	particle.velocity = object.Velocity() * 0.1f;					// 取object速度的fraction. 匀速.
	particle.size = object.Size() * 0.9f;							// 取object size的0.9倍.
	particle.color = glm::vec4(color, color, color, 1.0f);			// 生成的粒子的RGB颜色值随机的在[0.5f, 1.0f]之间.
	particle.life = 1.0f;											// 初始生命值为1.0, 均匀按照delta time递减.	
}

#endif // !PARTICLE_EMITTER_H

