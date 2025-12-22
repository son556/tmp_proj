#include "pch.h"
#include "Terrain.h"
#include "Chunk.h"
#include "Graphics.h"
#include "Block.h"
#include "TestCam.h"
#include "DeferredGraphics.h"
#include "Entity.h"
//#include "FmodSound.h"

Terrain::Terrain(
	int size_w,
	int size_h,
	HWND hwnd,
	UINT width,
	UINT height,
	int fov_chunk,
	int thread_cnt
)
{
	this->m_manager = make_unique<Map>(
		size_w,
		size_h,
		fov_chunk,
		thread_cnt,
		hwnd,
		width,
		height
	);
}

Terrain::~Terrain()
{
}

void Terrain::Render()
{
	this->m_manager->r_system.Render();
}

ComPtr<ID3D11ShaderResourceView>& Terrain::getSRV()
{
	return this->m_manager->r_system.getSRV();
}

void Terrain::putBlock(
	vec3 const& ray_pos,
	vec3 const& ray_dir,
	int type
)
{
	WorldIndex widx = this->m_manager->_mapInfo.pickBlock(ray_pos, ray_dir);
	if (widx.flag) {
		WorldIndex add_idx;
		Index2& cidx = widx.c_idx;
		Index3& bidx = widx.b_idx;
		vec3 const& pos = widx.pos;
		if (widx.dir == 0) {
			if (ray_pos.y > pos.y && pos.y + 1 < 256) {
				add_idx = this->m_manager->
					_mapInfo.getBlockIndex(pos.x, pos.y + 1, pos.z);
			}
			else if (ray_pos.y < pos.y && pos.y - 1 > -1) {
				add_idx = this->
					m_manager->_mapInfo.getBlockIndex(pos.x, pos.y - 1, pos.z);
			}
		}
		else if (widx.dir == 1) {
			if (ray_pos.z < pos.z) {
				add_idx = this->m_manager->_mapInfo.getBlockIndex(pos.x, pos.y, pos.z - 1);
			}
			else {
				add_idx = this->m_manager->_mapInfo.getBlockIndex(pos.x, pos.y, pos.z + 1);
			}
		}
		else {
			if (ray_pos.x < pos.x) {
				add_idx = this->
					m_manager->_mapInfo.getBlockIndex(pos.x - 1, pos.y, pos.z);
			}
			else {
				add_idx = this->
					m_manager->_mapInfo.getBlockIndex(pos.x + 1, pos.y, pos.z);
			}
		}
		if (add_idx.flag) {
			vector<Index2> v_idx;
			cidx = add_idx.c_idx;
			bidx = add_idx.b_idx;
			Index2 adj_idx;
			Index2 const& cpos = 
				this->m_manager->_mapInfo.chunks[cidx.y][cidx.x]->chunk_pos;
			
			if (entity->checkAABBWithEntity(add_idx.pos))// 캐릭터가 있으면 블록 안놓음
				return;

			this->m_manager->_mapInfo.writeBookAboutBlockStatus(
				this->m_manager->_mapInfo.chunks[cidx.y][cidx.x]->chunk_pos,
				bidx,
				static_cast<BlockType>(this->m_manager->_mapInfo.findBlock(cidx, bidx)),
				static_cast<BlockType>(type)
			);

			this->m_manager->_mapInfo.addBlock(cidx, bidx, type);
			
			// height map 갱신
			if (bidx.y + 1 > this->m_manager->_mapInfo.findHeight(cidx, bidx.x, bidx.z))
				this->m_manager->_mapInfo.setHeight(cidx, bidx.x, bidx.z, bidx.y + 1);
			
			if (bidx.x == 0) {
				adj_idx = this->m_manager->_mapInfo.findChunkIndex(cpos.x - 16, cpos.y);
				if (adj_idx.flag && this->m_manager->_mapInfo.findBlock(adj_idx, 15, bidx.y, bidx.z)) {
					this->m_manager->_mapInfo.chunks[adj_idx.y][adj_idx.x]->vertices_idx = 0;
					v_idx.push_back(adj_idx);
				}
			}
			if (bidx.x == 15) {
				adj_idx = this->m_manager->_mapInfo.findChunkIndex(cpos.x + 16, cpos.y);
				if (adj_idx.flag && this->m_manager->_mapInfo.findBlock(adj_idx, 0, bidx.y, bidx.z)) {
					this->m_manager->_mapInfo.chunks[adj_idx.y][adj_idx.x]->vertices_idx = 0;
					v_idx.push_back(adj_idx);
				}
			}
			if (bidx.z == 15) {
				adj_idx = this->m_manager->_mapInfo.findChunkIndex(cpos.x, cpos.y - 16);
				if (adj_idx.flag && this->m_manager->_mapInfo.findBlock(adj_idx, bidx.x, bidx.y, 0)) {
					this->m_manager->_mapInfo.chunks[adj_idx.y][adj_idx.x]->vertices_idx = 0;
					v_idx.push_back(adj_idx);
				}
			}
			if (bidx.z == 0) {
				adj_idx = this->m_manager->_mapInfo.findChunkIndex(cpos.x, cpos.y + 16);
				if (adj_idx.flag && this->m_manager->_mapInfo.findBlock(adj_idx, bidx.x, bidx.y, 15)) {
					this->m_manager->_mapInfo.chunks[adj_idx.y][adj_idx.x]->vertices_idx = 0;
					v_idx.push_back(adj_idx);
				}
			}
			int16& max_h = this->m_manager->_mapInfo.chunks[cidx.y][cidx.x]->max_h;
			max_h = max(max_h, bidx.y + 1);
			if (type > 0 && type != BlockType::OAK_LEAVES)
				this->m_manager->l_system.BFSLightBlockAdd(cidx, bidx, v_idx);
			v_idx.push_back(cidx);
			this->m_manager->chunksSetVerticesAndIndices(v_idx, 0, v_idx.size());
			//FmodSound::playPutBlockSound(BlockType::AIR); // 놓는 소리 고정
		}
	}
}

