#include "pch.h"
#include "Map.h"
#include "Chunk.h"
#include "DeferredGraphics.h"
#include "Graphics.h"
#include "Block.h"
#include <time.h>

vector<vec2> Map::direction = { {0, 16}, {16, 0}, {0, -16}, {-16, 0} };

Map::Map(
	int size_w, 
	int size_h, 
	int fov_chunk, 
	int thread_cnt,
	HWND hwnd,
	UINT window_w,
	UINT window_h
) : _mapInfo(size_w, size_h, hwnd, window_w, window_h), 
	l_system(&_mapInfo), 
	t_system(&_mapInfo), r_system(&_mapInfo)
{
	_chunkFOV = fov_chunk;
	this->thread_cnt = thread_cnt;
	_userSightRadius = _chunkFOV * 8;

	_mapInfo.loadGame();
	this->t_system.createHeightMap();
	this->l_system.createLightMap();
	this->t_system.createTrees();
	this->terrainSetVerticesAndIndices();

}


void Map::resetChunk(Index2 const& c_idx)
{
	for (int y = 0; y < 256; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				_mapInfo.addBlock(c_idx, x, y, z, 0);
				_mapInfo.setLight(c_idx, x, y, z, 0);
			}
		}
	}
}

void Map::terrainSetVerticesAndIndices()
{
	vector<Index2> v_idxs;
	int c_cnt = (_mapInfo.size_h - 2) * (_mapInfo.size_w - 2);
	int t = c_cnt / this->thread_cnt;
	int m = c_cnt % this->thread_cnt;

	Index2 pos, c_idx;
	for (int i = 1; i < _mapInfo.size_h - 1; i++) {
		for (int j = 1; j < _mapInfo.size_w - 1; j++) {
			pos = _mapInfo.s_pos + Index2(16 * j, -16 * i);
			c_idx = _mapInfo.findChunkIndex(pos.x, pos.y);
			v_idxs.push_back(c_idx);
		}
	}

	int st = 0;
	int siz;
	for (int i = 0; i < this->thread_cnt; i++) {
		if (m) {
			siz = t + 1;
			m--;
		}
		else
			siz = t;
		this->chunksSetVerticesAndIndices(v_idxs, st, st + siz);
		st = st + siz;
	}
}

void Map::vertexAndIndexGenerator(
	Index2 const& c_idx,
	Index2 const& adj_idx,
	Index3 const& move,
	int dir,
	vector<VertexGeo>& vertices,
	vector<uint32>* indices,
	uint32* index
)
{
	int16 const& max_h = _mapInfo.chunks[c_idx.y][c_idx.x]->max_h;
	for (int y = 0; y < max_h; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				int type = _mapInfo.findBlock(c_idx, x, y, z);
				if (type <= 0)
					continue;
				Index3 next(x + move.x, y + move.y, z - move.z);
				int adj_type = _mapInfo.findBlock(c_idx, next);
				if (adj_type > 0 && adj_type != BlockType::OAK_LEAVES)
					continue;
			
				Block::addFaceQuadPosAndTex(
					_mapInfo.chunks[c_idx.y][c_idx.x]->start_pos,
					dir,
					x, y, z, type,
					vertices
				);
				if (indices) {
					Block::addBlockFaceIndices(*index, *indices);
					*index += 4;
				}
			}
		}
	}
}

void Map::vertexShadowGenerator(
	Index2 const& c_idx,
	Index3 const& move,
	int dir,
	vector<VertexShadow>& vertices,
	vector<uint32>* indices,
	uint32* index
)
{
	int16 const& max_h = _mapInfo.chunks[c_idx.y][c_idx.x]->max_h;
	int shadow_flag;
	Index2 const& cpos = _mapInfo.chunks[c_idx.y][c_idx.x]->chunk_pos;
	Index2 const& sv_pos = _mapInfo.sv_pos;
	Index2 const& ev_pos = _mapInfo.ev_pos;
	for (int y = 0; y < max_h; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				int type = _mapInfo.findBlock(c_idx, x, y, z);
				if (type <= 0)
					continue;
				Index3 next(x + move.x, y + move.y, z - move.z);
				if (_mapInfo.findBlock(c_idx, next) > 0) {
					bool c_flag = true;
					if (cpos.x == sv_pos.x && next.x < 0)
						c_flag = false;
					else if (cpos.x == ev_pos.x - 16 && next.x > 15)
						c_flag = false;
					else if (cpos.y == sv_pos.y && next.z < 0)
						c_flag = false;
					else if (cpos.y == ev_pos.y + 16 && next.z > 15)
						c_flag = false;
					if (c_flag)
						continue;
				}
				shadow_flag = _mapInfo.findLight(c_idx, next.x, next.y, next.z);
				Block::addBlockFacePosAndTex(
					_mapInfo.chunks[c_idx.y][c_idx.x]->start_pos,
					x, y, z, dir,
					shadow_flag,
					vertices
				);
				if (indices) {
					Block::addBlockFaceIndices(*index, *indices);
					*index += 4;
				}
			}
		}
	}
}

