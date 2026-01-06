#include "pch.h"
#include "Map.h"
#include "Chunk.h"
#include "DeferredGraphics.h"
#include "Graphics.h"
#include "Block.h"
#include "ThreadPool/ThreadPool.h"
#include <time.h>

#include <chrono>


vector<vec2> Map::direction = { {0, 16}, {16, 0}, {0, -16}, {-16, 0} };

Map::Map(
	int size_w, 
	int size_h, 
	int fov_chunk,
	HWND hwnd,
	UINT window_w,
	UINT window_h
) : _mapInfo(size_w, size_h, hwnd, window_w, window_h), 
	l_system(&_mapInfo), 
	t_system(&_mapInfo), r_system(&_mapInfo)
{
	auto start = std::chrono::high_resolution_clock::now();

	_chunkFOV = fov_chunk;
	_userSightRadius = _chunkFOV * 8;
	_threadPool = make_unique<ThreadPool>();
	this->thread_cnt = _threadPool->GetThreadCount();

	_mapInfo.loadGame();
	CreateMap();
	this->t_system.createTrees();
	FrustumCulling();
	this->terrainSetVerticesAndIndices();

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> ms_double = end - start;
	std::cout << "실행 시간: " << ms_double.count() << "ms" << std::endl;
}

Map::~Map() = default;

void Map::resetChunk(Index2 const& c_idx)
{
	_mapInfo.ResetChunk(c_idx);
}

void Map::CreateMap()
{
	Index2 chunkIndex;
	Index2 chunkPos;
	vector<Index2> chunkList;
	chunkList.reserve(_mapInfo.size_h * _mapInfo.size_w);
	for (int i = 0; i < _mapInfo.size_h; i++) 
	{
		for (int j = 0; j < _mapInfo.size_w; j++) 
		{
			chunkPos = _mapInfo.s_pos + Index2(j * 16, -i * 16);
			chunkIndex = _mapInfo.getChunkIndex(chunkPos.x, chunkPos.y);
			chunkList.push_back(chunkIndex);
			_mapInfo.chunks[chunkIndex.y][chunkIndex.x] = make_unique<Chunk>(); // new delete는 lock을 걸음
			_threadPool->SetTask([this, chunkIndex, chunkPos](int threadID) {this->t_system.CreateChunk(chunkIndex, chunkPos);});
		}
	}
	_threadPool->Wait();
	
	_mapInfo.SetAdjChunkLightList(this->thread_cnt);
	for (auto chunkIndex : chunkList)
	{
		_threadPool->SetTask([this, chunkIndex](int threadID) { l_system.SetLightChunk(chunkIndex, threadID); });
	}
	_threadPool->Wait();
	//l_system.SetLightAdjChunkInMain();
}

void Map::terrainSetVerticesAndIndices()
{
	for (auto chunkIndex : _mapInfo.GetRenderableChunkListToRead())
	{
		_threadPool->SetTask([this, chunkIndex](int threadID) {this->chunksSetVerticesAndIndices(chunkIndex); });
	}
	//_threadPool->Wait();
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
				if (_mapInfo.findBlock(c_idx, next) > 0) 
				{
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

void Map::chunksSetVerticesAndIndices(const Index2 c_idx)
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

	_mapInfo.chunks[c_idx.y][c_idx.x]->vertices_idx = 0;
	Index2 apos = _mapInfo.chunks[c_idx.y][c_idx.x]->chunk_pos;
	s_idx = 0;
	idx = 0;
	for (int dir = 0; dir < 6; dir++)
	{
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

	_createNewBufferChunkIndices.insert(chunkIndex);
	_createNewChunkIndices.insert(chunkIndex);
	for (auto& dir : direction)
	{
		adjChunkPos = chunkPos + Index2(dir.x, dir.y);
		adjChunkIndex = _mapInfo.findChunkIndex(adjChunkPos.x, adjChunkPos.y);
		if (adjChunkIndex.flag == true && IsChunkInViewDistance(userPos.x, userPos.y, adjChunkPos.x, adjChunkPos.y) == true)
		{
			_createNewBufferChunkIndices.insert(adjChunkIndex);
		}
	}
}

void Map::UpdateChunks()
{
	// test 용 로직
	for (const Index2& chunkIndex : _createNewChunkIndices)
	{
		this->resetChunk(chunkIndex);
		this->t_system.fillChunk(chunkIndex, _mapInfo.chunks[chunkIndex.y][chunkIndex.x]->chunk_pos);
		this->t_system.fillWithUserPlacedBlocks(chunkIndex);
	}


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
	for (const Index2& chunkIndex : _createNewBufferChunkIndices)
	{
		Index2 const& c_idx = chunkIndex;
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

void Map::FrustumCulling()
{
	//TODO: frusutm culling 로직 작성 할 것(청크를 개별 단위에서 묶음으로 고친 후)

	for (int i = 0; i < _mapInfo.size_h; i++) 
	{
		for (int j = 0; j < _mapInfo.size_w; j++) 
		{
			Index2 c_pos = _mapInfo.s_pos + Index2(j * 16, -i * 16);
			Index2 c_idx = _mapInfo.findChunkIndex(c_pos.x, c_pos.y);
			_mapInfo.AddChunkToRenderableChunkList(c_idx);
		}
	}
}

void Map::TestUserPositionCheck(float x, float z)
{
	_createNewChunkIndices.clear();
	_createNewBufferChunkIndices.clear();
	_mapInfo.ResetRenderableChunkList();
	
	// start chunk index loop
	vec2 userSightAABBMin = vec2(x - _chunkFOV * 8, z + _chunkFOV * 8);

	Index2 startChunkIndex = _mapInfo.findChunkIndex(userSightAABBMin.x, userSightAABBMin.y);
	Index2 chunkIndex;
	Index2 chunkPos;
	Index2 startChunkPos = _mapInfo.chunks[startChunkIndex.y][startChunkIndex.x]->chunk_pos;

	// shadow를 위해 시야 범위를 넘어간 곳 까지 만들기 -> fog 가 필요

	for (int i = 0; i <= _chunkFOV; i++)
	{
		for (int j = 0; j <= _chunkFOV; j++)
		{
			chunkPos = startChunkPos + Index2(16 * j, -16 * i);
			chunkIndex = _mapInfo.findChunkIndex(chunkPos.x, chunkPos.y);
			Chunk* chunk = _mapInfo.chunks[chunkIndex.y][chunkIndex.x].get();
			
			if (IsChunkInViewDistance(x, z, chunkPos.x, chunkPos.y) == false)
				continue;

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
			_mapInfo.AddChunkToRenderableChunkList(chunkIndex);
		}
	}
	UpdateChunks();
}
