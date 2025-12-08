#pragma once

#include "WorldUtils.h"
#include "Chunk.h"

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

/**
 * s_pos - - - - - - - - - - - *
 * |						   |
 * |	sv_p - - - - - - *	   |
 * |	|				 |	   |
 * |	|				 |	   |
 * |	|				 |	   |
 * |	|				 |	   |
 * |	* - - - - - - ev_p	   |
 * |						   |
 * * - - - - - - - - - - - - - *
 */

class MapUtils
{
public:
	MapUtils(int size_w, int size_h, HWND hwnd, 
		UINT w_width, UINT w_heigh);
	~MapUtils();
public:
	inline vec3 intersectionRayAndPlane(
		vec3 const& r_pos,
		vec3 const& r_dir,
		vec3 const& p_pos,
		vec3 const& p_dir
	) {
		vec3 res;
		float t;
		t = (p_pos.Dot(p_dir) - r_pos.Dot(p_dir)) / p_dir.Dot(r_dir);
		res = r_pos + t * r_dir;
		return res;
	}

public:
	inline int findBlock(Index2 const& c_idx, int x, int y, int z) const {
		int idx = 16 * 16 * 256 * (c_idx.x + this->size_w * c_idx.y) +
			x + 16 * (z + 16 * y);
		if (x < 0 || x > 15 || z < 0 || z > 15 || y < 0 || y > 255)
			return this->findAdjBlock(c_idx, x, y, z);
		return this->blocks[idx];
	}

	inline int findBlock(Index2 const& c_idx, Index3 const& b_idx) const {
		int idx = 16 * 16 * 256 * (c_idx.x + this->size_w * c_idx.y) +
			b_idx.x + 16 * (b_idx.z + 16 * b_idx.y);
		if (b_idx.x < 0 || b_idx.x > 15 || b_idx.z < 0 || b_idx.z > 15
			|| b_idx.y < 0 || b_idx.y > 255)
			return this->findAdjBlock(c_idx, b_idx.x, b_idx.y, b_idx.z);
		return this->blocks[idx];
	}

	inline void addBlock(Index2 const& c_idx, int x, int y, int z, int type) {
		int idx = 16 * 16 * 256 * (c_idx.x + this->size_w * c_idx.y) + x +
			16 * (z + 16 * y);
		this->blocks[idx] = type;
	}

	inline void addBlock(Index2 const& c_idx, Index3 const& b_idx, int type) {
		int idx = 16 * 16 * 256 * (c_idx.x + this->size_w * c_idx.y) +
			b_idx.x + 16 * (b_idx.z + 16 * b_idx.y);
		this->blocks[idx] = type;
	}
	
	inline int findHeight(Index2 const& c_idx, Index2 const& h_idx) const {
		int idx = 16 * 16 * (c_idx.x + this->size_w * c_idx.y) +
			h_idx.x + 16 * h_idx.y;
		return this->h_map[idx];
	}
	/**
	 * 현위치의 최고 높이를 반환합니다.
	 * 
	 * \param c_idx 청크 인덱스
	 * \param x 블록 인덱스 x
	 * \param z 블록 인덱스 z
	 * \return 
	 */
	inline int findHeight(Index2 const& c_idx, int x, int z) const {
		int idx = 16 * 16 * (c_idx.x + this->size_w * c_idx.y) + x + 16 * z;
		return this->h_map[idx];
	}

	inline void setHeight(Index2 const& c_idx, Index2 const& b_idx, int h) {
		int idx = 16 * 16 * (c_idx.x + this->size_w * c_idx.y) +
			b_idx.x + 16 * b_idx.y;
		this->h_map[idx] = h;
	}

	inline void setHeight(Index2 const& c_idx, int x, int z, int h) {
		int idx = 16 * 16 * (c_idx.x + this->size_w * c_idx.y) + x + 16 * z;
		this->h_map[idx] = h;
	}
	
	// light
	inline uint8 findLight(Index2 const& c_idx, int x, int y, int z) const {
		int idx = 16 * 16 * 256 * (c_idx.x + this->size_w * c_idx.y) +
			x + 16 * (z + 16 * y);
		if (x < 0 || x > 15 || z < 0 || z > 15 || y < 0 || y > 255)
			return this->findAdjLightBlock(c_idx, x, y, z);
		return this->light_map[idx];
	}

