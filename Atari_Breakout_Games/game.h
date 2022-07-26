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
* ���Ƕ���һ��Game��, Game���а���������ص���Ⱦ����Ϸ����. ����һ��Game�����ͼ��, ���ǽ�������������֯���ǵ���Ϸ����, ͬʱ�������д��ڴ�������Ϸ������뿪��.
* ���, ���ǾͿ��Ժ�������������ȫ��ͬ�Ĵ��ڿ�(���� SDL �� SFML)��ʹ����ͬ��Game��.
*
* ��Ϸ����������Ϸ��������(��ʼ��), ��ʾһ��������game loop����غ���(��Ϸ���봦����, ��Ϸ״̬���º�������Ϸͼ����Ⱦ����).
*
* Game��ʼ������, ����Game�����һ����ɫ������������Լ��ؿ�������Ϣ.
* Game loop member functions:
*	1. ��Ϸ���봦����ProcessInput(deltaTime).
*	2. ��Ϸ״̬���º���Update(deltaTime).
*	3. ��Ϸ������Ⱦ����Render().
*
*/
class Game
{
public:
	// ����һ����Ԫ��, ���������Ƿ�������ײ��������ײ��AABB�ߡ���ʾ�����AABB�ڲ��̶ȵĲ�����R.
	typedef std::tuple<bool, Side, glm::vec2> CollsionRecord;

	Game(unsigned int w, unsigned int h);
public:
	void Init();									// Game��ʼ������, ����Game�����һ����ɫ������������Լ��ؿ�������Ϣ.
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

	void DoCollisions();												// ���ball��game�г��ֵ�����tile�Լ�paddle����ײ���.
	CollsionRecord CheckCollision(Ball& ball, GameObject& tile);		// ��鵥��tile�Ƿ��ball������ײ
	bool CheckCollision(GameObject& one, GameObject& two);				// GameObject��AABB����Ƿ�����ײ.
	Side CheckCollisionSide(const glm::vec2& target);					// ���ball��tile����paddle����һ���߷�����ײ.

	bool ShouldSpawn(unsigned int chance);								// chanceȡֵ[0, 1000]֮��, 0��ʾno spawn, 1000��ʾspawn, chanceֵԽ��spawn����Խ��. ���spawn����1000�����1000.
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

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Initialize spriteRenderer��particleEmitter�� postprocessor. ~~~~~~~~~~~~~~~~~~~~~~ 
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
	// �ͱ�׼�Ĵ�������ռ䱣��һ��, reverse y��. +x������, +y������. ����ͶӰ�������������Ϸ�е�object��һ��, ���Ե������һ����set uniform variable. ͬ�����background image.
	const glm::mat4 PROJECTION = glm::ortho(0.0f, static_cast<float>(this->width), static_cast<float>(this->height), 0.0f, -1.0f, 1.0f);     // z��ֱ��ͶӰ��NDC�ռ�Z������.
	ResourceManager::GetShader("sprite")->Use();
	ResourceManager::GetShader("sprite")->SetMat4x4("projection", PROJECTION);           // set orthogonal projection matrix to sprite shader.
	ResourceManager::GetShader("particle")->Use();
	ResourceManager::GetShader("particle")->SetMat4x4("projection", PROJECTION);         // set orthogonal projection matrix to particle shader.
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
}

