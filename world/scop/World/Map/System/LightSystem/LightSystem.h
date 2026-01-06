#pragma once

#include "MapUtils.h"

class LightSystem
{
public:
	LightSystem(MapUtils* minfo);
	~LightSystem() = default;
	
public:
	void SetLightChunk(const Index2 chunkIndex, const int threadID);
	void SetLightAdjChunkInMain();

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
	void setSunLight(Index2 const& c_idx, queue<tuple<Index2, Index3, uint8>>& que);
	void lightBFS(queue<tuple<Index2, Index3, uint8>>& que, const int threadID, const Index2 nowChunkIndex);
	bool AtomicMax(uint8& target, uint8 cmp);

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

