#include "pch.h"
#include "LightSystem.h"
#include "Chunk.h"

#include <mutex>

static mutex test_mutex;

const vector<Index3> move_dir = {
		{0, 1, 0},
		{0, -1, 0},
		{1, 0, 0},
		{-1, 0, 0},
		{0, 0, 1},
		{0, 0, -1}
};

LightSystem::LightSystem(MapUtils* minfo) : m_info(minfo)
{
}

void LightSystem::SetLightChunk(const Index2 chunkIndex, const int threadID)
{
	queue<tuple<Index2, Index3, uint8>> que;
	this->setSunLight(chunkIndex, que);
	this->lightBFS(que, threadID, chunkIndex);
}

void LightSystem::SetLightAdjChunkInMain()
{
	for (auto adjchunks : this->m_info->GetAdjChunkLightList())
	{
		for (auto IndexAndLight : adjchunks)
		{
			if (IndexAndLight.y > this->m_info->findLight(IndexAndLight.x))
				this->m_info->setLight(IndexAndLight.x, IndexAndLight.y);
		}
	}
	this->m_info->ResetAdjChunkLightList();
}

/*
* 1. 빛 전파 -> (전파된 빛을 adj 큐에 넣음(light map 에 적지 않음), 일이 끝나면 chunk_cnt--; atomic<int> chunk_cnt
* 2. 매니저 스레드 -> (adj 큐에 들어온 데이터를 확인 후 light_map에 옮겨 적음, chunk_cnt 가 0 이고 adj 큐의 크기가 0일 때 탈출)
*/

// TODO 내 청크만 bfs 계산하고 다른 청크 넘어가는 경우는 따로 모아서 처리하는 방법?
void LightSystem::lightBFS(queue<tuple<Index2, Index3, uint8>>& que, const int threadID, const Index2 nowChunkIndex)
{ // 단점: 매 단계 Atomic 연산으로 인한 캐시 라인 경합(False Sharing) 및 무효화 비용이 매우 높음.
	Index2 s_cidx;
	Index3 s_bidx;
	Index2 n_cidx;
	Index3 n_bidx;
	uint8 light;
	uint8 n_light;
	int block_type;

	while (que.size())
	{
		s_cidx = get<0>(que.front());
		s_bidx = get<1>(que.front());
		light = get<2>(que.front());
		que.pop();

		for (int i = 0; i < 6; i++)
		{
			n_cidx = s_cidx;
			n_bidx = s_bidx + move_dir[i];
			this->getIndex(n_cidx, n_bidx);
			if (n_cidx.flag == false)
				continue;
			if (n_bidx.y > this->m_info->chunks[n_cidx.y][n_cidx.x]->max_h + 8)
				continue;
			block_type = this->m_info->findBlock(n_cidx, n_bidx);
			if (block_type > 0 && block_type != BlockType::OAK_LEAVES)
				continue;
			
			int idx = 16 * 16 * 256 * (n_cidx.x + this->m_info->size_w * n_cidx.y) + n_bidx.x + 16 * (n_bidx.z + 16 * n_bidx.y);
			n_light = this->m_info->findLight(idx);
			if (light > n_light + 1)
			{
				if (AtomicMax(this->m_info->getLight(idx), light - 1))
					que.push({ n_cidx, n_bidx, light - 1 });
				
				//{ // test mutex
				//	que.push({ n_cidx, n_bidx, light - 1 });
				//	lock_guard legion_lock(test_mutex);
				//	this->m_info->setLight(n_cidx, n_bidx, light - 1);
				//}
			}
		}
	}
}

bool LightSystem::AtomicMax(uint8& target, uint8 newValue)
{
	std::atomic_ref<uint8> atomicTarget(target);
	uint8 prevValue = atomicTarget.load(std::memory_order_relaxed);

	while (prevValue < newValue)
	{
		// CAS 시도: 성공하면 내가 이 구역의 빛을 결정한 것임
		if (atomicTarget.compare_exchange_weak(prevValue, newValue, std::memory_order_relaxed))
			return true;
	}
	return false;
}

void LightSystem::resetLight(Index2 const& c_idx)
{
	this->m_info->ResetChunkLight(c_idx);
}

