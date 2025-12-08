#pragma once

#include "Map.h"


class Terrain
{
public:
	Terrain(int size_w, int size_h, 
		HWND hwnd, UINT width, 
		UINT height, int fov_chunk, 
		int thread_cnt
	);
	~Terrain();
	void setSightChunk(int cnt);
	void userPositionCheck(float x, float z);
	int16 getHeight(float x, float z) const;
	void putBlock(vec3 const& ray_pos, vec3 const& ray_dir, int type);
	void deleteBlock(vec3 const& ray_pos, vec3 const& ray_dir);
	void saveGame();

	/**
	*  @brief 포함되는 world 좌표를 받아 block의 인덱스를 반환합니다.
	*  @brief 가장 왼쪽 하단 꼭지점이 시작 위치입니다.
	*  @brief 포함되는 world 좌표는 x축과 평행한 모서리 기준으로 시작점(왼쪽)은
	*  @brief 끝점(오른쪽)은 포함되지 않습니다.
	*  @brief WorldIndex 구성요소
	*  @brief c_idx: chunk index, b_idx: block index
	*  @brief flag: 실제 맞았는지 유효 확인
	*  @brief pos : block의 world 좌표
	*  @brief dir: 맞은 평면 
	*  @brief 0. x축에 수직인 평면
	*  @brief 1. y축에 수직인 평면
	*  @brief 2. z축에 수직인 평면
	*  @brief block_type: 블록의 타입
	* \param x world x
	* \param y world y
	* \param z world z
	* \return block index
	*/
	WorldIndex getBlock(float x, float y, float z);
	WorldIndex getBlock(vec3 world_pos);


public: // test
	void testClickLightBlock(vec3 const& ray_pos, 
		vec3 const& ray_dir);

public:
	void Render();
	ComPtr<ID3D11ShaderResourceView>& getSRV();

private:
	shared_ptr<Map> m_manager;
	set<string> file_book;
	map<vec3, uint32> object_book;
	map<vec3, shared_ptr<Chunk>> tmp_storage;
};

