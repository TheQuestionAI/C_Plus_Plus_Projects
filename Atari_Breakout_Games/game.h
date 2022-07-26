#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <irrklang/irrKlang.h>

#include <vector>
#include <memory>
#include <tuple>
#include <algorithm>

#include "resource_manager.h"
#include "game_level.h"
#include "sprite_renderer.h"
#include "particle_emitter.h"
#include "post_processor.h"
#include "ball.h"
#include "player.h"
#include "utility.h"
#include "power_up.h"

enum class GameState
{
	ACTIVE,
	MENU,
	WIN
};

enum class Side
{
	UP,
	RIGHT,
	DOWN,
	LEFT
};

/*
* Game:
* 我们定义一个Game类, Game类中包含所有相关的渲染和游戏代码. 定义一个Game类的意图是, 我们将用它完整的组织我们的游戏代码, 同时还将所有窗口代码与游戏代码分离开来.
* 如此, 我们就可以毫不费力地在完全不同的窗口库(例如 SDL 或 SFML)中使用相同的Game类.
*
* 游戏类必须包含游戏启动函数(初始化), 表示一个完整的game loop的相关函数(游戏输入处理函数, 游戏状态更新函数和游戏图像渲染函数).
*
* Game初始化函数, 载入Game所需的一切着色器、纹理对象以及关卡数据信息.
* Game loop member functions:
*	1. 游戏输入处理函数ProcessInput(deltaTime).
*	2. 游戏状态更新函数Update(deltaTime).
*	3. 游戏画面渲染函数Render().
*
*/
class Game
{
public:
	// 定义一个三元组, 告诉我们是否发生了碰撞、发生碰撞的AABB边、表示球进入AABB内部程度的差向量R.
	typedef std::tuple<bool, Side, glm::vec2> CollsionRecord;

	Game(unsigned int w, unsigned int h);
public:
	void Init();									// Game初始化函数, 载入Game所需的一切着色器、纹理对象以及关卡数据信息.
	// game loop functions
	void ProcessInput(float delta_time);
	void Update(float delta_time);
	void Render();
public:
	// auxiliary member function.
	void KeyPress(unsigned int key);
	void KeyRelease(unsigned int key);
private:
	unsigned int width, height;		// resolution of the game.
	bool keys[1024];				// store key press state.
	GameState state;

	std::vector<GameLevel> levels;
	unsigned int currentLevel;

	std::shared_ptr<SpriteRenderer> spriteRenderer;
	std::shared_ptr<ParticleEmitter> particleEmitter;
	std::shared_ptr<PostProcessor> postProcessor;
	std::shared_ptr<irrklang::ISoundEngine> soundEngine;

	std::shared_ptr<Ball> ball;
	std::shared_ptr<Player> player;
	std::shared_ptr<GameObject> background;

	float shakeDuration;
	float reverseDuration;
	float chaosDuration;

	std::vector<PowerUp> powerUps;

	void DoCollisions();												// 检查ball和game中出现的所有tile以及paddle的碰撞情况.
	CollsionRecord CheckCollision(Ball& ball, GameObject& tile);		// 检查单个tile是否和ball发生碰撞
	bool CheckCollision(GameObject& one, GameObject& two);				// GameObject的AABB检查是否发生碰撞.
	Side CheckCollisionSide(const glm::vec2& target);					// 检查ball和tile或者paddle的哪一条边发生碰撞.

	bool ShouldSpawn(unsigned int chance);								// chance取值[0, 1000]之间, 0表示no spawn, 1000表示spawn, chance值越大spawn概率越大. 如果spawn大于1000则等于1000.
	bool IsPowerUpActive(const std::string& type);
	void SpawnPowerUps(const Tile& tile);
	void ActivatePowerUp(PowerUp& power_up);
	void UpdatePowerUps(float delta_time);


	void ReSetGame();
};

Game::Game(unsigned int w, unsigned int h) : width{ w }, height{ h }, keys{ }, state{ GameState::ACTIVE }, shakeDuration{ 0.0f }, reverseDuration{ 0.0f }, chaosDuration{ 0.0f }, powerUps( )
{
	this->Init();
}