void LightSystem::setSunLight(Index2 const& c_idx, queue<tuple<Index2, Index3, uint8>>& que)
{
	for (int z = 0; z < 16; z++) {
		for (int x = 0; x < 16; x++) {
			for (int y = this->m_info->chunks[c_idx.y][c_idx.x]->max_h + 10; y > 0; y--) {
				int block_type = this->m_info->findBlock(c_idx, x, y, z);
				if (block_type > 0 && block_type != BlockType::OAK_LEAVES) {
					break;
				}
				this->m_info->setLight(c_idx, x, y, z, 15);
				que.push(make_tuple(c_idx, Index3(x, y, z), 15));
			}
		}
	}
}

// light one block
void LightSystem::BFSLightBlockDelete(
	Index2 const& c_idx,
	Index3 const& b_idx,
	vector<Index2>& cidx_vec
)
{
	queue<pair<Index2, Index3>> que;
	Index2 s_cidx = c_idx;
	Index3 s_bidx = b_idx;
	Index2 n_cidx;
	Index3 n_bidx;
	s_cidx.flag = true;
	uint8 light;
	uint8 n_light;
	set<Index2> book;
	
	book.insert(c_idx);
	que.push({ s_cidx, b_idx });
	// 지금 블록 light 갱신
	light = this->m_info->findLight(s_cidx, s_bidx);
	for (int i = 0; i < 6; i++) {
		n_cidx = s_cidx;
		n_bidx = s_bidx + move_dir[i];
		this->getIndex(n_cidx, n_bidx);
		if (n_cidx.flag == false)
			continue;
		n_light = this->m_info->findLight(n_cidx, n_bidx);
		if (n_light == 15 && move_dir[i].y == 1) // 태양에서 수직으로 빛이 오는 경우
			light = max(light, n_light);
		else
			light = max(light, n_light - 1);
	}
	this->m_info->setLight(s_cidx, s_bidx, light);

	// bfs
	int b_type;
	bool exit_flag = false;
	while (que.size()) 
	{
		s_cidx = que.front().first;
		s_bidx = que.front().second;
		light = this->m_info->findLight(s_cidx, s_bidx);
		que.pop();
		book.insert(s_cidx);

		for (int i = 0; i < 6; i++) 
		{
			n_cidx = s_cidx;
			n_bidx = s_bidx + move_dir[i];
			this->getIndex(n_cidx, n_bidx);
			if (n_cidx.flag == false)
				continue;
			if (n_bidx.y > this->m_info->chunks[n_cidx.y][n_cidx.x]->max_h)
				continue;
			b_type = this->m_info->findBlock(n_cidx, n_bidx);
			if (b_type > 0 && b_type != BlockType::OAK_LEAVES)
				continue;

			n_light = this->m_info->findLight(n_cidx, n_bidx);
			if (light == 15 && move_dir[i].y == -1) {
				this->m_info->setLight(n_cidx, n_bidx, 15);
				que.push({ n_cidx, n_bidx});
				continue;
			}
			if (n_light + 1 >= light)
				continue;
			
			this->m_info->setLight(n_cidx, n_bidx, light - 1);
			que.push({ n_cidx, n_bidx });
		}
	}
	for (auto it : book) {
		if (it == c_idx)
			continue;
		cidx_vec.push_back(it);
	}
}

