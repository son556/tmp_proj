#pragma once

#include "WorldUtils.h"
#include "PerlinNoise.h"

#include "SetWater.h"
#include "SetTree.h"
#include "SetCave.h"

// 청크에 블록들(땅, 나무, 물)을 채움
class MapUtils;
struct Index2;

class TerrainSystem
{
public:
	TerrainSystem(MapUtils* minfo);
	void fillChunk(Index2 const& c_idx, Index2 const& c_pos);
	void createHeightMap();
	void createTrees();
	void createTrees(vector<Index2>& chunks, int dir);
	void fillWithUserPlacedBlocks(Index2 const& c_idx);

private:
	MapUtils* m_info;
	PerlinNoise perlin_noise;
	SetWater s_water; // 물 만들기
	SetTree s_tree; // 나무 만들기
	SetCave s_cave; // 동굴 만들기
};

