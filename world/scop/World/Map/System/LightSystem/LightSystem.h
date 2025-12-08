#pragma once

#include "MapUtils.h"

class LightSystem
{
public:
	LightSystem(MapUtils* minfo);
	~LightSystem() = default;
	
public:
	void createLightMap(); // 빛을 모든 chunk에 대해 채움
	void createLightMap( // 빛을 특정청크에 대해 채움
		vector<Index2> const& new_cidxs,
		vector<Index2>& renew_cidxs
	);

	/**
	 * 없어진 블록에 대한 light 계산.
	 * 
	 * \param c_idx 현 청크 인덱스
	 * \param b_idx 블록위치 인덱스
	 * \param cidx_vec 업데이트 된 청크들의 인덱스가 담길 vector
	 */
	void BFSLightBlockDelete(
		Index2 const& c_idx, 
		Index3 const& b_idx, 
		vector<Index2>& cidx_vec
	);

	void BFSLightBlockAdd(
		Index2 const& c_idx, 
		Index3 const& b_idx,
		vector<Index2>& cidx_vec
	);

private:
	LightSystem() = delete;
	LightSystem(LightSystem const&) = delete;
	LightSystem& operator=(LightSystem const&) = delete;

private:
	void resetLight(Index2 const& c_idx);
	void setSunLight(Index2 const& c_idx, queue<pair<Index2, Index3>> &que);
	void lightBFS(queue<pair<Index2, Index3>>& que);

	/**
	 * 캐릭터의 시야범위가 맵을 벗어나서 청크를 만들 때 빛 갱신 후 갱신된 청크 인덱스들 반환.
	 * 
	 * \param que 갱신이 이루어질 시작점
	 * \return 갱신된 청크 인덱스 목록
	 */
	set<Index2> lightBFSAddChunks(queue<pair<Index2, Index3>>& que);

private:
	/**
	 * 청크와 블록의 인덱스를 반환합니다.(인접한 경우 인접 청크의 청크 인덱스와 블록인덱스 반환)
	 * 
	 * \param c_idx 현재 청크 인덱스
	 * \param b_idx 체크하려는 블록 인덱스(완전 다른 청크가 아닌 현재 청크에서 이동해서)
	 */
	void getIndex(Index2& c_idx, Index3& b_idx) const;

private:
	MapUtils* m_info;
};