void Game::Init()
{
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Initialize all shaders and tetxures. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	ResourceManager::LoadShader("sprite", "sprite.vert", "sprite.frag");
	ResourceManager::LoadShader("particle", "particle.vert", "particle.frag");
	ResourceManager::LoadShader("post_processor", "post_processing.vert", "post_processing.frag");

	ResourceManager::LoadTexture("background", "background.jpg");
	ResourceManager::LoadTexture("block", "block.png");
	ResourceManager::LoadTexture("block_solid", "block_solid.png");
	ResourceManager::LoadTexture("paddle", "paddle.png");
	ResourceManager::LoadTexture("face", "awesomeface.png");
	ResourceManager::LoadTexture("particle", "particle.png");

	ResourceManager::LoadTexture("speed", "powerup_speed.png");
	ResourceManager::LoadTexture("sticky", "powerup_sticky.png");
	ResourceManager::LoadTexture("pass_through", "powerup_passthrough.png");
	ResourceManager::LoadTexture("pad_size_increase", "powerup_increase.png");
	ResourceManager::LoadTexture("confusion", "powerup_confuse.png");
	ResourceManager::LoadTexture("chaos", "powerup_chaos.png");
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Load all game level data. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
	this->levels.emplace_back("Level_1_Standard.txt", this->width, this->height / 2);
	this->currentLevel = 0;
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Initialize spriteRenderer、particleEmitter、 postprocessor. ~~~~~~~~~~~~~~~~~~~~~~ 
	this->spriteRenderer = std::make_shared<SpriteRenderer>(ResourceManager::GetShader("sprite"));
	this->particleEmitter = std::make_shared<ParticleEmitter>(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);
	this->postProcessor = std::make_shared<PostProcessor>(ResourceManager::GetShader("post_processor"), this->width, this->height);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 

	// ~~~~~~~~~~~~~~~~~~~~~~~~ Initialize soundEngine and play game background music. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	this->soundEngine = std::shared_ptr<irrklang::ISoundEngine>(irrklang::createIrrKlangDevice());
	this->soundEngine->play2D("Komiku_-_06_-_School.mp3", true);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Initialize paddle and ball. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	const glm::vec2 PADDLE_SIZE = glm::vec2(160.0f, 30.f);
	const glm::vec2 PADDLE_POSITION = glm::vec2(this->width / 2.0f - PADDLE_SIZE.x / 2.0f, this->height - PADDLE_SIZE.y);
	const float PADDLE_VELOCITY = 500.0f;
	this->player = std::make_shared<Player>(ResourceManager::GetTexture("paddle"), PADDLE_POSITION, PADDLE_SIZE, PADDLE_VELOCITY);

	const float BALL_RADIUS = 19.0f;
	const glm::vec2 BALL_POSITION = glm::vec2(this->width / 2.0f - BALL_RADIUS, PADDLE_POSITION.y - 2.0f * BALL_RADIUS);
	const glm::vec2 BALL_VELOCITY = glm::vec2(100.0f, -400.0f);
	this->ball = std::make_shared<Ball>(ResourceManager::GetTexture("face"), BALL_POSITION, BALL_RADIUS, BALL_VELOCITY);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Initialize background. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	const glm::vec2 BACKGROUND_POSITION = glm::vec2(0.0f);
	const glm::vec2 BACKGROUND_SIZE = glm::vec2(this->width, this->height);
	this->background = std::make_shared<GameObject>(ResourceManager::GetTexture("background"), BACKGROUND_POSITION, BACKGROUND_SIZE);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 

	// ~~~~~~~~~~~~~~~~~~~~~ Define 2D game projection matrix and set shader in use. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	// 和标准的窗口坐标空间保持一致, reverse y轴. +x轴向右, +y轴向下. 正交投影矩阵对于所有游戏中的object都一致, 所以单拎出来一次性set uniform variable. 同理对于background image.
	const glm::mat4 PROJECTION = glm::ortho(0.0f, static_cast<float>(this->width), static_cast<float>(this->height), 0.0f, -1.0f, 1.0f);     // z轴直接投影成NDC空间Z轴区间.
	ResourceManager::GetShader("sprite")->Use();
	ResourceManager::GetShader("sprite")->SetMat4x4("projection", PROJECTION);           // set orthogonal projection matrix to sprite shader.
	ResourceManager::GetShader("particle")->Use();
	ResourceManager::GetShader("particle")->SetMat4x4("projection", PROJECTION);         // set orthogonal projection matrix to particle shader.
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
}