void Map::vertexAndIndexGeneratorTP(Index2 const& c_idx)
{
	Chunk& chunk = *(_mapInfo.chunks[c_idx.y][c_idx.x]);
	uint32& vu_idx = chunk.tp_chunk.vertices_idx_up;
	vector<VertexColor>& vertices_up = chunk.tp_chunk.vertices_up;
	vector<uint32>& indices_up = chunk.tp_chunk.indices_up;

	uint32& vd_idx = chunk.tp_chunk.vertices_idx_down;
	vector<VertexColor>& vertices_down = chunk.tp_chunk.vertices_down;
	vector<uint32>& indices_down = chunk.tp_chunk.indices_down;
	
	chunk.tp_chunk.reset();

	for (int y = 0; y <= chunk.max_h; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				int type = _mapInfo.findBlock(c_idx, x, y, z);
				if (type < 0 && type != BlockType::WATER) {
					for (int i = 0; i < 6; i++) {
						if (y >= WATER_HEIGHT) {
							Block::addBlockFacePosAndCol(
								chunk.start_pos, x, y, z, i, type, 
								vertices_up);
							Block::addBlockFaceIndices(vu_idx, indices_up);
							vu_idx += 4;
						}
						else {
							Block::addBlockFacePosAndCol(
								chunk.start_pos, x, y, 
								z, i, type, vertices_down);
							Block::addBlockFaceIndices(
								vd_idx, indices_down);
							vd_idx += 4;
						}
					}
				}
			}
		}
	}
	chunk.tp_chunk.update(d_graphic->getDevice(), true);
	chunk.tp_chunk.update(d_graphic->getDevice(), false);
}

void Map::vertexAndIndexGeneratorWater(Index2 const& c_idx)
{
	Chunk& chunk = *(_mapInfo.chunks[c_idx.y][c_idx.x]);
	vector<VertexWater>& vertices = chunk.w_chunk.vertices;
	vector<uint32>& indices = chunk.w_chunk.indices;
	uint32& v_idx = chunk.w_chunk.vertices_idx;
	chunk.w_chunk.reset();
	for (int z = 0; z < 16; z++) {
		for (int x = 0; x < 16; x++) {
			int type = 
				_mapInfo.findBlock(c_idx, x, WATER_HEIGHT - 1, z);
			if (type != BlockType::WATER)
				continue;
			_mapInfo;
			vec3 xyz(x, WATER_HEIGHT - 1, z);
			Block::addBlockFaceWater(chunk.start_pos, _mapInfo.sv_pos,
				_mapInfo.size_w - 2, _mapInfo.size_h - 2,
				xyz, vertices);
			Block::addBlockFaceIndices(v_idx, indices);
			v_idx += 4;
		}
	}
	chunk.w_chunk.update(d_graphic->getDevice());
}

void Map::setSightChunk(int chunk_cnt)
{
	_chunkFOV = chunk_cnt;
}

