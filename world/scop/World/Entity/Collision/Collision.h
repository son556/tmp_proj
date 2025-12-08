#pragma once

#include "WorldUtils.h"
#include "CollisionUtils.h"

class Collision
{
public:
	Collision(vec3 const& pos, vec3 size_xyz);
	~Collision() = default;
	void update(vec3 const& new_pos);

	/**
	 * @brief 충돌 후 캐릭터 위치 계산.
	 * 
	 * \param down_pos 현위치
	 * \param dir 이동방향
	 * \param distance 이동거리
	 * \return 이동 후 위치
	 */
	vec3 calcCollision(
		vec3 const& down_pos,
		vec3 const& dir,
		float distance
	);
	bool checkCollisionNowPosition(vec3 const& down_pos);
	bool checkBottom(vec3 const& down_pos);

private: // test
	Collision() = delete;
	Collision(Collision const&) = delete;
	Collision& operator=(Collision const&) = delete;

private:
	CollisionUtils c_utils;
	vec3 center_pos;
	float size_x;
	float size_y;
	float size_z;

};

