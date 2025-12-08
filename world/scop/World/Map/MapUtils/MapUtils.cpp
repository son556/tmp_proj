#include "pch.h"
#include "MapUtils.h"
#include "json.hpp"
#include <fstream>

#define STDZERO 0.00000001

using json = nlohmann::json;

MapUtils::MapUtils(
	int size_w, int size_h, HWND hwnd,
	UINT w_width, UINT w_height
)
{
	this->size_h = size_h;
	this->size_w = size_w;
	this->width = w_width;
	this->height = w_height;
	this->hWnd = hwnd;
	this->s_pos.x = -8 * this->size_w;
	this->s_pos.y = 8 * this->size_h;
	this->sv_pos.x = this->s_pos.x + 16;
	this->sv_pos.y = this->s_pos.y - 16;
	this->ev_pos.x = -this->sv_pos.x;
	this->ev_pos.y = -this->sv_pos.y;

	this->blocks = new int[16 * 16 * size_h * size_w * 256];
	this->light_map = new uint8[16 * 16 * size_h * size_w * 256];
	this->h_map = new int[16 * 16 * size_h * size_w];
	fill(this->blocks, this->blocks + 16 * 16 * size_h * size_w * 256, 0);
	fill(this->light_map, this->light_map + 16 * 16 * size_h * size_w * 256, 0);
	fill(this->h_map, this->h_map + 16 * 16 * size_h * size_w, 0);
}

MapUtils::~MapUtils()
{
	delete[] this->blocks;
	delete[] this->h_map;
	delete[] this->light_map;
}


WorldIndex MapUtils::pickBlock(vec3 r_pos, vec3 r_dir)
{
	// 1. x y z index 찾기
	WorldIndex ans;
	int x, y, z;
	vec3 tmp = r_pos;
	if (r_pos.y >= 256) {
		if (r_dir.y >= 0)
			return ans;
		tmp = this->intersectionRayAndPlane(
			r_pos,
			r_dir,
			vec3(r_pos.x, 256, r_pos.z),
			vec3(0, 1, 0)
		);
	}
	else if (r_pos.y < 0) {
		if (r_dir.y <= 0)
			return ans;
		tmp = this->intersectionRayAndPlane(
			r_pos,
			r_dir,
			vec3(r_pos.x, 0, r_pos.z),
			vec3(0, 1, 0)
		);
	}
	x = static_cast<int>(floor(tmp.x));
	y = static_cast<int>(floor(tmp.y));
	z = static_cast<int>(floor(tmp.z));
	if (z < tmp.z)
		z += 1;
	int step_x = 1;
	int step_y = 1;
	int step_z = 1;
	if (r_dir.x < 0)
		step_x = -1;
	if (r_dir.y < 0)
		step_y = -1;
	if (r_dir.z < 0)
		step_z = -1;

	// 2 max x y z, delta x y z 구하기
	float max_x, max_y, max_z;
	float delta_x = 0, delta_y = 0, delta_z = 0;
	bool flag_x = true, flag_y = true, flag_z = true;
	int sx, sy, sz;
	vec3 first, second;
	sx = x;
	if (r_dir.x > 0)
		sx += 1;
	max_x = r_dir.Dot(vec3(1, 0, 0));
	if (max_x <= STDZERO && max_x >= -STDZERO)
		flag_x = false;
	if (flag_x) {
		first = this->intersectionRayAndPlane(
			r_pos,
			r_dir,
			vec3(sx, 0, 0),
			vec3(1, 0, 0)
		);
		second = this->intersectionRayAndPlane(
			r_pos,
			r_dir,
			vec3(sx + step_x, 0, 0),
			vec3(1, 0, 0)
		);
		max_x = (r_pos - first).Length();
		delta_x = (first - second).Length();
	}
	sy = y;
	if (r_dir.y > 0)
		sy += 1;
	max_y = r_dir.Dot(vec3(0, 1, 0));
	if (max_y <= STDZERO && max_y >= -STDZERO)
		flag_y = false;
	if (flag_y) {
		first = this->intersectionRayAndPlane(
			r_pos,
			r_dir,
			vec3(0, sy, 0),
			vec3(0, 1, 0)
		);
		second = this->intersectionRayAndPlane(
			r_pos,
			r_dir,
			vec3(0, sy + step_y, 0),
			vec3(0, 1, 0)
		);
		max_y = (r_pos - first).Length();
		delta_y = (first - second).Length();
	}
	sz = z;
	if (r_dir.z < 0)
		sz -= 1;
	max_z = r_dir.Dot(vec3(0, 0, 1));
	if (max_z <= STDZERO && max_z >= -STDZERO)
		flag_z = false;
	if (flag_z) {
		first = this->intersectionRayAndPlane(
			r_pos,
			r_dir,
			vec3(0, 0, sz),
			vec3(0, 0, 1)
		);
		second = this->intersectionRayAndPlane(
			r_pos,
			r_dir,
			vec3(0, 0, sz + step_z),
			vec3(0, 0, 1)
		);
		max_z = (r_pos - first).Length();
		delta_z = (first - second).Length();
	}
	if (flag_x == false)
		max_x = INT_MAX;
	if (flag_y == false)
		max_y = INT_MAX;
	if (flag_z == false)
		max_z = INT_MAX;

	for (int i = 0; i < 12; i++) {
		if (max_x < max_y) {
			if (max_x < max_z) {
				x += step_x;
				max_x += delta_x;
				ans.dir = 2;
			}
			else {
				z += step_z;
				max_z += delta_z;
				ans.dir = 1;
			}
		}
		else {
			if (max_y < max_z) {
				y += step_y;
				max_y += delta_y;
				ans.dir = 0;
			}
			else {
				z += step_z;
				max_z += delta_z;
				ans.dir = 1;
			}
		}
		int8 dir = ans.dir;
		ans = this->getBlockIndex(x, y, z);
		if (ans.flag) {
			int b = this->findBlock(ans.c_idx, ans.b_idx);
			if (b && b != BlockType::WATER) {
				ans.pos = vec3(x, y, z); // block position
				ans.dir = dir;
				return ans;
			}
		}
	}
	ans.flag = false;
	return ans;
}