// 2D break out game, ����ֻ�����ҿ���paddle���ƶ�.
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
	// Update����������Ϸ��״̬. paddle����processInput����������, ����Ϸ״̬��update�����
	// 1. ball��movement and collision.
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

		// update shake duration time. confusion��chaos������powerup����, �����UpdatePowerUps()������reduce time.
		if (this->shakeDuration > 0.0f)
		{
			this->shakeDuration -= delta_time;
			if (this->shakeDuration <= 0.0f)
			{
				this->postProcessor->Shake(false);
			}
		}
		
		// update lose condition: ball reach out the ball edge
		if (this->ball->Position().y > this->height)	// �����ʾballȫ����ȫ��ʧ�ڴ���֮��ʱ, game lose.
		{
			this->ReSetGame();
		}
	}
}
void Game::Render()
{
	// ��Ⱦ��˳��һ��Ҫ��ȷ.
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

// ���ball��tile��paddle�Ƿ�����ײ; ���powerup�Ƿ�������Ƿ��paddle������ײ.
void Game::DoCollisions()
{
	// ����, ���ball��game�����д��ڵ�tile����ײ���.
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
					// �����ʳ���������������.
					this->SpawnPowerUps(tile);
				}
				else
				{
					// if block is solid, enable shake effect
					this->shakeDuration = 0.2f;
					this->postProcessor->Shake(true);
					this->soundEngine->play2D("solid.wav", false);
				}

				// ����ײ��solid tile�����, ��һ���ٶȷ����ı�; ������ײ����solid tile����ball��passthroughû�м������ٶȷ���Ҳ�ı�.
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
	
	// Ȼ��, ���powerup��paddle����ײ���, �����ײ�����򼤻��powerupȻ���ٽ���destroy.
	for (PowerUp& power_up : this->powerUps)
	{
		if (!power_up.IsDestroyed())
		{
			if (CheckCollision(power_up, *this->player))			// AABB����ײ���.
			{	// ���paddle�Ե�powerup, �򼤻�powerup���ݻ���.
				power_up.ShouldActivate(true);
				this->ActivatePowerUp(power_up);
				power_up.Destroy();
				this->soundEngine->play2D("powerup.wav", false);	// ��Ч����.
			}
		}
	}
	
	// Ȼ��, ���ball��game��paddle����ײ���.
	CollsionRecord collision_record = CheckCollision(*this->ball, *this->player);
	if (!ball->IsStuck() && std::get<0>(collision_record))
	{
		// check where it hit the board, and change velocity
		float paddle_center_x = this->player->Position().x + this->player->Size().x / 2.0f;
		float ball_center_x = this->ball->Position().x + this->ball->Radius();

		// distance���������:
		//      1. ���distance��ball��ˮƽ�ٶ�ͬ��(��ͬ����ͬ��), ��ô��ʾ����ballײ������paddle��ball��Զ����һ���, ���ײ����ball��ˮƽ�ٶȻ᷽�򲻱�.
		//      2. ���distance��ball��ˮƽ�ٶȷ���, ��ô��ʾ����ballײ������paddle��ball��������һ���, ���ײ����ball��ˮƽ�ٶȻᷴ��.
		// ball�Ĵ�ֱ�ٶȺ�paddleײ����ʼ�ջᷴ��.
		float distance = ball_center_x - paddle_center_x;           // directional distance.
		float percentage = distance / (this->player->Size().x / 2.0f);      // ���һ��ballײ��ʱƫ��paddle center�ĳ̶�, ʹ�ðٷֱȱ�ʾ.

		// then move accordingly
		float strength = 2.0f;;
		glm::vec2 ball_velocity = glm::vec2(this->ball->InitVelocity().x * percentage * strength, ball->InitVelocity().y);
		this->ball->Velocity() = glm::normalize(ball_velocity) * glm::length(this->ball->Velocity());

		// solve sticky paddle issue.
		this->ball->Velocity() = glm::vec2(this->ball->Velocity().x, -1.0f * std::abs(this->ball->Velocity().y));

		// if Sticky powerup is activated, also stick ball to paddle once new velocity vectors were calculated
		ball->ShouldStuck(ball->IsSticky());

		this->soundEngine->play2D("bleep.wav", false);	// ��Ч����.
	}
}
Game::CollsionRecord Game::CheckCollision(Ball& ball, GameObject& tile)
{
	/*
	* ������ AABB ���ҵ���Բ����ĵ�P, ���Բ�ĵ������P�ľ���С�����İ뾶���ͻᷢ����ײ.
	*
	*   �ǳ�trick��collision detection�ķ�ʽ. ����Բ/���AABB����ײ�ж�:
	*       ���������������C ��AABB ����B ֮��Ĳ�����D. Ȼ������D��x,y����clampa��[-w/2, w/2]x[-h/2,h/2]������.
	*       �����������, ���clamp������, �⽫����һ��ʼ��λ�� AABB ����ĳ����λ����(����Բ�������� AABB �ڲ�).
	*       ���clamp����P��AABB���Ľ���shift������P + B�ͱ�ʾ���Ǵ�AABB����������!!
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
	// ShouldSpawn������������1000����ײ�л����powerup�Ĵ���.
	if (chance > 1000)
	{
		chance = 1000;
	}

	unsigned int random = rand() % 1001;		// randomȡֵ[0, 1000]���������.

	return random < chance;
}
bool Game::IsPowerUpActive(const std::string& type)
{
	// �жϻ���û��type��һ���͵�powerup�Ǵ��ڼ���״̬.
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
		glm::vec3 COLOR = glm::vec3(0.5f, 0.5f, 1.0f);			// ��ͬ����������, ʹ�ò�ͬ����ɫ.
		float DURATION = 0.0f;
		this->powerUps.emplace_back("speed", TEXTURE, POSITION, SIZE, VELOCITY, COLOR, DURATION);
	}
	if (ShouldSpawn(50))
	{
		auto TEXTURE = ResourceManager::GetTexture("sticky");
		glm::vec3 COLOR = glm::vec3(1.0f, 0.5f, 1.0f);			// ��ͬ����������, ʹ�ò�ͬ����ɫ.
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
void Game::ActivatePowerUp(PowerUp& power_up)				// ������������ײ�������Ƿ���ֵļ����DoCollision()������, �����ڸú���Ҳ�������������Ƿ���ҳԵ�(����ײ��), �ٵ��Ļ��򼤻���������.
{
	if (power_up.Type() == "speed")
	{
		this->ball->Velocity() *= 1.2f;
	}
	if (power_up.Type() == "sticky")
	{
		// ����sticky��һЧ��, ����û��ֱ��set isStuck����Ϊtrue, �����Ļ�ball��ֱ���ڿ���ֹͣ�˶�. 
		// ������Ҫ����ball�ڲ�ά��һ����������Ϊtrue, ����ball��paddleײ��֮ʱ�����һ������state, ����һ������state��ֵ��ֵ��stuck����.
		this->ball->ShouldSticky(true);
		this->player->Color() = glm::vec3(1.0f, 0.5f, 1.0f);		// ���Ч��һ��, paddle��ɫҲ�仯.
	}
	if (power_up.Type() == "pass_through")
	{
		// ������Ҫ��ball�ڲ�ά��һ��passthrough����, ���Ե���͸����ʱ, ������Ϊtrue.
		this->ball->ShouldPassThrough(true);
		this->player->Color() = glm::vec3(1.0f, 0.5f, 0.5f);		// ���Ч��һ��, paddle��ɫҲ�仯.
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
			{	// ֻҪpowerupû�б�destroy���Ȼ��ֱ���¼����˶�. �����Ѿ�destroy��powerup��ǧ�������½�����position, ��Ȼ�ᴥ����һ��if condition������Ҵݻ�.
				power_up.Position().y += power_up.Velocity().y * delta_time;		
			}
			
			if (power_up.IsActivated())					// �жϵ����Ƿ�Ч��������.
			{
				// ����ע��һ��������������, powerup����ֻ����powerup��paddle�Ե���ʱ��, ���һ��powerup�����Ȼpowerup��destroy����.
				power_up.Duration() -= delta_time;		// ����Ч������ʱ�����ÿ֡����.
				if (power_up.Duration() < 0.0f)
				{
					power_up.ShouldActivate(false);
					// As the duration time goes to 0, we need to re-cover to the normal game state.
					if (power_up.Type() == "speed")
					{
						continue;		// ��ball�ļ��ٲ��������õ�.
					}
					else if (power_up.Type() == "sticky")
					{
						if (!this->IsPowerUpActive("sticky"))		// Ҫ�ж�һ���Ƿ�������ͬ����powerup�Ǽ���״̬��, �ǵĻ�����re-cover.
						{
							this->ball->ShouldSticky(false);
							this->player->Color() = glm::vec3(1.0f);
						}
					}
					else if (power_up.Type() == "pass_through")
					{
						if (!this->IsPowerUpActive("pass_through"))		// Ҫ�ж�һ���Ƿ�������ͬ����powerup�Ǽ���״̬��, �ǵĻ�����re-cover.
						{
							this->ball->ShouldPassThrough(false);
							this->player->Color() = glm::vec3(1.0f);
						}
					}
					else if (power_up.Type() == "pad_size_increase")
					{
						continue;		// ��paddle�ĳߴ�����Ч�������õ�.
					}
					else if (power_up.Type() == "confusion")
					{
						if (!this->IsPowerUpActive("confusion"))		// Ҫ�ж�һ���Ƿ�������ͬ����powerup�Ǽ���״̬��, �ǵĻ�����re-cover.
						{
							this->postProcessor->Reverse(false);
						}
					}
					else if (power_up.Type() == "chaos")
					{
						if (!this->IsPowerUpActive("chaos"))		// Ҫ�ж�һ���Ƿ�������ͬ����powerup�Ǽ���״̬��, �ǵĻ�����re-cover.
						{
							this->postProcessor->Chaos(false);
						}
					}
				}
			}
		}	// ���powerupû�б�����, ��do nothing, ��ʱpowerup���������˶���û�б�paddle�Ե�.
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
