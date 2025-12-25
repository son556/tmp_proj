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

	// spiral 방식으로 변경하기 위한 테스트 함수
	void TestUserPositionCheck(float x, float z);
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
		//Index2 const& adj_idx,
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
	bool IsChunkInViewDistance(float userPosX, float userPosZ, float chunkPosX, float chunkPosZ);

	/**
	 * @brief 버텍스 버퍼와 라이트 맵에서 수정이 일어나야 하는 청크의 인덱스를 모아줍니다.
	 * @param chunkIndex 새로 만드는 청크의 인덱스
	 * @param userPos 유저 위치(world)
	 */
	void CheckChunkVertices(const Index2& chunkIndex, vec2 userPos);
	
	/**
	 * @brief 새로 만들어야(light, vertex, index...) 하는 청크들을 업데이트 하는 함수입니다.
	 */
	void UpdateChunks();

public:
	MapUtils _mapInfo;
	LightSystem l_system;
	TerrainSystem t_system;
	RenderSystem r_system;

private:
	static vector<vec2> direction;

	int _chunkFOV; // chunk 시야 범위
	int thread_cnt;
	float _userSightRadius;
	set<Index2> _createNewBufferChunkIndices;
	set<Index2> _createNewChunkIndices;
};