void Terrain::deleteBlock(vec3 const& ray_pos, vec3 const& ray_dir)
{
	WorldIndex widx = this->m_manager->_mapInfo.pickBlock(ray_pos, ray_dir);
	if (widx.flag) {
		int type = this->m_manager->_mapInfo.findBlock(widx.c_idx, widx.b_idx);
		this->m_manager->_mapInfo.writeBookAboutBlockStatus(
			this->m_manager->_mapInfo.chunks[widx.c_idx.y][widx.c_idx.x]->chunk_pos,
			widx.b_idx,
			static_cast<BlockType>(type),
			BlockType::AIR
		);
		
		this->m_manager->_mapInfo.addBlock(widx.c_idx, widx.b_idx, 0);
		
		// height map 갱신
		if (widx.b_idx.y + 1 == this->m_manager->_mapInfo.findHeight(
			widx.c_idx, widx.b_idx.x, widx.b_idx.z)) {
			for (int y = widx.b_idx.y; y > -1; y--) {
				if (this->m_manager->_mapInfo.findBlock(widx.c_idx, widx.b_idx.x, y, widx.b_idx.z)) {
					this->m_manager->_mapInfo.setHeight(widx.c_idx, widx.b_idx.x, widx.b_idx.z, y + 1);
					break;
				}
			}
		}
		
		int16& max_h = 
			this->m_manager->_mapInfo.chunks[widx.c_idx.y][widx.c_idx.x]->max_h;
		max_h = max(max_h, widx.b_idx.y - 1);
		
		vector<Index2> v_idx;
		v_idx.push_back(widx.c_idx);
		Index2 cidx;
		Index2 const& pos = this->m_manager->_mapInfo.chunks[widx.c_idx.y][widx.c_idx.x]->chunk_pos;
		
		// 인접 청크도 vertex 갱신
		cidx = this->m_manager->_mapInfo.findChunkIndex(pos.x - 16, pos.y);
		if (cidx.flag && widx.b_idx.x == 0) {
			this->m_manager->_mapInfo.chunks[cidx.y][cidx.x]->vertices_idx = 0;
			v_idx.push_back(cidx);
		}
		cidx = this->m_manager->_mapInfo.findChunkIndex(pos.x + 16, pos.y);
		if (cidx.flag && widx.b_idx.x == 15) {
			this->m_manager->_mapInfo.chunks[cidx.y][cidx.x]->vertices_idx = 0;
			v_idx.push_back(cidx);
		}

		cidx = this->m_manager->_mapInfo.findChunkIndex(pos.x, pos.y + 16);
		if (cidx.flag && widx.b_idx.z == 0) {
			this->m_manager->_mapInfo.chunks[cidx.y][cidx.x]->vertices_idx = 0;
			v_idx.push_back(cidx);
		}

		cidx = this->m_manager->_mapInfo.findChunkIndex(pos.x, pos.y - 16);
		if (cidx.flag && widx.b_idx.z == 15) {
			this->m_manager->_mapInfo.chunks[cidx.y][cidx.x]->vertices_idx = 0;
			v_idx.push_back(cidx);
		}

		// 그림자 갱신
		if (type > 0 && type != BlockType::OAK_LEAVES)
			this->m_manager->l_system.BFSLightBlockDelete(widx.c_idx, widx.b_idx, v_idx);
		this->m_manager->chunksSetVerticesAndIndices(v_idx, 0, v_idx.size());
		//FmodSound::playPutBlockSound(static_cast<BlockType>(type));
	}
}

void Terrain::saveGame()
{
	this->m_manager->_mapInfo.saveGame();
}

WorldIndex Terrain::getBlock(float x, float y, float z)
{
	return this->m_manager->_mapInfo.getBlockIndex(x, y, z);
}

WorldIndex Terrain::getBlock(vec3 world_pos)
{
	return this->m_manager->_mapInfo.getBlockIndex(world_pos.x, world_pos.y, world_pos.z);
}

void Terrain::setSightChunk(int cnt)
{
	int max_fov = min(this->m_manager->_mapInfo.size_h - 1, 
		this->m_manager->_mapInfo.size_w - 1);
	this->m_manager->setSightChunk(min(max_fov, cnt));
}

void Terrain::userPositionCheck(float x, float z)
{
	this->m_manager->userPositionCheck(x, z);
}

int16 Terrain::getHeight(float x, float z) const
{
	WorldIndex w_idx = m_manager->_mapInfo.getBlockIndex(x, 0, z);
	return m_manager->_mapInfo.findHeight(w_idx.c_idx, w_idx.b_idx.x, w_idx.b_idx.z);
}

void Terrain::testClickLightBlock(
	vec3 const& ray_pos,
	vec3 const& ray_dir)
{
	WorldIndex widx = this->m_manager->_mapInfo.pickBlock(ray_pos, ray_dir);
	if (widx.flag) {
		Index2& cidx = widx.c_idx;
		Index3& bidx = widx.b_idx;

		cout << "now height: " << 
			this->m_manager->_mapInfo.findHeight(cidx, bidx.x, bidx.z) << endl;
		cout << "block type: " << widx.block_type << endl;
		cout << "chunk idx: " << cidx.x << ' ' << cidx.y << endl;
		cout << "block idx x: " << bidx.x << ' ' << bidx.y << ' ' << bidx.z << endl << endl;
	}
}
