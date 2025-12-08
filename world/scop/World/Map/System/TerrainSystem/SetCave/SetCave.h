#pragma once

class MapUtils;
class PerlinNoise;
struct Index2;

class SetCave
{
public:
	SetCave(MapUtils* minfo, PerlinNoise* noise);
	void makeCave(Index2 const& c_idx, Index2 const& c_pos);

private:
	int getAdjInfo(Index2 const& c_idx, int x, int y, int z, int dir);

private:
	MapUtils* m_info;
	PerlinNoise* p_noise;
};

