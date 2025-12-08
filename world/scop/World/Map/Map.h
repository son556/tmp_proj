#pragma once

#include "MapUtils.h"
#include "TerrainSystem.h"
#include "LightSystem.h"
#include "RenderSystem.h"

class Chunk;
class Graphic;

class Map
{
public:
	Map(
		int size_w,
		int size_h,
		int fov_chunk,
		int thread_cnt,
		HWND hwnd,
		UINT window_w,
		UINT window_h
	);
	void userPositionCheck(float x, float z);
	void chunksSetVerticesAndIndices( // 그림자와 면 생성
		vector<Index2> const& v_idx,
		int st,
		int ed
	);
	void vertexAndIndexGenerator( // goe render 용
		Index2 const& c_idx,
		Index2 const& adj_idx,
		Index3 const& move,
		int dir,
		vector<VertexGeo>& vertices,
		vector<uint32>* indices = nullptr,
		uint32* index = nullptr
	);
	void vertexShadowGenerator( // shadow 용
		Index2 const& c_idx,
		Index2 const& adj_idx,
		Index3 const& move,
		int dir,
		vector<VertexShadow>& vertices,
		vector<uint32>* indices = nullptr,
		uint32* index = nullptr
	);
	void vertexAndIndexGeneratorTP(
		Index2 const& c_idx
	);
	void vertexAndIndexGeneratorWater(Index2 const& c_idx);
	void setSightChunk(int chunk_cnt);

private:
	Map() = delete;
	Map& operator=(Map const&) = delete;
	Map(Map const&) = delete;

private:
	void terrainSetVerticesAndIndices();
	void resetChunk(Index2 const& c_idx);
	int checkTerrainBoundary(float x, float z) const;
	void threadFunc(vector<Index2>& vec, int dir);

public:
	MapUtils m_info;
	LightSystem l_system;
	TerrainSystem t_system;
	RenderSystem r_system;

private:
	int c_fov;
	int thread_cnt;
};