// 2D break out game, 我们只能左右控制paddle的移动.
void Game::ProcessInput(float delta_time)
{
	if (this->state == GameState::ACTIVE)
	{
		float player_velocity = this->player->Velocity().x * delta_time;
		if (this->keys[GLFW_KEY_A] && this->player->Position().x > 0.0f)
		{
			this->player->Position().x -= player_velocity;
			if (this->ball->IsStuck())
			{
				this->ball->Position().x -= player_velocity;
			}
		}
		if (this->keys[GLFW_KEY_D] && this->player->Position().x + this->player->Size().x < this->width)
		{
			this->player->Position().x += player_velocity;
			if (this->ball->IsStuck())
			{
				this->ball->Position().x += player_velocity;
			}
		}
	}

	// press space button to release the ball from paddle.
	if (this->keys[GLFW_KEY_SPACE])
	{
		this->ball->ShouldStuck(false);
	}
}
void Game::Update(float delta_time)
{
	// Update函数更新游戏的状态. paddle是由processInput函数来控制, 那游戏状态的update会包含
	// 1. ball的movement and collision.
	// 2. tile state update.
	// 2. particleEmitter update.
	if (this->state == GameState::ACTIVE)
	{
		// update ball movement.
		this->ball->Move(delta_time, this->width, this->height);

		// check collision and update ball and tile state.
		this->DoCollisions();
		
		// update powerups.
		this->UpdatePowerUps(delta_time);

		// update particles.
		this->particleEmitter->Update(delta_time, *this->ball, glm::vec2(0.0f, this->ball->Radius()));

		// update shake duration time. confusion和chaos被算在powerup里面, 因此在UpdatePowerUps()函数里reduce time.
		if (this->shakeDuration > 0.0f)
		{
			this->shakeDuration -= delta_time;
			if (this->shakeDuration <= 0.0f)
			{
				this->postProcessor->Shake(false);
			}
		}
		
		// update lose condition: ball reach out the ball edge
		if (this->ball->Position().y > this->height)	// 这里表示ball全身完全消失在窗口之外时, game lose.
		{
			this->ReSetGame();
		}
	}
}
void Game::Render()
{
	// 渲染的顺序一定要正确.
	// ~~~~~~~~~~~~~~~ start post-processing game initial rendering ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	this->postProcessor->BeginSceneRender();
	{
		// First draw background.
		this->background->Draw(*this->spriteRenderer);

		// Second draw game level object.
		this->levels[currentLevel].Draw(*this->spriteRenderer);

		// Third draw powerups.
		for (PowerUp& power_up : this->powerUps)
		{
			if (!power_up.IsDestroyed())
			{
				power_up.Draw(*this->spriteRenderer);
			}
		}

		// Fourth draw particle emitter and ball.
		this->particleEmitter->Draw();
		this->ball->Draw(*this->spriteRenderer);

		// Fifth draw paddle.
		this->player->Draw(*this->spriteRenderer);
	}
	this->postProcessor->EndSceneRender();
	// ~~~~~~~~~~~~~~~ end post-processing game initial rendering ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// ~~~~~~~~~~~~~~~ render post-processing effect ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	this->postProcessor->RenderToWindow(Timer::Time());
}

void Game::KeyPress(unsigned int key)
{
	this->keys[key] = true;
}
void Game::KeyRelease(unsigned int key)
{
	this->keys[key] = false;
}

