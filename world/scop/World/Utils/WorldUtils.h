#pragma once

#include <set>

enum class Face {
	Top,
	Bottom,
	Front,
	Back,
	Left,
	Right
};

struct Index3 {
	int x;
	int y;
	int z;
	bool flag;

	Index3() : x(0), y(0), z(0), flag(false) {}
	Index3(int x, int y, int z) : x(x), y(y), z(z), flag(true) {}
	Index3(Index3 const& idx3) { *this = idx3; }
	Index3& operator=(Index3 const& idx3) {
		this->x = idx3.x;
		this->y = idx3.y;
		this->z = idx3.z;
		this->flag = idx3.flag;
		return *this;
	}

	Index3& operator+=(Index3 const& idx3) {
		this->x += idx3.x;
		this->y += idx3.y;
		this->z += idx3.z;
		this->flag |= idx3.flag;
		return *this;
	}

	bool operator<(Index3 const& idx3) const {
		if (this->x == idx3.x) {
			if (this->y == idx3.y)
				return this->z < idx3.z;
			return this->y < idx3.y;
		}
		return this->x < idx3.x;
	}

	bool operator==(Index3 const& idx3) const {
		return this->x == idx3.x && this->y == idx3.y &&
			this->z == idx3.z;
	}
};

inline Index3 operator+(Index3 const& idx1, Index3 const& idx2) {
	Index3 res(idx1.x + idx2.x, idx1.y + idx2.y, idx1.z + idx2.z);
	return res;
}

inline Index3 operator-(Index3 const& idx1, Index3 const& idx2) {
	Index3 res(idx1.x - idx2.x, idx1.y - idx2.y, idx1.z - idx2.z);
	return res;
}

struct Index2 {
	int x;
	int y;
	bool flag;

	Index2() : x(0), y(0), flag(false) {}
	Index2(int x, int y) : x(x), y(y), flag(true) {}
	Index2(Index2 const& idx2) { *this = idx2; }

	Index2& operator=(Index2 const& idx2) {
		this->x = idx2.x;
		this->y = idx2.y;
		this->flag = idx2.flag;
		return *this;
	}

	Index2& operator+=(Index2 const& idx2) {
		this->x += idx2.x;
		this->y += idx2.y;
		this->flag |= idx2.flag;
		return *this;
	}

	bool operator<(Index2 const& idx2) const {
		if (this->x == idx2.x) {
			return this->y < idx2.y;
		}
		return this->x < idx2.x;
	}

	bool operator==(Index2 const& idx2) const {
		return this->x == idx2.x && this->y == idx2.y;
	}
};

inline Index2 operator+(Index2 const& idx1, Index2 const& idx2) {
	Index2 res(idx1.x + idx2.x, idx1.y + idx2.y);
	return res;
}

inline Index2 operator-(Index2 const& idx1, Index2 const& idx2) {
	Index2 res(idx1.x - idx2.x, idx1.y - idx2.y);
	return res;
}

struct Index3Hash {
	size_t operator() (Index3 const& idx3) const {
		return hash<int>()(idx3.x) ^ hash<int>()(idx3.y) ^ hash<int>()(idx3.z);
	}
};

struct Index2Hash {
	size_t operator() (Index2 const& idx2) const {
		return hash<int>()(idx2.x) ^ hash<int>()(idx2.y);
	}
};

inline void setIndex3(Index3& idx3, int x, int y, int z) {
	idx3.x = x;
	idx3.y = y;
	idx3.z = z;
}

inline void setIndex2(Index2& idx2, int x, int y) {
	idx2.x = x;
	idx2.y = y;
}

// c_idx : chunk index, b_idx : block index
struct WorldIndex {
	Index2 c_idx;
	Index3 b_idx;
	bool flag;
	vec3 pos;
	int8 dir;
	int block_type;
	WorldIndex() 
		: c_idx(0, 0), b_idx(0, 0, 0), 
		flag(false), pos(0, 0, 0), dir(0), block_type(0) {}
	WorldIndex(Index2 const& cidx, Index3 const& bidx)
		: c_idx(cidx), b_idx(bidx), flag(true), 
		pos(0, 0,0), dir(0), block_type(0) {}
	WorldIndex(WorldIndex const& world) 
		: c_idx(world.c_idx), b_idx(world.b_idx), flag(world.flag), pos(world.pos), 
		dir(world.dir), block_type(world.block_type) {}
	WorldIndex& operator=(WorldIndex const& world) {
		this->c_idx = world.c_idx;
		this->b_idx = world.b_idx;
		this->flag = world.flag;
		this->dir = world.dir;
		this->pos = world.pos;
		this->block_type = world.block_type;
		return *this;
	}
};

struct VertexGeo {
	int tex_arr_idx;
	vec3 pos;
	vec3 normal;
	vec3 tangent;
	vec2 uv;
};

struct VertexShadow {
	vec3 pos;
	vec3 normal;
	int shadow_flag = 0;
};

struct VertexDefer {
	vec3 pos;
	vec2 uv;
};

struct VertexColor {
	vec3 pos;
	color col;
};

struct VertexWater {
	vec3 pos;
	vec4 col;
	vec2 uv;
};

struct CamPos {
	vec4 view_pos;
	vec4 light_pos;
	Mat view;
};

struct VertexPTN {
	vec3 pos;
	vec2 uv;
	vec3 normal;
};
