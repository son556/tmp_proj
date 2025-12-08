#include "pch.h"
#include "SetWater.h"
#include "MapUtils.h"

SetWater::SetWater(MapUtils* m_info) : m_info(m_info)
{
}

void SetWater::fillWater(Index2 const& c_idx)
{
	for (int y = 0; y < WATER_HEIGHT; y++) {
		for (int z = 0; z < 16; z++) {
			for (int x = 0; x < 16; x++) {
				if (this->m_info->findBlock(c_idx, x, y, z))
					continue;
				this->m_info->addBlock(c_idx, x, y, z, BlockType::WATER);
			}
		}
	}
}
