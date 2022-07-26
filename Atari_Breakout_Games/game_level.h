#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include "tile.h"
#include "sprite_renderer.h"
#include "resource_manager.h"

/*
* 我们定义关卡数据存储于类似矩阵的结构中, 其中每个数字代表一种砖块, 每个砖块之间用空格分隔. 在关卡代码中, 我们可以分配
* 每个数字代表的内容. 我们选择了以下表示:
*	A. 数字0: 无砖，代表关卡中的一个空位.
*	B. 数量1: 实心砖，不能被破坏的砖.
*	C. 大于1的数字: 可破坏的砖块, 每个后续数字仅颜色不同.
* 
*   file:						// 这里有个有趣的地方, 当我们使用vector<vector<>>来读取file时, 行的顺序恰好被flip了, 但被正交投影矩阵作用后, 又flip回来了, 
*			1 1 1 1 1 1			// 所以file所呈现的tile的出现顺序, 就刚好和我们在窗口空间(窗口左上角为坐标原点)中看到的从上到下的出现顺序完全一样
*			2 2 0 0 2 2
*			3 3 4 4 3 3
* 每一个GameLevel包含一个关卡的所有完整数据, 并提供Draw API可以绘制整个关卡.
*/
class GameLevel					// checked, no need to check again!
{
public:
	GameLevel() = default;
	// Here we assume each title has the same length and width.
	GameLevel(const char* game_level_file, unsigned int level_width, unsigned int level_height);				
public:
	void Load(const char* game_level_file, unsigned int level_width, unsigned int level_height);
	void Draw(SpriteRenderer& sprite_renderer);

	bool IsCompleted() const;

	std::vector<Tile>& Tiles();
private:
	std::vector<Tile> tiles;

	void LoadCodeToTile(std::vector<std::vector<unsigned int>> tile_data, unsigned int level_width, unsigned int level_height);
};

GameLevel::GameLevel(const char* game_level_file, unsigned int level_width, unsigned int level_height)
{
	Load(game_level_file, level_width, level_height);
}
void GameLevel::Load(const char* game_level_file, unsigned int level_width, unsigned int level_height)
{
	std::ifstream file_stream(game_level_file);
	if (file_stream.is_open())
	{
		unsigned int tile_code;
		std::vector<std::vector<unsigned int>> tile_data;
		std::string line;
		while (std::getline(file_stream, line))
		{
			std::istringstream str_stream(line);
			std::vector<unsigned int> tile_row_data;
			while (str_stream >> tile_code)
			{
				tile_row_data.push_back(tile_code);
			}
			tile_data.push_back(tile_row_data);
		}

		if (tile_data.size() > 0)
		{
			LoadCodeToTile(tile_data, level_width, level_height);
		}
	}
	else
	{
		fprintf(stderr, "%s @ %d: fail to load the game_level_file.\n", __FILE__, __LINE__);
	}
}
void GameLevel::LoadCodeToTile(std::vector<std::vector<unsigned int>> tile_data, unsigned int level_width, unsigned int level_height)
{
	this->tiles.clear();

	unsigned int nrows = tile_data.size();
	unsigned int ncols = tile_data[0].size();
	// Here we assume each title has the same length and width.
	float tile_width = static_cast<float>(level_width) / static_cast<float>(ncols);
	float tile_height = static_cast<float>(level_height) / static_cast<float>(nrows);
	// 以游戏平面[0, Width] x [0, Height] x 0 左下角为锚点/原点位置(0,0)进行tile GameObject的构造.
	for (unsigned int y = 0; y < nrows; ++y)		// y代表行, x代表列. tile创建顺序为每行每行的创建, 创建完一行的tile后再进入下一行.
	{
		for (unsigned int x = 0; x < ncols; ++x)	// 当我们使用vector<vector<>>来读取file时, 行的顺序恰好被flip了, 但被正交投影矩阵作用后, 又flip回来了
		{
			if (tile_data[y][x] == 0)
			{
				continue;
			}
			else
			{
				glm::vec2 position(tile_width * x, tile_height * y);
				glm::vec2 size(tile_width, tile_height);
				if (tile_data[y][x] == 1)		// solid. 数量1: 实心砖，不能被破坏的砖.
				{
					this->tiles.emplace_back(ResourceManager::GetTexture("block_solid"), position, size, 0.0f, glm::vec2(0.0f), glm::vec3(0.8f, 0.8f, 0.7f));
					this->tiles.back().Solid();		// make solid.
				}
				else // tileData[y][x] > 1	=> non-solid; now determine its color based on level data. 大于1的数字: 可破坏的砖块, 每个后续数字仅颜色不同.
				{
					glm::vec3 color = glm::vec3(1.0f);
					if (tile_data[y][x] == 2)
						color = glm::vec3(0.2f, 0.6f, 1.0f);
					else if (tile_data[y][x] == 3)
						color = glm::vec3(0.0f, 0.7f, 0.0f);
					else if (tile_data[y][x] == 4)
						color = glm::vec3(0.8f, 0.8f, 0.4f);
					else if (tile_data[y][x] == 5)
						color = glm::vec3(1.0f, 0.5f, 0.0f);

					this->tiles.emplace_back(ResourceManager::GetTexture("block"), position, size, 0.0f, glm::vec2(0.0f), color);
				}
			}
		}
	}
}

void GameLevel::Draw(SpriteRenderer& sprite_renderer)
{
	for (auto& tile : this->tiles)
	{
		if (!tile.IsDestroyed())
		{
			tile.Draw(sprite_renderer);
		}
	}
}

bool GameLevel::IsCompleted() const
{
	for (auto& tile : this->tiles)
	{
		if (!tile.IsSolid() && !tile.IsDestroyed())
		{
			return false;
		}
	}

	return true;
}

std::vector<Tile>& GameLevel::Tiles()
{
	return this->tiles;
}

#endif // !GAME_LEVEL_H