// 检查ball和tile、paddle是否发生碰撞; 检查powerup是否产生、是否和paddle发生碰撞.
void Game::DoCollisions()
{
	// 首先, 检查ball和game中所有存在的tile的碰撞情况.
	for (auto& tile : this->levels[this->currentLevel].Tiles())
	{
		if (!tile.IsDestroyed())
		{
			CollsionRecord collision_record = this->CheckCollision(*this->ball, tile);
			if (std::get<0>(collision_record))      // if collision is true
			{
				if (!tile.IsSolid()) // destroy block if not solid
				{
					tile.Destroy();
					this->soundEngine->play2D("bleep.mp3", false);
					// 依概率尝试生成能量道具.
					this->SpawnPowerUps(tile);
				}
				else
				{
					// if block is solid, enable shake effect
					this->shakeDuration = 0.2f;
					this->postProcessor->Shake(true);
					this->soundEngine->play2D("solid.wav", false);
				}

				// 对于撞到solid tile的情况, 则一定速度方向会改变; 而对于撞到非solid tile但是ball的passthrough没有激活则速度方向也改变.
				if (tile.IsSolid() || !this->ball->IsPassThrough())
				{
					// collision happens, reposition the ball and update the velocity direction.
					Side side = std::get<1>(collision_record);
					glm::vec2 diff = std::get<2>(collision_record);
					if (side == Side::LEFT || side == Side::RIGHT)
					{
						this->ball->Velocity().x = -this->ball->Velocity().x;
						// re-position ball.
						float offset = this->ball->Radius() - std::abs(diff.x);
						if (side == Side::LEFT)
						{
							this->ball->Position().x -= offset;
						}
						else
						{
							this->ball->Position().x += offset;
						}
					}
					else
					{
						this->ball->Velocity().y = -this->ball->Velocity().y;
						float offset = this->ball->Radius() - std::abs(diff.y);
						if (side == Side::UP)
						{
							this->ball->Position().y -= offset;
						}
						else
						{
							this->ball->Position().y += offset;
						}
					}
				}
			}
		}
	}
	
	// 然后, 检查powerup和paddle的碰撞情况, 如果碰撞发生则激活该powerup然后再将其destroy.
	for (PowerUp& power_up : this->powerUps)
	{
		if (!power_up.IsDestroyed())
		{
			if (CheckCollision(power_up, *this->player))			// AABB间碰撞检测.
			{	// 如果paddle吃到powerup, 则激活powerup并摧毁它.
				power_up.ShouldActivate(true);
				this->ActivatePowerUp(power_up);
				power_up.Destroy();
				this->soundEngine->play2D("powerup.wav", false);	// 音效启动.
			}
		}
	}
	
	// 然后, 检查ball和game中paddle的碰撞情况.
	CollsionRecord collision_record = CheckCollision(*this->ball, *this->player);
	if (!ball->IsStuck() && std::get<0>(collision_record))
	{
		// check where it hit the board, and change velocity
		float paddle_center_x = this->player->Position().x + this->player->Size().x / 2.0f;
		float ball_center_x = this->ball->Position().x + this->ball->Radius();

		// distance是有向距离:
		//      1. 如果distance和ball的水平速度同向(即同正或同负), 那么表示的是ball撞击的是paddle靠ball更远的那一半边, 如此撞击后ball的水平速度会方向不变.
		//      2. 如果distance和ball的水平速度反向, 那么表示的是ball撞击的是paddle靠ball更近的那一半边, 如此撞击后ball的水平速度会反向.
		// ball的垂直速度和paddle撞击后始终会反向.
		float distance = ball_center_x - paddle_center_x;           // directional distance.
		float percentage = distance / (this->player->Size().x / 2.0f);      // 求出一个ball撞击时偏离paddle center的程度, 使用百分比表示.

		// then move accordingly
		float strength = 2.0f;;
		glm::vec2 ball_velocity = glm::vec2(this->ball->InitVelocity().x * percentage * strength, ball->InitVelocity().y);
		this->ball->Velocity() = glm::normalize(ball_velocity) * glm::length(this->ball->Velocity());

		// solve sticky paddle issue.
		this->ball->Velocity() = glm::vec2(this->ball->Velocity().x, -1.0f * std::abs(this->ball->Velocity().y));

		// if Sticky powerup is activated, also stick ball to paddle once new velocity vectors were calculated
		ball->ShouldStuck(ball->IsSticky());

		this->soundEngine->play2D("bleep.wav", false);	// 音效启动.
	}
}
Game::CollsionRecord Game::CheckCollision(Ball& ball, GameObject& tile)
{
	/*
	* 我们在 AABB 上找到离圆最近的点P, 如果圆心到这个点P的距离小于它的半径，就会发生碰撞.
	*
	*   非常trick的collision detection的方式. 用于圆/球和AABB的碰撞判断:
	*       我们首先求得球心C 和AABB 中心B 之间的差向量D. 然后将向量D的x,y分量clampa到[-w/2, w/2]x[-h/2,h/2]区间内.
	*       精髓就在这里, 如此clamp操作后, 这将返回一个始终位于 AABB 边上某处的位置向(除非圆的中心在 AABB 内部).
	*       这个clamp向量P对AABB中心进行shift操作后P + B就表示的是从AABB到球的最近点!!
	*/
	glm::vec2 ball_center = ball.Position() + ball.Radius();              // get ball center.
	glm::vec2 aabb_center = tile.Position() + tile.Size() / 2.0f;       // get aabb center.

	glm::vec2 diff = ball_center - aabb_center;                                 // get difference vector between both centers 
	glm::vec2 offset = glm::clamp(diff, -tile.Size() / 2.0f, tile.Size() / 2.0f);
	glm::vec2 closest = aabb_center + offset;                                   // add clamped value to AABB_center and get the value closest to circle

	diff = ball_center - closest;                                               // update difference.
	float distance = glm::length(diff);                                         // distance from the closed point on AABB to ball center.
	if (distance < ball.Radius())
	{
		return { true, CheckCollisionSide(diff), diff };
	}

	return { false, Side::UP, glm::vec2(0.0f) };
}
Side Game::CheckCollisionSide(const glm::vec2& target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, -1.0f), // up
		glm::vec2(1.0f, 0.0f), // right
		glm::vec2(0.0f, 1.0f), // down
		glm::vec2(-1.0f, 0.0f) // left
	};

	float max_value = 0.0f;
	unsigned int best_match = -1;
	for (unsigned int idx = 0; idx < 4; ++idx)
	{
		float value = glm::dot(target, compass[idx]);
		if (value > max_value)
		{
			max_value = value;
			best_match = idx;
		}
	}

	return static_cast<Side>(best_match);
}
bool Game::CheckCollision(GameObject& one, GameObject& two)
{
	// collision x-axis?
	bool collisionX = (one.Position().x + one.Size().x >= two.Position().x) && (two.Position().x + two.Size().x >= one.Position().x);
	// collision y-axis?
	bool collisionY = (one.Position().y + one.Size().y >= two.Position().y) && (two.Position().y + two.Size().y >= one.Position().y);
	// collision only if on both axes
	return collisionX && collisionY;
}

