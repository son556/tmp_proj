#pragma once

#include "WorldUtils.h"
/*
block idx
u : 윗면, d : 아랫면

 0 -- 1
 | u  |
 3 -- 2

 4 -- 5
 | d  |
 7 -- 6
*/

static const vector<vec3> box = {
	{-0.5, 0.5, 0.5},
	{0.5, 0.5, 0.5},
	{0.5, 0.5, -0.5},
	{-0.5, 0.5, -0.5},

	{-0.5, -0.5, 0.5},
	{0.5, -0.5, 0.5},
	{0.5, -0.5, -0.5},
	{-0.5, -0.5, -0.5},
};

static const vector<vec3> pos = {
	// top
	box[0], box[1], box[2], box[3],

	// bottom
	box[7], box[6], box[5], box[4],

	// right
	box[1], box[5], box[6], box[2],

	// left
	box[0], box[3], box[7], box[4],

	// back
	box[1], box[0], box[4], box[5],

	// front
	box[3], box[2], box[6], box[7]

};

static const vector<vec3> normal = {
	{0, 1, 0}, // 윗면
	{0, -1, 0}, // 아랫면
	{1, 0, 0}, // 오른쪽면
	{-1, 0, 0}, // 왼쪽면
	{0, 0, 1}, // 뒷면
	{0, 0, -1} // 앞면
};

static const float d = 1.f / 8.f;
static const float h_d = 1.f / 16.f;

static const vector<vec2> human_head_uv = { // 시계방향 정렬
	// 위 0 1 2 3
	{d, 0},
	{d * 2, 0},
	{d * 2, d},
	{d, d},

	// 아래 7 6 5 4
	{2 * d, 0},
	{3 * d, 0},
	{3 * d, d},
	{2 * d, d},

	// 오른쪽 1 5 6 2
	{3 * d, d},
	{3 * d, 2 * d},
	{2 * d, 2 * d},
	{2 * d, d},

	// 왼 0 3 7 4
	{0, d},
	{d, d},
	{d, 2 * d},
	{0, 2 * d},

	// 뒤 1 0 4 5
	{3 * d, d},
	{4 * d, d},
	{4 * d, 2 * d},
	{3 * d, 2 * d},

	// 앞 3 2 6 7
	{d, d},
	{2 * d, d},
	{2 * d, 2 * d},
	{d, 2 * d}
};
static const vector<vec2> human_body_uv = {
	// 위 
	{2 * d + h_d, d * 2},
	{3 * d + h_d, d * 2},
	{3 * d + h_d, 2 * d + h_d},
	{2 * d + h_d, 2 * d + h_d},

	// 아래
	{3 * d + h_d, 2 * d},
	{4 * d + h_d, 2 * d},
	{4 * d + h_d, 2 * d + h_d},
	{3 * d + h_d, 2 * d + h_d},

	// 오른
	{4 * d, 2 * d + h_d},
	{4 * d, 4 * d},
	{3 * d + h_d, 4 * d},
	{3 * d + h_d, 2 * d + h_d},

	// 왼
	{2 * d, 2 * d + h_d},
	{2 * d + h_d, 2 * d + h_d},
	{2 * d + h_d, 4 * d},
	{2 * d, 4 * d},

	// 뒤
	{4 * d, 2 * d + h_d},
	{5 * d, 2 * d + h_d},
	{5 * d, 4 * d},
	{4 * d, 4 * d},

	// 앞
	{2 * d + h_d, 2 * d + h_d},
	{3 * d + h_d, 2 * d + h_d},
	{3 * d + h_d, 4 * d},
	{2 * d + h_d, 4 * d}
};
static const vector<vec2> human_left_arm_uv = {
	// 위
	{5 * d + h_d, 2 * d},
	{6 * d, 2 * d},
	{6 * d, 2 * d + h_d},
	{5 * d + h_d, 2 * d + h_d},

	// 아래
	{6 * d, 2 * d},
	{6 * d + h_d, 2 * d},
	{6 * d + h_d, 2 * d + h_d},
	{6 * d, 2 * d + h_d},

	// 오른
	{6 * d + h_d, 2 * d + h_d},
	{6 * d + h_d, 4 * d},
	{6 * d, 4 * d},
	{6 * d, 2 * d + h_d},

	// 왼
	{5 * d, 2 * d + h_d},
	{5 * d + h_d, 2 * d + h_d},
	{5 * d + h_d, 4 * d},
	{5 * d, 4 * d},

	// 뒤
	{6 * d + h_d, 2 * d + h_d},
	{7 * d, 2 * d + h_d},
	{7 * d, 4 * d},
	{6 * d + h_d, 4 * d},

	// 앞
	{5 * d + h_d, 2 * d + h_d},
	{6 * d, 2 * d + h_d},
	{6 * d, 4 * d},
	{5 * d + h_d, 4 * d}
};
static const vector<vec2> human_right_arm_uv = {
	// 위
	{5 * d + h_d, 2 * d},
	{6 * d, 2 * d},
	{6 * d, 2 * d + h_d},
	{5 * d + h_d, 2 * d + h_d},

	// 아래
	{6 * d, 2 * d},
	{6 * d + h_d, 2 * d},
	{6 * d + h_d, 2 * d + h_d},
	{6 * d, 2 * d + h_d},

	// 오른
	{5 * d, 2 * d + h_d},
	{5 * d, 4 * d},
	{5 * d + h_d, 4 * d},
	{5 * d + h_d, 2 * d + h_d},

	// 왼
	{6 * d, 2 * d + h_d},
	{6 * d + h_d, 2 * d + h_d},
	{6 * d + h_d, 4 * d},
	{6 * d, 4 * d},

	// 뒤
	{6 * d + h_d, 2 * d + h_d},
	{7 * d, 2 * d + h_d},
	{7 * d, 4 * d},
	{6 * d + h_d, 4 * d},

	// 앞
	{5 * d + h_d, 2 * d + h_d},
	{6 * d, 2 * d + h_d},
	{6 * d, 4 * d},
	{5 * d + h_d, 4 * d}
};
static const vector<vec2> human_left_leg_uv = {
	// 위
	{h_d, 2 * d},
	{d, 2 * d},
	{d, 2 * d + h_d},
	{h_d, 2 * d + h_d},

	// 아래
	{d, 2 * d},
	{d + h_d, 2 * d},
	{d + h_d, 2 * d + h_d},
	{d, 2 * d + h_d},

	// 오른
	{d + h_d, 2 * d + h_d},
	{d + h_d, 4 * d},
	{d, 4 * d},
	{d, 2 * d + h_d},

	// 왼
	{0, 2 * d + h_d},
	{h_d, 2 * d + h_d},
	{h_d, 4 * d},
	{0, 4 * d},

	// 뒤
	{d + h_d, 2 * d + h_d},
	{2 * d, 2 * d + h_d},
	{2 * d, 4 * d},
	{d + h_d, 4 * d},

	// 앞
	{h_d, 2 * d + h_d},
	{d, 2 * d + h_d},
	{d, 4 * d},
	{h_d, 4 * d}
};
static const vector<vec2> human_righ_leg_uv = {
	// 위
	{h_d, 2 * d},
	{d, 2 * d},
	{d, 2 * d + h_d},
	{h_d, 2 * d + h_d},

	// 아래
	{d, 2 * d},
	{d + h_d, 2 * d},
	{d + h_d, 2 * d + h_d},
	{d, 2 * d + h_d},

	// 오른
	{0, 2 * d + h_d},
	{0, 4 * d},
	{h_d, 4 * d},
	{h_d, 2 * d + h_d},

	// 왼
	{d, 2 * d + h_d},
	{d + h_d, 2 * d + h_d},
	{d + h_d, 4 * d},
	{d, 4 * d},

	// 뒤
	{d + h_d, 2 * d + h_d},
	{2 * d, 2 * d + h_d},
	{2 * d, 4 * d},
	{d + h_d, 4 * d},
	
	// 앞
	{h_d, 2 * d + h_d},
	{d, 2 * d + h_d},
	{d, 4 * d},
	{h_d, 4 * d}
};
static const vector<uint32> arr_idx = {
	// 위 0 1 2 3
	0, 1, 2, 0, 2, 3,
	// 아래 7 6 5 4
	4, 5, 6, 4, 6, 7,
	// 오른
	8, 9, 10, 8, 10, 11,
	// 왼 0 3 7 4
	12, 13, 14, 12, 14, 15,
	// 뒤 1 0 4 5
	16, 17, 18, 16, 18, 19,
	// 앞 3 2 6 7
	20, 21, 22, 20, 22, 23
};

