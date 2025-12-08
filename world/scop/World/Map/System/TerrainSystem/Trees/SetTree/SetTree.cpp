#include "pch.h"
#include "SetTree.h"
#include "MapUtils.h"
#include "Chunk.h"
#include "PerlinNoise.h"
#include "Trees.h"

SetTree::SetTree(MapUtils* minfo, PerlinNoise* noise)
	: m_info(minfo), p_noise(noise)
{
}

void SetTree::makeTree(Index2 const& c_idx)
{
	Index2 cpos = this->m_info->chunks[c_idx.y][c_idx.x]->chunk_pos;
	bool continue_flag = false;
	double offset = 0;
	for (int z = 0; z < 16; z++) {
		double d_z = (cpos.y - z + offset) / 32.f;
		for (int x = 0; x < 16; x++) {
			double d_x = (cpos.x + x + offset) / 32.f;
			int h = this->m_info->findHeight(c_idx, x, z);

			// 높이가 낮거나 바닥이 없는 경우
			int block_type = this->m_info->findBlock(c_idx, x, h - 1, z);
			if (h < 20 || block_type == 0 || 
				this->m_info->findLight(c_idx, x, h, z) < 15)
				continue;

			if (block_type == BlockType::OAK_LEAVES ||
				block_type == BlockType::OAK_LOG)
				continue;


			// 펄린 노이즈
			double p = this->p_noise->getNoise2D(d_x, d_z, 3, 0.5);
			if (p < 0.2)
				continue;
			
			// 놓을 수 있는 충분한 높이가 안되는 경우 (이미 다른 나무가 있음)
			// 기둥은 다른 나무 밑에 못 놓게 하기 위한 조건
			for (int y = h; y < h + 7; y++) {
				if (this->m_info->findBlock(c_idx, x, y, z)) {
					continue_flag = true;
					break;
				}
			}
			if (continue_flag)
				continue;
			this->putTree(c_idx, x, h, z);
			return;
		}
	}
}

void SetTree::putTree(Index2 const& c_idx, int x, int y, int z)
{
	int ix = -1;
	int iy = -1;
	int iz = -1;
	Index2 here_cidx;
	Index3 here_bidx;
	Index2 pos = this->m_info->chunks[c_idx.y][c_idx.x]->chunk_pos;

	for (int sy = y; sy < y + 7; sy++) {
		iy++;
		iz = -1;
		for (int sz = z - 2; sz <= z + 2; sz++) {
			iz++;
			ix = -1;
			for (int sx = x - 2; sx <= x + 2; sx++) {
				ix++;
				int tr = Trees::getOakTreeInfo(ix, iy, iz);
				if (tr) {
					here_bidx = Index3(sx, sy, sz);
					this->checkChunk(c_idx, sx, sy, sz,
						here_cidx, here_bidx);
					if (here_cidx.flag == false)
						continue;
					int b_type =
						this->m_info->findBlock(here_cidx, here_bidx);
					if (b_type && b_type != BlockType::OAK_LEAVES)
						continue;
					if (tr == 2 && b_type == BlockType::OAK_LEAVES)
						continue;
					if (tr == 1)
						this->m_info->addBlock(here_cidx, here_bidx,
							BlockType::OAK_LOG);
					if (tr == 2)
						this->m_info->addBlock(here_cidx, here_bidx,
							BlockType::OAK_LEAVES);
					this->resetChunkHeight(here_cidx, here_bidx.x,
						here_bidx.z, here_bidx.y + 1);
				}
			}
		}
	}
}

void SetTree::createTrees()
{
	for (int i = 1; i < this->m_info->size_h - 1; i++) {
		for (int j = 1; j < this->m_info->size_w - 1; j++) {
			Index2 c_pos = this->m_info->s_pos + Index2(j * 16, -i * 16);
			Index2 c_idx;
			c_idx = this->m_info->getChunkIndex(c_pos.x, c_pos.y);
			this->makeTree(c_idx);
		}
	}
}