	inline uint8 findLight(Index2 const& c_idx, Index3 const& b_idx) const {
		int idx = 16 * 16 * 256 * (c_idx.x + this->size_w * c_idx.y) +
			b_idx.x + 16 * (b_idx.z + 16 * b_idx.y);
		if (b_idx.x < 0 || b_idx.x > 15 || b_idx.z < 0 || b_idx.z > 15
			|| b_idx.y < 0 || b_idx.y > 255)
			return this->findAdjBlock(c_idx, b_idx.x, b_idx.y, b_idx.z);
		return this->light_map[idx];
	}

	inline void setLight(Index2 const& c_idx, int x, int y, int z, uint8 type) {
		int idx = 16 * 16 * 256 * (c_idx.x + this->size_w * c_idx.y) +
			x + 16 * (z + 16 * y);
		this->light_map[idx] = type;
	}
	
	inline void setLight(Index2 const& c_idx, Index3 const& b_idx, uint8 type) {
		int idx = 16 * 16 * 256 * (c_idx.x + this->size_w * c_idx.y) +
			b_idx.x + 16 * (b_idx.z + 16 * b_idx.y);
		this->light_map[idx] = type;
	}

	// book manage
	void writeBookAboutBlockStatus(Index2 const& chunk_pos,
		Index3 const& block_idx, BlockType original, BlockType new_block);
	/**
	 * 유저가 놓은 블록들의 정보를 반환합니다.
	 * 
	 * \param chunk_pos 청크 위치
	 * \return 유저가 놓은 블록들의 정보를 shared_ptr<const map<Index3, Index2>> 를
	 * 반환 합니다. 없으면 nullptr 반환
	 */
	map<Index3, Index2> const* const getUserPlacedBlocks(
		Index2 const& chunk_pos);

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
	inline WorldIndex getBlockIndex(float x, float y, float z) const {
		WorldIndex ans;
		int w_x = static_cast<int>(floor(x)) / 16;
		int w_z = static_cast<int>(floor(z)) / 16;

		if (w_x * 16 > x)
			w_x--;
		if (w_z * 16 < z)
			w_z++;
		Index2 c_idx = this->findChunkIndex(w_x * 16, w_z * 16);
		Index2 pos = this->chunks[c_idx.y][c_idx.x]->chunk_pos;
		int ex = pos.x + 16;
		int ez = pos.y - 16;
		ans.flag = false;
		if (x < pos.x || x > ex || z > pos.y || z < ez || y < 0 || y > 255)
			return ans;
		ans.flag = true;
		ans.c_idx = c_idx;
		int ix = static_cast<int>(floor(x) - pos.x);
		int iy = static_cast<int>(floor(y));
		int iz = floor(z);
		if (iz < z)
			iz += 1;
		iz = static_cast<int>(pos.y - iz);
		ans.b_idx = Index3(ix, iy, iz);
		ans.pos = vec3(pos.x, 0, pos.y) + vec3(ans.b_idx.x, ans.b_idx.y, -ans.b_idx.z);
		ans.block_type = this->findBlock(ans.c_idx, ans.b_idx);
		return ans;
	}


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


	inline Index2 findChunkIndex(int w_x, int w_z) const {
		int t = w_x / 16;
		int x = (t % this->size_w + this->size_w) % this->size_w;
		t = w_z / 16;
		int z = (t % this->size_h + this->size_h) % this->size_h;
		Index2 ans = Index2(x, z);
		if (this->chunks[ans.y][ans.x] == nullptr) {
			ans.flag = false;
			return ans;
		}
		Index2 pos = this->chunks[ans.y][ans.x]->chunk_pos;
		if (pos.x != w_x || pos.y != w_z)
			ans.flag = false;
		return ans;
	}


	inline Index2 getChunkIndex(int w_x, int w_z) const {
		int t = w_x / 16;
		int x = (t % this->size_w + this->size_w) % this->size_w;
		t = w_z / 16;
		int z = (t % this->size_h + this->size_h) % this->size_h;
		Index2 ans = Index2(x, z);
		return ans;
	}