void Map::chunksSetVerticesAndIndices(
	vector<Index2> const& v_idx,
	int st,
	int ed
)
{
	static const Index3 move_arr[6] = {
		Index3(0, 1, 0),
		Index3(0, -1, 0),
		Index3(0, 0, -1),
		Index3(0, 0, 1),
		Index3(-1, 0, 0),
		Index3(1, 0, 0)
	};
	vector<VertexGeo> vertices_geo;
	vector<VertexShadow> vertices_shadow;
	vector<uint32> indices;
	vector<uint32> s_indices;
	uint32 s_idx;
	uint32 idx;
	ed = min(ed, v_idx.size());
	for (int i = st; i < ed; i++) {
		Index2 const& c_idx = v_idx[i];
		_mapInfo.chunks[c_idx.y][c_idx.x]->vertices_idx = 0;
		Index2 apos = _mapInfo.chunks[c_idx.y][c_idx.x]->chunk_pos;
		s_idx = 0;
		idx = 0;
		for (int dir = 0; dir < 6; dir++) {
			Index2 pos = apos + Index2(16 * move_arr[dir].x,
				16 * move_arr[dir].z);
			Index2 adj_idx = _mapInfo.findChunkIndex(pos.x, pos.y);
			this->vertexAndIndexGenerator(
				c_idx,
				adj_idx,
				move_arr[dir],
				dir,
				vertices_geo,
				&indices,
				&idx
			);
			this->vertexShadowGenerator(
				c_idx,
				move_arr[dir],
				dir,
				vertices_shadow,
				&s_indices,
				&s_idx
			);
		}
		this->vertexAndIndexGeneratorTP(c_idx);
		this->vertexAndIndexGeneratorWater(c_idx);
		_mapInfo.chunks[c_idx.y][c_idx.x]->createGeoBuffer(
			d_graphic->getDevice(),
			vertices_geo,
			indices
		);
		_mapInfo.chunks[c_idx.y][c_idx.x]->createShadowBuffer(
			d_graphic->getDevice(),
			vertices_shadow,
			s_indices
		);
		vertices_shadow.clear();
		vertices_geo.clear();
		s_indices.clear();
		indices.clear();
	}
}

int Map::checkTerrainBoundary(float x, float z) const
{
	float r = 16.f * _chunkFOV;
	int mask = 0;
	if (x - r < _mapInfo.sv_pos.x)
		mask |= 1 << 0; // left out(-x)
	if (x + r > _mapInfo.ev_pos.x)
		mask |= 1 << 1; // right out(+x)
	if (z + r > _mapInfo.sv_pos.y)
		mask |= 1 << 2; // back out (+z)
	if (z - r < _mapInfo.ev_pos.y)
		mask |= 1 << 3; // front out (-z)
	return mask;
}