void SetTree::createTrees(vector<Index2>& chunks, int dir)
{
	vector<Index2> p_chunks;
	set<Index2> check_list;
	p_chunks.resize(chunks.size());
	Index2 cpos;
	int cnt = 0;
	for (Index2& cidx : chunks) {
		check_list.insert(cidx);
		// 시야가 청크를 넘었을 때 새로 만들어야 하는 청크만 나무를 심기 위해
		if ((dir == 1 || dir == 2) && cnt < this->m_info->size_h - 1) {
			this->makeTree(cidx);
			cnt++;
			continue;
		}
		if ((dir == 4 || dir == 8) && cnt < this->m_info->size_w - 1) {
			this->makeTree(cidx);
			cnt++;
			continue;
		}
	}

	// 나무가 추가되었기 때문에 인접한 이전 청크의 정점을 갱신해주기 위해.
	if (dir == 1) {
		for (int i = 0; i < chunks.size(); i++) {
			cpos = this->m_info->chunks[chunks[i].y][chunks[i].x]->chunk_pos;
			cpos = cpos + Index2(16, 0);
			p_chunks[i] = this->m_info->findChunkIndex(cpos.x, cpos.y);
		}
	}
	else if (dir == 2) {
		for (int i = 0; i < chunks.size(); i++) {
			cpos = this->m_info->chunks[chunks[i].y][chunks[i].x]->chunk_pos;
			cpos = cpos + Index2(-16, 0);
			p_chunks[i] = this->m_info->findChunkIndex(cpos.x, cpos.y);
		}
	}
	else if (dir == 4) {
		for (int i = 0; i < chunks.size(); i++) {
			cpos = this->m_info->chunks[chunks[i].y][chunks[i].x]->chunk_pos;
			cpos = cpos + Index2(0, -16);
			p_chunks[i] = this->m_info->findChunkIndex(cpos.x, cpos.y);
		}
	}
	else {
		for (int i = 0; i < chunks.size(); i++) {
			cpos = this->m_info->chunks[chunks[i].y][chunks[i].x]->chunk_pos;
			cpos = cpos + Index2(0, 16);
			p_chunks[i] = this->m_info->findChunkIndex(cpos.x, cpos.y);
		}
	}
	for (int i = 0; i < p_chunks.size(); i++) {
		if (check_list.find(p_chunks[i]) != check_list.end())
			continue;
		chunks.push_back(p_chunks[i]);
	}
}

void SetTree::checkChunk(Index2 const& c_idx, int x, int y, int z, 
	Index2& res_cidx, Index3& res_bidx)
{
	Index2 pos = this->m_info->chunks[c_idx.y][c_idx.x]->chunk_pos;
	Index2 mx_cidx = this->m_info->findChunkIndex(pos.x - 16, pos.y);
	Index2 px_cidx = this->m_info->findChunkIndex(pos.x + 16, pos.y);
	Index2 mz_cidx = this->m_info->findChunkIndex(pos.x, pos.y + 16);
	Index2 pz_cidx = this->m_info->findChunkIndex(pos.x, pos.y - 16);
	Index2 mxmz_cidx = this->m_info->findChunkIndex(pos.x - 16, pos.y + 16);
	Index2 mxpz_cidx = this->m_info->findChunkIndex(pos.x - 16, pos.y - 16);
	Index2 pxmz_cidx = this->m_info->findChunkIndex(pos.x + 16, pos.y + 16);
	Index2 pxpz_cidx = this->m_info->findChunkIndex(pos.x + 16, pos.y - 16);

	int here_x = x;
	int here_z = z;
	if (x < 0)
		here_x = 16 + x;
	else if (x > 15)
		here_x = x - 16;
	if (z < 0)
		here_z = 16 + z;
	else if (z > 15)
		here_z = z - 16;

	res_cidx = c_idx;
	if (x < 0 && z < 0)
		res_cidx = mxmz_cidx;
	else if (x < 0 && z > 15)
		res_cidx = mxpz_cidx;
	else if (x > 15 && z < 0)
		res_cidx = pxmz_cidx;
	else if (x > 15 && z > 15)
		res_cidx = pxpz_cidx;
	else if (x < 0)
		res_cidx = mx_cidx;
	else if (x > 15)
		res_cidx = px_cidx;
	else if (z < 0)
		res_cidx = mz_cidx;
	else if (z > 15)
		res_cidx = pz_cidx;
	else
		res_cidx = c_idx;
	res_bidx = Index3(here_x, y, here_z);
}

void SetTree::resetChunkHeight(Index2 const& c_idx, int x, int z, int h)
{
	this->m_info->setHeight(c_idx, x, z, h);
	int16& max_h = this->m_info->chunks[c_idx.y][c_idx.x]->max_h;
	max_h = max(max_h, static_cast<int16>(h));
}
