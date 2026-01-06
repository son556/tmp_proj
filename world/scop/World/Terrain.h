#pragma once

#include "Map.h"


class Terrain
{
public:
	Terrain(int size_w, int size_h, HWND hwnd, UINT width, UINT height, int fov_chunk);
	~Terrain();
	void setSightChunk(int cnt);
	void userPositionCheck(float x, float z);
	int16 getHeight(float x, float z) const;
	void putBlock(vec3 const& ray_pos, vec3 const& ray_dir, int type);
	void deleteBlock(vec3 const& ray_pos, vec3 const& ray_dir);
	void saveGame();

	WorldIndex getBlock(float x, float y, float z);
	WorldIndex getBlock(vec3 world_pos);


public: // test
	void testClickLightBlock(vec3 const& ray_pos, 
		vec3 const& ray_dir);

public:
	void Render();
	ComPtr<ID3D11ShaderResourceView>& getSRV();

private:
	unique_ptr<Map> m_manager;
	set<string> file_book;
	map<vec3, uint32> object_book;
	map<vec3, shared_ptr<Chunk>> tmp_storage;
};

