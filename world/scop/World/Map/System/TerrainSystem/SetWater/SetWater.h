#pragma once

class MapUtils;
struct Index2;

class SetWater
{
public:
	SetWater(MapUtils* m_info);
	void fillWater(Index2 const& c_idx);

private:
	MapUtils* m_info;
};

