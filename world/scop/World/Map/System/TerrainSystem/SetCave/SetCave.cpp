#include "pch.h"
#include "SetCave.h"
#include "MapUtils.h"
#include "PerlinNoise.h"
#include "Chunk.h"

SetCave::SetCave(MapUtils* minfo, PerlinNoise* noise) 
	: m_info(minfo), p_noise(noise)
{
}




void SetCave::makeCave(Index2 const& c_idx, Index2 const& c_pos)
{
	float x, sy, z;
	float offset = 0.000001f;
	int cm_h = 0;
	for (int i = 0; i < 16; i++) {
		z = (c_pos.y - i + offset) / 32.f;
		for (int j = 0; j < 16; j++) {
			x = (c_pos.x + j + offset) / 32.f;
			int h = this->m_info->findHeight(c_idx, j, i);
			for (int y = 1; y < h; y++) {
				sy = y / 16.f;
				double h_ = this->p_noise->getNoise3D(x, sy, z, 3, 0.5);
				if (h_ <= 0.1)
					continue;
				if (this->m_info->findBlock(c_idx, j, y, i) == BlockType::WATER)
					continue;
				bool flag = false;
				for (int dir = 0; dir < 6; dir++) {
					if (this->getAdjInfo(c_idx, j, y, i, dir) == BlockType::WATER) {
						flag = true;
						break;
					}
				}
				if (flag)
					continue;
				this->m_info->addBlock(c_idx, j, y, i, 0);
			}
			
			// 높이 갱신
			int m_h = 0;
			for (int y = h - 1; y > 0; y--) {
				if (this->m_info->findBlock(c_idx, j, y, i)) {
					this->m_info->setHeight(c_idx, j, i, y + 1);
					m_h = y + 1;
					break;
				}
			}
			cm_h = max(cm_h, m_h);
		}
	}
	this->m_info->chunks[c_idx.y][c_idx.x]->max_h = cm_h;
}

int SetCave::getAdjInfo(Index2 const& c_idx, int x, int y, int z, int dir)
{
	Index2 adj_idx;
	Index2 pos = this->m_info->chunks[c_idx.y][c_idx.x]->chunk_pos;
	if (dir == 0) {
		if (x - 1 < 0) {
			pos.x -= 16;
			adj_idx = this->m_info->findChunkIndex(pos.x, pos.y);
			if (adj_idx.flag == false)
				return 0;
		}
		else
			x -= 1;
	}
	else if (dir == 1) {
		if (x + 1 > 15) {
			pos.x += 16;
			adj_idx = this->m_info->findChunkIndex(pos.x, pos.y);
			if (adj_idx.flag == false)
				return 0;
		}
		else
			x += 1;
	}
	else if (dir == 2) {
		if (z - 1 < 0) {
			pos.y += 16;
			adj_idx = this->m_info->findChunkIndex(pos.x, pos.y);
			if (adj_idx.flag == false)
				return 0;
		}
		else
			z -= 1;
	}
	else if (dir == 3) {
		if (z + 1 > 15) {
			pos.y -= 16;
			adj_idx = this->m_info->findChunkIndex(pos.x, pos.y);
			if (adj_idx.flag == false)
				return 0;
		}
		else
			z += 1;
	}
	else if (dir == 4) {
		if (y - 1 < 0)
			return 0;
		y -= 1;
	}
	else {
		if (y + 1 > 255)
			return 0;
		y += 1;
	}
	int res = this->m_info->findBlock(c_idx, x, y, z);
	return res;
}