void MapUtils::writeBookAboutBlockStatus(
	Index2 const& chunk_pos, 
	Index3 const& block_idx, 
	BlockType original, 
	BlockType new_block
)
{
	auto it = this->book.find(chunk_pos);
	if (it == this->book.end()) {
		this->book.insert({ chunk_pos, map<Index3, Index2>()});
		this->book[chunk_pos].insert({ block_idx, Index2(new_block, original) });
	}
	else {
		auto v_it = it->second.find(block_idx);
		if (v_it == it->second.end())
			it->second.insert({ block_idx, Index2(new_block, original) });
		else {
			if (v_it->second.y == static_cast<int>(new_block)) {
				it->second.erase(block_idx);
				if (it->second.size() == 0)
					this->book.erase(it->first);
			}
			else {// original 은 처음 청크가 생겼을 때 유지
				v_it->second = Index2(new_block, v_it->second.y);
			}
		}
	}
}

map<Index3, Index2>const* const MapUtils::getUserPlacedBlocks(
	Index2 const& chunk_pos
)
{
	auto it = this->book.find(chunk_pos);
	if (it == this->book.end())
		return nullptr;
	return &(it->second);
}


void to_json(json& j, const Index2& idx) {
	j = json{ {"x", idx.x}, {"y", idx.y}, {"flag", idx.flag} };
}

void to_json(json& j, const Index3& idx) {
	j = json{ {"x", idx.x}, {"y", idx.y}, {"z", idx.z}, {"flag", idx.flag} };
}

void from_json(const json& j, Index2& idx) {
	j.at("x").get_to(idx.x);
	j.at("y").get_to(idx.y);
	j.at("flag").get_to(idx.flag);
}

void from_json(const json& j, Index3& idx) {
	j.at("x").get_to(idx.x);
	j.at("y").get_to(idx.y);
	j.at("z").get_to(idx.z);
	j.at("flag").get_to(idx.flag);
}

void MapUtils::saveGame()
{
	map<Index2, map<Index3, Index2>>::iterator it;
	map<Index3, Index2>::iterator iit;
	json j_arr = json::array();
	for (it = this->book.begin(); it != this->book.end(); it++) {
		json out;
		out["OuterKey"] = it->first;
		json j_in_arr = json::array();
		for (iit = it->second.begin(); iit != it->second.end(); iit++) {
			json entry;
			entry["InnerKey"] = iit->first;
			entry["Value"] = iit->second;
			j_in_arr.push_back(entry);
		}
		out["Map"] = j_in_arr;
		j_arr.push_back(out);
	}

	ofstream o("book.json");
	if (o.is_open()) {
		o << j_arr.dump(4);
		o.close();
		cout << "게임이 저장되었습니다." << endl;
	}
	else {
		o.close();
		cout << "게임 저장 실패!!!" << endl;
	}
}

void MapUtils::loadGame()
{
	ifstream i("book.json");
	if (i.is_open() == false) {
		cout << "파일을 불러오는데 실패했습니다." << endl;
		return;
	}

	i.seekg(0, std::ios::end);
	if (i.tellg() == 0)
		return;
	i.seekg(0, std::ios::beg);

	json j;
	i >> j;
	i.close();

	for (auto const& entry : j) {
		Index2 chunk_pos = entry.at("OuterKey").get<Index2>();
		map<Index3, Index2> inner_map;
		for (auto const& inner_entry : entry.at("Map")) {
			Index3 block_idx = inner_entry.at("InnerKey").get<Index3>();
			Index2 block_type = inner_entry.at("Value").get<Index2>();
			inner_map.insert({ block_idx, block_type });
		}
		this->book.insert({ chunk_pos, inner_map });
	}
}