enum class HumanParts {
	HEAD,
	BODY,
	LEFTARM,
	RIGHTARM,
	LEFTLEG,
	RIGHTLEG
};

namespace Parts {
	inline void humanVertices(
		vector<VertexPTN>& vertices, 
		float scale_x,
		float scale_y,
		float scale_z,
		HumanParts parts_type
	) {
		vertices.clear();
		VertexPTN ptn;
		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 4; j++) {
				ptn.pos = pos[i * 4 + j];
				ptn.pos.x *= scale_x;
				ptn.pos.y *= scale_y;
				ptn.pos.z *= scale_z;
				ptn.normal = normal[i];
				if (parts_type == HumanParts::HEAD)
					ptn.uv = human_head_uv[i * 4 + j];
				else if (parts_type == HumanParts::BODY)
					ptn.uv = human_body_uv[i * 4 + j];
				else if (parts_type == HumanParts::LEFTARM)
					ptn.uv = human_left_arm_uv[i * 4 + j];
				else if (parts_type == HumanParts::RIGHTARM)
					ptn.uv = human_right_arm_uv[i * 4 + j];
				else if (parts_type == HumanParts::LEFTLEG)
					ptn.uv = human_left_leg_uv[i * 4 + j];
				else
					ptn.uv = human_righ_leg_uv[i * 4 + j];
				vertices.push_back(ptn);
			}
		}
	}

	inline void updateLeftArmVertices(
		vector<VertexPTN>& vertices,
		vec3 const& x_move,
		vec3 const& z_move,
		vec3 const& up,
		vec3 const& down
	) {
		vector<vec3> nbox = {
			up + z_move - x_move,
			up + z_move + x_move,
			up - z_move + x_move,
			up - z_move - x_move,

			down + z_move - x_move,
			down + z_move + x_move,
			down - z_move + x_move,
			down - z_move - x_move
		};
		vector<vec3> npos = {
			// top
			nbox[0], nbox[1], nbox[2], nbox[3],

			// bottom
			nbox[7], nbox[6], nbox[5], nbox[4],

			// right
			nbox[1], nbox[5], nbox[6], nbox[2],

			// left
			nbox[0], nbox[3], nbox[7], nbox[4],

			// back
			nbox[1], nbox[0], nbox[4], nbox[5],

			// front
			nbox[3], nbox[2], nbox[6], nbox[7]
		};
		for (int i = 0; i < 6; i++) {
			for (int j = 0; j < 4; j++) {
				vertices[i * 4 + j].pos = npos[i * 4 + j];
			}
		}
	}

	inline void makeIndices(vector<uint32>& indices) {
		indices.clear();
		for (int i = 0; i < 36; i++) {
			indices.push_back(arr_idx[i]);
		}
	}
};