void Map::userPositionCheck(float x, float z)
{
	int mask = this->checkTerrainBoundary(x, z);
	vector<Index2> v_idxs, f_idxs;
	Index2 cidx, cpos;
	if (mask == 1) { // out left
		for (int i = 0; i < _mapInfo.size_h; i++) {
			if (i && i < _mapInfo.size_h - 1) { // 보여주기
				cpos = Index2(_mapInfo.s_pos.x, _mapInfo.s_pos.y - 16 * i);
				cidx = _mapInfo.findChunkIndex(cpos.x, cpos.y);
				v_idxs.push_back(cidx);
			}
			cpos.x = _mapInfo.s_pos.x - 16;
			cpos.y = _mapInfo.s_pos.y - 16 * i;
			cidx = _mapInfo.getChunkIndex(cpos.x, cpos.y);
			_mapInfo.chunks[cidx.y][cidx.x]->reset();
			_mapInfo.chunks[cidx.y][cidx.x]->setPos(cpos);
			f_idxs.push_back(cidx);
		}
		_mapInfo.sv_pos.x -= 16;
		_mapInfo.s_pos.x -= 16;
		_mapInfo.ev_pos.x -= 16;
	}
	else if (mask == 2) 
	{ // out right
		for (int i = 0; i < _mapInfo.size_h; i++) 
		{
			if (i && i < _mapInfo.size_h - 1) 
			{
				cpos = Index2(_mapInfo.ev_pos.x, _mapInfo.s_pos.y - 16 * i);
				cidx = _mapInfo.findChunkIndex(cpos.x, cpos.y);
				v_idxs.push_back(cidx);
			}
			cpos.x = _mapInfo.ev_pos.x + 16;
			cpos.y = _mapInfo.s_pos.y - 16 * i;
			cidx = _mapInfo.getChunkIndex(cpos.x, cpos.y);
			_mapInfo.chunks[cidx.y][cidx.x]->reset();
			_mapInfo.chunks[cidx.y][cidx.x]->setPos(cpos);
			f_idxs.push_back(cidx);
		}
		_mapInfo.sv_pos.x += 16;
		_mapInfo.s_pos.x += 16;
		_mapInfo.ev_pos.x += 16;
	}
	else if (mask == 4) 
	{ // out +z
		for (int i = 0; i < _mapInfo.size_w; i++) 
		{
			if (i && i < _mapInfo.size_w - 1) 
			{
				cpos = Index2(_mapInfo.s_pos.x + 16 * i, _mapInfo.s_pos.y);
				cidx = _mapInfo.findChunkIndex(cpos.x, cpos.y);
				v_idxs.push_back(cidx);
			}
			cpos.x = _mapInfo.s_pos.x + 16 * i;
			cpos.y = _mapInfo.s_pos.y + 16;
			cidx = _mapInfo.getChunkIndex(cpos.x, cpos.y);
			_mapInfo.chunks[cidx.y][cidx.x]->reset();
			_mapInfo.chunks[cidx.y][cidx.x]->setPos(cpos);
			f_idxs.push_back(cidx);
		}
		_mapInfo.sv_pos.y += 16;
		_mapInfo.s_pos.y += 16;
		_mapInfo.ev_pos.y += 16;
	}
	else if (mask == 8) { // out -z
		for (int i = 0; i < _mapInfo.size_w; i++) 
		{
			if (i && i < _mapInfo.size_w - 1) 
			{
				cpos = Index2(_mapInfo.s_pos.x + 16 * i, _mapInfo.ev_pos.y);
				cidx = _mapInfo.findChunkIndex(cpos.x, cpos.y);
				v_idxs.push_back(cidx);
			}
			cpos.x = _mapInfo.s_pos.x + 16 * i;
			cpos.y = _mapInfo.ev_pos.y - 16;
			cidx = _mapInfo.getChunkIndex(cpos.x, cpos.y);
			_mapInfo.chunks[cidx.y][cidx.x]->reset();
			_mapInfo.chunks[cidx.y][cidx.x]->setPos(cpos);
			f_idxs.push_back(cidx);
		}
		_mapInfo.sv_pos.y -= 16;
		_mapInfo.s_pos.y -= 16;
		_mapInfo.ev_pos.y -= 16;
	}
	if (f_idxs.size()) 
	{
		for (Index2& c_idx : f_idxs) 
		{
			this->resetChunk(c_idx);
			this->t_system.fillChunk(c_idx, 
				_mapInfo.chunks[c_idx.y][c_idx.x]->chunk_pos);
			this->t_system.fillWithUserPlacedBlocks(c_idx);
			this->l_system.createLightMap(f_idxs, v_idxs);
		}
	}
	if (v_idxs.size())
		this->threadFunc(v_idxs, mask);
}

void Map::threadFunc(vector<Index2>& vec, int dir)
{
	int v_size = vec.size();
	this->t_system.createTrees(vec, dir); // 새로 만든나무와 인접했던 나무 추가
	for (int i = 0; i < vec.size(); i++) {
		this->t_system.fillWithUserPlacedBlocks(vec[i]);
	}

	set<Index2> book;
	for (int i = 0; i < vec.size(); i++)
		book.insert(vec[i]);

	Index2 cidx;
	Index2 cpos;
	if (dir == 1) {
		for (int i = 1; i < _mapInfo.size_h - 1; i++) {
			cpos = _mapInfo.sv_pos + Index2(16, -16 * (i - 1));
			cidx = _mapInfo.findChunkIndex(cpos.x, cpos.y);
			if (book.find(cidx) == book.end())
				vec.push_back(cidx);
		}
	}

	else if (dir == 2) {
		for (int i = 1; i < _mapInfo.size_h - 1; i++) {
			cpos = Index2(_mapInfo.ev_pos.x - 32,
				_mapInfo.sv_pos.y - 16 * (i - 1));
			cidx = _mapInfo.findChunkIndex(cpos.x, cpos.y);
			if (book.find(cidx) == book.end())
				vec.push_back(cidx);
		}
	}

	else if (dir == 4) {
		for (int i = 1; i < _mapInfo.size_w - 1; i++) {
			cpos = _mapInfo.sv_pos + Index2(16 * (i - 1), -16);
			cidx = _mapInfo.findChunkIndex(cpos.x, cpos.y);
			if (book.find(cidx) == book.end())
				vec.push_back(cidx);
		}
	}

	else {
		for (int i = 1; i < _mapInfo.size_w - 1; i++) {
			cpos = Index2(_mapInfo.sv_pos.x + 16 * (i - 1),
				_mapInfo.ev_pos.y + 32);
			cidx = _mapInfo.findChunkIndex(cpos.x, cpos.y);
			if (book.find(cidx) == book.end())
				vec.push_back(cidx);
		}
	}

	int t = vec.size() / this->thread_cnt;
	int m = vec.size() % this->thread_cnt;
	int st = 0;
	int siz;
	int idx = 0;
	for (int i = 0; i < this->thread_cnt; i++) {
		if (m) {
			siz = t + 1;
			m--;
		}
		else
			siz = t;
		this->chunksSetVerticesAndIndices(vec, st, st + siz);
		st = st + siz;
	}
}


