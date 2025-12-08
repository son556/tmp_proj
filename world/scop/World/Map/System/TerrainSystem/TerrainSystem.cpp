#include "pch.h"
#include "TerrainSystem.h"
#include "MapUtils.h"
#include "Chunk.h"
#include "Trees.h"

TerrainSystem::TerrainSystem(MapUtils* minfo)
	: m_info(minfo), perlin_noise(), 
	s_water(minfo), s_tree(minfo, &perlin_noise), s_cave(minfo, &perlin_noise)
{
}

void TerrainSystem::fillChunk(Index2 const& c_idx, Index2 const& c_pos)
{
	float x, sy, z;
	float offset = 0.000001f;
	int16& max_h = this->m_info->chunks[c_idx.y][c_idx.x]->max_h;
	for (int i = 0; i < 16; i++) {
		z = (c_pos.y - i + offset) / 32.f;
		for (int j = 0; j < 16; j++) {
			x = (c_pos.x + j + offset) / 32.f;
			double h_ = this->perlin_noise.getNoise2D(x, z, 3, 0.5);
			int16 h = static_cast<int16>((h_ + 0.8) * 0.5 * 50.f);
			max_h = max(max_h, h);
			this->m_info->setHeight(c_idx, j, i, h);
			for (int y = 0; y < h; y++) {
				this->m_info->addBlock(c_idx, j, y, i, 1);
			}
		}
	}
	this->s_water.fillWater(c_idx);
	this->s_cave.makeCave(c_idx, c_pos);
}

void TerrainSystem::createHeightMap()
{
	for (int i = 0; i < this->m_info->size_h; i++) {
		for (int j = 0; j < this->m_info->size_w; j++) {
			Index2 c_pos = this->m_info->s_pos + Index2(j * 16, -i * 16);
			Index2 c_idx;
			c_idx = this->m_info->getChunkIndex(c_pos.x, c_pos.y);
			this->m_info->chunks[c_idx.y][c_idx.x] = make_shared<Chunk>();
			this->m_info->chunks[c_idx.y][c_idx.x]->start_pos =
				vec3(c_pos.x + 0.5f, 0.5f, c_pos.y - 0.5f);
			this->m_info->chunks[c_idx.y][c_idx.x]->chunk_pos = c_pos;
			this->fillChunk(c_idx, c_pos);
		}
	}
}

void TerrainSystem::createTrees()
{
	this->s_tree.createTrees();
	for (int i = 0; i < this->m_info->size_h; i++) {
		for (int j = 0; j < this->m_info->size_w; j++) {
			Index2 c_pos = this->m_info->s_pos + Index2(j * 16, -i * 16);
			Index2 c_idx;
			c_idx = this->m_info->getChunkIndex(c_pos.x, c_pos.y);
			this->fillWithUserPlacedBlocks(c_idx);
		}
	}
}

void TerrainSystem::createTrees(vector<Index2>& chunks, int dir)
{
	this->s_tree.createTrees(chunks, dir);
}

void TerrainSystem::fillWithUserPlacedBlocks(Index2 const& c_idx)
{
	map<Index3, Index2> const* const book = this->m_info->
		getUserPlacedBlocks(this->m_info->chunks[c_idx.y][c_idx.x]->chunk_pos);
	if (book == nullptr)
		return;

	int16& max_h = this->m_info->chunks[c_idx.y][c_idx.x]->max_h;
	for (auto it = book->begin(); it != book->end(); it++) {
		this->m_info->addBlock(c_idx, it->first, it->second.x);
		if (it->second.x)
			max_h = max(max_h, it->first.y + 1);
	}
}