void LightSystem::BFSLightBlockAdd(
	Index2 const& c_idx, 
	Index3 const& b_idx,
	vector<Index2>& cidx_vec
)
{
	queue<pair<Index2, Index3>> que;
	stack<pair<Index2, Index3>> l_stack;
	Index2 s_cidx = c_idx;
	Index3 s_bidx = b_idx;
	Index2 n_cidx;
	Index3 n_bidx;

	s_cidx.flag = true;
	uint8 light;
	uint8 n_light;
	set<Index2> book;
	int b_type;

	int h = this->m_info->findHeight(c_idx, b_idx.x, b_idx.z);
	// 최고 높이가 갱신된 경우
	if (h == this->m_info->chunks[c_idx.y][c_idx.x]->max_h) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				for (int y = h + 8; y > -1; y--) {
					b_type = this->m_info->findBlock(c_idx, x, y, z);
					light = this->m_info->findLight(c_idx, x, y, z);
					if (b_type || light)
						break;
					this->m_info->setLight(c_idx, x, y, z, 15);
				}
			}
		}
	}

	// erase light value
	que.push({ s_cidx, s_bidx });
	while (que.size()) {
		s_cidx = que.front().first;
		s_bidx = que.front().second;
		light = this->m_info->findLight(s_cidx, s_bidx);
		book.insert(s_cidx);
		this->m_info->setLight(s_cidx, s_bidx, 0);
		que.pop();
		for (int i = 0; i < 6; i++) {
			n_cidx = s_cidx;
			n_bidx = s_bidx + move_dir[i];
			this->getIndex(n_cidx, n_bidx);
			if (n_cidx.flag == false)
				continue;
			if (n_bidx.y > this->m_info->chunks[n_cidx.y][n_cidx.x]->max_h)
				continue;
			b_type = this->m_info->findBlock(n_cidx, n_bidx);
			if (b_type && b_type != BlockType::OAK_LEAVES)
				continue;
			n_light = this->m_info->findLight(n_cidx, n_bidx);
			if (n_light + 1 == light || (light == 15 && move_dir[i].y == -1)) {
				que.push({ n_cidx, n_bidx });
				l_stack.push({ n_cidx, n_bidx });
			}
		}
	}
	
	// light injection
	while (l_stack.size()) {
		s_cidx = l_stack.top().first;
		s_bidx = l_stack.top().second;
		light = 0;
		l_stack.pop();
		for (int i = 0; i < 6; i++) {
			n_cidx = s_cidx;
			n_bidx = s_bidx + move_dir[i];
			this->getIndex(n_cidx, n_bidx);
			if (n_cidx.flag == false)
				continue;
			n_light = this->m_info->findLight(n_cidx, n_bidx);
			if (move_dir[i].y == 1 && n_light == 15)
				light = max(light, n_light);
			else if (n_light > 0)
				light = max(light, n_light - 1);
		}
		if (light == 0)
			continue;
		this->m_info->setLight(s_cidx, s_bidx, light);
		que.push({ s_cidx, s_bidx });
	}

	// light propagation
	while (que.size()) {
		s_cidx = que.front().first;
		s_bidx = que.front().second;
		light = this->m_info->findLight(s_cidx, s_bidx);
		que.pop();
		for (int i = 0; i < 6; i++) {
			n_cidx = s_cidx;
			n_bidx = s_bidx + move_dir[i];
			this->getIndex(n_cidx, n_bidx);
			if (n_cidx.flag == false)
				continue;
			b_type = this->m_info->findBlock(n_cidx, n_bidx);
			if (b_type > 0 && b_type != BlockType::OAK_LEAVES)
				continue;
			n_light = this->m_info->findLight(n_cidx, n_bidx);
			if (n_light + 1 < light) {
				this->m_info->setLight(n_cidx, n_bidx, light - 1);
				que.push({ n_cidx, n_bidx });
			}
		}
	}

	this->m_info->setLight(c_idx, b_idx, 0);
	for (auto it : book) {
		if (it == c_idx)
			continue;
		cidx_vec.push_back(it);
	}
}

void LightSystem::getIndex(Index2& c_idx, Index3& b_idx) const
{
	if (b_idx.x >= 0 && b_idx.x < 16 && b_idx.z >= 0 && b_idx.z < 16 
		&& b_idx.y > -1 && b_idx.y < 256) {
		c_idx.flag = true;
		return;
	}
	Index2 cpos = this->m_info->chunks[c_idx.y][c_idx.x]->chunk_pos;
	c_idx.flag = true;
	if (b_idx.x < 0) {
		c_idx = this->m_info->findChunkIndex(cpos.x - 1, cpos.y);
		b_idx.x = 16 + b_idx.x;
		return;
	}
	if (b_idx.x > 15) {
		c_idx = this->m_info->findChunkIndex(cpos.x + 17, cpos.y);
		b_idx.x -= 16;
		return;
	}
	if (b_idx.z < 0) {
		c_idx = this->m_info->findChunkIndex(cpos.x, cpos.y + 1);
		b_idx.z = 16 + b_idx.z;
		return;
	}
	if (b_idx.z > 15) {
		c_idx = this->m_info->findChunkIndex(cpos.x, cpos.y - 17);
		b_idx.z -= 16;
		return;
	}
	else {
		c_idx.flag = false;
		return;
	}
}
