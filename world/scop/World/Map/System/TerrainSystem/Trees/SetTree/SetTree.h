#pragma once

class MapUtils;
class PerlinNoise;
struct Index2;
struct Index3;

class SetTree
{
public:
	SetTree(MapUtils* minfo, PerlinNoise* noise);
	void createTrees();
	void createTrees(vector<Index2>& chunks, int dir);

private:
	void makeTree(Index2 const& c_idx);
	void putTree(Index2 const& c_idx, int x, int y, int z);

	void checkChunk(
		Index2 const& c_idx,
		int x, int y, int z,
		Index2& res_cidx,
		Index3& res_bidx
	);
	void resetChunkHeight(
		Index2 const& c_idx,
		int x, int z, int h
	);

private:
	MapUtils* m_info;
	PerlinNoise* p_noise;
};

