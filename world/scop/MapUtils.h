#pragma once

#include "WorldUtils.h"

class Chunk;

/*
* * -- *
* |    | // 윗면
* * -- *
* 
* 0 -- *
* |    | // 아랫 면
* * -- *
* 
* block의 시작위치 -> 위 그림을 block이라 할 때 0 번
* 
* 청크의 시작 위치 -> 위 그림을 chunk라 할 때 0 번
*/

class MapUtils
{
public:
	MapUtils(int size_w, int size_h, HWND hwnd, 
		UINT w_width, UINT w_heigh);
	~MapUtils();
public:
	vec3 intersectionRayAndPlane(
		vec3 const& r_pos,
		vec3 const& r_dir,
		vec3 const& p_pos,
		vec3 const& p_dir
	);
	bool inChunkBoundary(int x, int y, int z);
	bool inChunkBoundary(Index3 const& bidx);

public:
	int findBlock(Index2 const& c_idx, int x, int y, int z) const;
	int findBlock(Index2 const& c_idx, Index3 const& b_idx) const;
	void addBlock(Index2 const& c_idx, int x, int y, int z, int type);
	void addBlock(Index2 const& c_idx, Index3 const& b_idx, int type);
	int findHeight(Index2 const& c_idx, Index2 const& h_idx) const;
	int findHeight(Index2 const& c_idx, Index3 const& b_idx) const;
	int findHeight(Index2 const& c_idx, int x, int z) const;
	void setHeight(Index2 const& c_idx, Index2 const& b_idx, int h);
	void setHeight(Index2 const& c_idx, int x, int z, int h);
	uint8 findLight(Index2 const& c_idx, int x, int y, int z) const;
	uint8 findLight(Index2 const& c_idx, Index3 const& b_idx) const;
	void setLight(Index2 const& c_idx, int x, int y, int z, uint8 type);
	void setLight(Index2 const& c_idx, Index3 const& b_idx, uint8 type);

public:
	/**
	*  @brief 포함되는 world 좌표를 받아 block의 인덱스를 반환합니다.
	*  @brief 가장 왼쪽 하단 꼭지점이 시작 위치입니다.
	*  @brief 포함되는 world 좌표는 x축과 평행한 모서리 기준으로 시작점(왼쪽)은
	*  @brief 끝점(오른쪽)은 포함되지 않습니다.
	*
	* \param x world x
	* \param y world y
	* \param z world z
	* \return block index
	*/
	WorldIndex getBlockIndex(float x, float y, float z) const;


	/**
	 * @brief 빛을 쏘아서 처음 맞은 블럭의 WorldIndex 반환합니다.
	 * @brief WorldIndex 구성요소
	 * @brief c_idx: chunk index, b_idx: block index
	 * @brief flag: 실제 맞았는지 유효 확인
	 * @brief pos : block의 world 좌표
	 * @brief dir: 맞은 평면 
	 * @brief 0. x축에 수직인 평면
	 * @brief 1. y축에 수직인 평면
	 * @brief 2. z축에 수직인 평면
	 * 
	 * \param r_pos
	 * \param r_dir
	 * \return 
	 */
	WorldIndex pickBlock(vec3 r_pos, vec3 r_dir);


	Index2 findChunkIndex(int w_x, int w_z) const;


	Index2 getChunkIndex(int w_x, int w_z) const;

private:
	int findAdjBlock(Index2 const& c_idx, int x, int y, int z) const;
	uint8 findAdjLightBlock(Index2 const& c_idx, int x, int y, int z) const;

public:
	int size_w; // 맵 세로 크기
	int size_h; // 맵 가로 크기
	Index2 s_pos; // 맵의 시작 위치
	Index2 sv_pos; // 실제로 보이는 부분의 시작위치
	Index2 ev_pos; // 실제로 보이는 부분의 끝 위치
	shared_ptr<Chunk> chunks[30][30];
	HWND hWnd;
	UINT width; // 창 가로 크기
	UINT height; // 창 세로 크기
	vec3 directional_light_pos;
	vec3 light_dir;

private:
	int* blocks;
	int* h_map;
	uint8* light_map;
};