	inline Index2 getAdjacentChunkIndex(Index2 const& c_idx, 
		Index3 const& next) {
		Index2 cidx;
		cidx.flag = true;
		Index2 const& cpos = this->chunks[c_idx.y][c_idx.x]->chunk_pos;
		if (next.x < 0)
			cidx = this->findChunkIndex(cpos.x - 16, cpos.y);
		else if (next.x > 15)
			cidx = this->findChunkIndex(cpos.x + 16, cpos.y);
		else if (next.z < 0)
			cidx = this->findChunkIndex(cpos.x, cpos.y + 16);
		else if (next.z > 15)
			cidx = this->findChunkIndex(cpos.x, cpos.y - 16);
		else if (next.y < 0)
			cidx = c_idx;
		else if (next.y > 255)
			cidx = c_idx;
		return cidx;
	}
private:
	inline int findAdjBlock(Index2 const& c_idx, int x, int y, int z) const {
		Index2 cpos = this->chunks[c_idx.y][c_idx.x]->chunk_pos;
		int idx = 0;
		if (x < 0) {
			cpos.x -= 16;
			Index2 adj_idx = this->findChunkIndex(cpos.x, cpos.y);
			if (adj_idx.flag == false)
				return 0;
			idx = 16 * 16 * 256 * (adj_idx.x + this->size_w * adj_idx.y) +
				15 + 16 * (z + 16 * y);
		}
		else if (x > 15) {
			cpos.x += 16;
			Index2 adj_idx = this->findChunkIndex(cpos.x, cpos.y);
			if (adj_idx.flag == false)
				return 0;
			idx = 16 * 16 * 256 * (adj_idx.x + this->size_w * adj_idx.y)
				+ 16 * (z + 16 * y);
		}
		else if (z < 0) {
			cpos.y += 16;
			Index2 adj_idx = this->findChunkIndex(cpos.x, cpos.y);
			if (adj_idx.flag == false)
				return 0;
			idx = 16 * 16 * 256 * (adj_idx.x + this->size_w * adj_idx.y)
				+ x + 16 * (15 + 16 * y);
		}
		else if (z > 15) {
			cpos.y -= 16;
			Index2 adj_idx = this->findChunkIndex(cpos.x, cpos.y);
			if (adj_idx.flag == false)
				return 0;
			idx = 16 * 16 * 256 * (adj_idx.x + this->size_w * adj_idx.y)
				+ x + 16 * (0 + 16 * y);
		}
		else if (y < 0)
			return 0;
		else if (y > 255)
			return 0;
		return this->blocks[idx];
	}

	inline uint8 findAdjLightBlock(Index2 const& c_idx, int x, int y, int z) const {
		Index2 cpos = this->chunks[c_idx.y][c_idx.x]->chunk_pos;
		int idx;
		if (x < 0) {
			cpos.x -= 16;
			Index2 adj_idx = this->findChunkIndex(cpos.x, cpos.y);
			if (adj_idx.flag == false)
				return 0;
			idx = 16 * 16 * 256 * (adj_idx.x + this->size_w * adj_idx.y) +
				15 + 16 * (z + 16 * y);
		}
		else if (x > 15) {
			cpos.x += 16;
			Index2 adj_idx = this->findChunkIndex(cpos.x, cpos.y);
			if (adj_idx.flag == false)
				return 0;
			idx = 16 * 16 * 256 * (adj_idx.x + this->size_w * adj_idx.y)
				+ 16 * (z + 16 * y);
		}
		else if (z < 0) {
			cpos.y += 16;
			Index2 adj_idx = this->findChunkIndex(cpos.x, cpos.y);
			if (adj_idx.flag == false)
				return 0;
			idx = 16 * 16 * 256 * (adj_idx.x + this->size_w * adj_idx.y)
				+ x + 16 * (15 + 16 * y);
		}
		else if (z > 15) {
			cpos.y -= 16;
			Index2 adj_idx = this->findChunkIndex(cpos.x, cpos.y);
			if (adj_idx.flag == false)
				return 0;
			idx = 16 * 16 * 256 * (adj_idx.x + this->size_w * adj_idx.y)
				+ x + 16 * (0 + 16 * y);
		}
		else if (y < 0)
			return 0;
		else if (y > 255)
			return 15;
		return this->light_map[idx];
	}

public:
	void saveGame();
	void loadGame();

private:
	MapUtils(MapUtils const&) = delete;
	MapUtils& operator=(MapUtils const&) = delete;

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
	vec3 directional_light_pos; // 태양 위치
	vec3 light_dir;

	/*
	Index2 -> chunk pos
	Index3 -> block idx
	Index2.x -> 지금 추가하려는 블록 상태
	Index2.y -> 현 블록위치의 최초 블록 상태
	*/
	map<Index2, map<Index3, Index2>> book; // 유저의 행동 결과를 저장

private:
	int* blocks;
	int* h_map; // x z 위치에서 블록의 최고 높이(블록의 윗면 높이를 기준으로 함)
	uint8* light_map;
};