bool Game::ShouldSpawn(unsigned int chance)
{
	// ShouldSpawn函数描述的是1000次碰撞中会产生powerup的次数.
	if (chance > 1000)
	{
		chance = 1000;
	}

	unsigned int random = rand() % 1001;		// random取值[0, 1000]间的正整数.

	return random < chance;
}
bool Game::IsPowerUpActive(const std::string& type)
{
	// 判断还有没有type这一类型的powerup是处于激活状态.
	for (const auto& power_up : this->powerUps)
	{
		if (power_up.IsActivated() && power_up.Type() == type)
		{
			return true;
		}
	}
	return false;
}
void Game::SpawnPowerUps(const Tile& tile)
{
	glm::vec2 POSITION = tile.Position();
	glm::vec2 SIZE = glm::vec2(tile.Size().x, tile.Size().y / 3.0f);
	glm::vec2 VELOCITY = glm::vec2(0.0f, 150.0f);
	if (ShouldSpawn(50))
	{
		auto TEXTURE = ResourceManager::GetTexture("speed");
		glm::vec3 COLOR = glm::vec3(0.5f, 0.5f, 1.0f);			// 不同的能量道具, 使用不同的颜色.
		float DURATION = 0.0f;
		this->powerUps.emplace_back("speed", TEXTURE, POSITION, SIZE, VELOCITY, COLOR, DURATION);
	}
	if (ShouldSpawn(50))
	{
		auto TEXTURE = ResourceManager::GetTexture("sticky");
		glm::vec3 COLOR = glm::vec3(1.0f, 0.5f, 1.0f);			// 不同的能量道具, 使用不同的颜色.
		float DURATION = 20.0f;
		this->powerUps.emplace_back("sticky", TEXTURE, POSITION, SIZE, VELOCITY, COLOR, DURATION);
	}
	if (ShouldSpawn(50))
	{
		auto TEXTURE = ResourceManager::GetTexture("pass_through");
		glm::vec3 COLOR = glm::vec3(0.5f, 1.0f, 0.5f);
		float DURATION = 10.0f;
		this->powerUps.emplace_back("pass_through", TEXTURE, POSITION, SIZE, VELOCITY, COLOR, DURATION);
	}
	if (ShouldSpawn(50))
	{
		auto TEXTURE = ResourceManager::GetTexture("pad_size_increase");
		glm::vec3 COLOR = glm::vec3(1.0f, 0.6f, 0.4f);
		float DURATION = 0.0f;
		this->powerUps.emplace_back("pad_size_increase", TEXTURE, POSITION, SIZE, VELOCITY, COLOR, DURATION);
	}
	if (ShouldSpawn(25))
	{
		auto TEXTURE = ResourceManager::GetTexture("confusion");
		glm::vec3 COLOR = glm::vec3(1.0f, 0.3f, 0.3f);
		float DURATION = 10.0f;
		this->powerUps.emplace_back("confusion", TEXTURE, POSITION, SIZE, VELOCITY, COLOR, DURATION);
	}
	if (ShouldSpawn(25))
	{
		auto TEXTURE = ResourceManager::GetTexture("chaos");
		glm::vec3 COLOR = glm::vec3(0.9f, 0.25f, 0.25f);
		float DURATION = 10.0f;
		this->powerUps.emplace_back("chaos", TEXTURE, POSITION, SIZE, VELOCITY, COLOR, DURATION);
	}
}
void Game::ActivatePowerUp(PowerUp& power_up)				// 能量道具在碰撞过程中是否出现的检查在DoCollision()函数中, 并且在该函数也会检查能量道具是否被玩家吃到(即碰撞到), 迟到的话则激活能量道具.
{
	if (power_up.Type() == "speed")
	{
		this->ball->Velocity() *= 1.2f;
	}
	if (power_up.Type() == "sticky")
	{
		// 对于sticky这一效果, 我们没法直接set isStuck变量为true, 这样的话ball会直接在空中停止运动. 
		// 我们需要再在ball内部维持一个变量来设为true, 并当ball和paddle撞击之时检查这一变量的state, 把这一变量的state的值赋值给stuck变量.
		this->ball->ShouldSticky(true);
		this->player->Color() = glm::vec3(1.0f, 0.5f, 1.0f);		// 配合效果一起, paddle颜色也变化.
	}
	if (power_up.Type() == "pass_through")
	{
		// 我们需要在ball内部维持一个passthrough变量, 当吃到穿透道具时, 则设置为true.
		this->ball->ShouldPassThrough(true);
		this->player->Color() = glm::vec3(1.0f, 0.5f, 0.5f);		// 配合效果一起, paddle颜色也变化.
	}
	if (power_up.Type() == "pad_size_increase")
	{
		this->player->Size().x += 50.0f;
	}
	if (power_up.Type() == "confusion")
	{
		if (!this->postProcessor->IsChaotic())
		{
			this->postProcessor->Reverse(true);
		}
	}
	if (power_up.Type() == "chaos")
	{
		if (!this->postProcessor->IsReversed())
		{
			this->postProcessor->Chaos(true);
		}
	}
}
void Game::UpdatePowerUps(float delta_time)
{
	for (PowerUp& power_up : this->powerUps)
	{
		// first check if powerup passed bottom edge, if so: keep as inactive and destroy
		if (power_up.Position().y >= this->height)
		{
			power_up.Destroy();
			power_up.ShouldActivate(false);
		}
		else
		{
			if (!power_up.IsDestroyed())
			{	// 只要powerup没有被destroy则必然垂直往下继续运动. 对于已经destroy的powerup则千万不能再下降计算position, 不然会触发第一条if condition语句自我摧毁.
				power_up.Position().y += power_up.Velocity().y * delta_time;		
			}
			
			if (power_up.IsActivated())					// 判断道具是否效果激活中.
			{
				// 这里注意一个隐含条件就是, powerup激活只有在powerup被paddle吃掉的时候, 因此一旦powerup激活必然powerup被destroy掉了.
				power_up.Duration() -= delta_time;		// 道具效果持续时间必须每帧减弱.
				if (power_up.Duration() < 0.0f)
				{
					power_up.ShouldActivate(false);
					// As the duration time goes to 0, we need to re-cover to the normal game state.
					if (power_up.Type() == "speed")
					{
						continue;		// 对ball的加速操作是永久的.
					}
					else if (power_up.Type() == "sticky")
					{
						if (!this->IsPowerUpActive("sticky"))		// 要判断一下是否还有其他同类型powerup是激活状态的, 是的话则不能re-cover.
						{
							this->ball->ShouldSticky(false);
							this->player->Color() = glm::vec3(1.0f);
						}
					}
					else if (power_up.Type() == "pass_through")
					{
						if (!this->IsPowerUpActive("pass_through"))		// 要判断一下是否还有其他同类型powerup是激活状态的, 是的话则不能re-cover.
						{
							this->ball->ShouldPassThrough(false);
							this->player->Color() = glm::vec3(1.0f);
						}
					}
					else if (power_up.Type() == "pad_size_increase")
					{
						continue;		// 对paddle的尺寸增大效果是永久的.
					}
					else if (power_up.Type() == "confusion")
					{
						if (!this->IsPowerUpActive("confusion"))		// 要判断一下是否还有其他同类型powerup是激活状态的, 是的话则不能re-cover.
						{
							this->postProcessor->Reverse(false);
						}
					}
					else if (power_up.Type() == "chaos")
					{
						if (!this->IsPowerUpActive("chaos"))		// 要判断一下是否还有其他同类型powerup是激活状态的, 是的话则不能re-cover.
						{
							this->postProcessor->Chaos(false);
						}
					}
				}
			}
		}	// 如果powerup没有被激活, 则do nothing, 此时powerup还在下落运动中没有被paddle吃到.
	}

	auto Predicate = [](const PowerUp& power_up) { return power_up.IsDestroyed() && !power_up.IsActivated(); };
	this->powerUps.erase(std::remove_if(this->powerUps.begin(), this->powerUps.end(), Predicate), this->powerUps.end());
}