bool Map::IsChunkInViewDistance(float userPosX, float userPosZ, float chunkPosX, float chunkPosZ)
{
	vec2 chunkCenterToUser = vec2(userPosX - (chunkPosX - 8), userPosZ - (chunkPosZ - 8));
	vec2 aabbHalfExtents = vec2(8, 8);
	vec2 clamped = chunkCenterToUser;
	clamped.Clamp(-aabbHalfExtents, aabbHalfExtents);
	vec2 closestPoint = vec2(chunkPosX, chunkPosZ) + clamped;

	float radius = _chunkFOV * 8;

	if (vec2::DistanceSquared(closestPoint, {userPosX, userPosZ}) < radius * radius)
		return true;
	return false;
}

void Map::CheckChunkVertices(const Index2& chunkIndex, vec2 userPos)
{
	const Index2 chunkPos = _mapInfo.chunks[chunkIndex.y][chunkIndex.x]->chunk_pos;
	Index2 adjChunkIndex;
	Index2 adjChunkPos;

	_resetChunkVerticesList.insert(chunkIndex);
	for (auto& dir : direction)
	{
		adjChunkPos = chunkPos + Index2(dir.x, dir.y);
		adjChunkIndex = _mapInfo.findChunkIndex(adjChunkPos.x, adjChunkPos.y);
		if (adjChunkIndex.flag && IsChunkInViewDistance(userPos.x, userPos.y, adjChunkPos.x, adjChunkPos.y))
		{
			_resetChunkVerticesList.insert(adjChunkIndex);
		}
	}
}

void Map::TestUserPositionCheck(float x, float z)
{
	_mapInfo.ResetRenderableChunkIndices();
	_resetChunkVerticesList.clear();
	_renderChunkIndices.clear();
	
	// start chunk index loop
	vec2 userSightAABBMin = vec2(x - _chunkFOV * 8, z + _chunkFOV * 8);

	Index2 startChunkIndex = _mapInfo.getChunkIndex(userSightAABBMin.x, userSightAABBMin.y);
	Index2 chunkIndex;
	Index2 chunkPos;
	Index2 startChunkPos = _mapInfo.chunks[startChunkIndex.y][startChunkIndex.x]->chunk_pos;

	// shadow를 위해 시야 범위를 넘어간 곳 까지 만들기 -> fog 가 필요

	for (int i = 0; i <= _chunkFOV; i++)
	{
		for (int j = 0; j <= _chunkFOV; j++)
		{
			chunkPos = startChunkPos + Index2(16 * i, -16 * j);
			chunkIndex = _mapInfo.findChunkIndex(chunkPos.x, chunkPos.y);
			Chunk* chunk = _mapInfo.chunks[chunkIndex.y][chunkIndex.x].get();
			
			if (IsChunkInViewDistance(x, z, chunkPos.x, chunkPos.y) == false)
			{
				if (chunkIndex.flag == true) // 시야 범위 벗어난 경우
					chunk->render_flag = false;
				continue;
			}

			if (chunkIndex.flag == false) // 새로 만들어야 하는 부분 (빛 계산)
			{
				if (chunk == nullptr)
				{
					_mapInfo.chunks[chunkIndex.y][chunkIndex.x] = make_unique<Chunk>();
					chunk = _mapInfo.chunks[chunkIndex.y][chunkIndex.x].get();
				}
				else
					chunk->reset();
				chunk->setPos(chunkPos);
				CheckChunkVertices(chunkIndex, { x, z });
			}
			chunk->render_flag = true;
			_renderChunkIndices.emplace_back(chunkIndex);
		}
	}
}