void Game::ReSetGame()
{
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Re-Load all game level data. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	this->levels[currentLevel].Load("Level_1_Standard.txt", this->width, this->height / 2);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Re-Initialize paddle and ball. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	const glm::vec2 PADDLE_SIZE = glm::vec2(160.0f, 30.f);
	const glm::vec2 PADDLE_POSITION = glm::vec2(this->width / 2.0f - PADDLE_SIZE.x / 2.0f, this->height - PADDLE_SIZE.y);
	const float PADDLE_VELOCITY = 500.0f;
	this->player = std::make_shared<Player>(ResourceManager::GetTexture("paddle"), PADDLE_POSITION, PADDLE_SIZE, PADDLE_VELOCITY);

	const float BALL_RADIUS = 19.0f;
	const glm::vec2 BALL_POSITION = glm::vec2(this->width / 2.0f - BALL_RADIUS, PADDLE_POSITION.y - 2.0f * BALL_RADIUS);
	const glm::vec2 BALL_VELOCITY = glm::vec2(100.0f, -350.0f);
	this->ball = std::make_shared<Ball>(ResourceManager::GetTexture("face"), BALL_POSITION, BALL_RADIUS, BALL_VELOCITY);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Re-Initialize powerup. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	this->powerUps.clear();
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Re-Initialize effect state. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	this->ball->ShouldStuck(true);
	this->ball->ShouldSticky(false);
	this->ball->ShouldPassThrough(false);

	this->postProcessor->Shake(false);
	this->postProcessor->Reverse(false);
	this->postProcessor->Chaos(false);
}

#endif // !GAME_H
