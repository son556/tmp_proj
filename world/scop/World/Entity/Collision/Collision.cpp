#include "pch.h"
#include "Collision.h"
#include "Terrain.h"
#include "SweptAABB.h"

#define STDZERO 0.00000001

Collision::Collision(vec3 const& pos, vec3 size_xyz) 
	: c_utils(size_xyz), center_pos(pos), size_x(size_xyz.x), 
	size_y(size_xyz.y), size_z(size_xyz.z)
{
}

void Collision::update(vec3 const& new_pos)
{
	this->center_pos = new_pos;
}

vec3 Collision::calcCollision(
	vec3 const& down_pos, 
	vec3 const& dir,
	float distance
)
{
	vec3 ans;
	ans = this->c_utils.calcCollisionY(down_pos, dir.y * distance);
	if (abs(dir.z) > abs(dir.x)) {
		ans = this->c_utils.calcCollisionZ(ans, dir.z * distance);
		ans = this->c_utils.calcCollisionX(ans, dir.x * distance);
	}
	else {
		ans = this->c_utils.calcCollisionX(ans, dir.x * distance);
		ans = this->c_utils.calcCollisionZ(ans, dir.z * distance);
	}
	return ans;
}

bool Collision::checkCollisionNowPosition(vec3 const& down_pos)
{
	return this->c_utils.checkCollision(down_pos);
}

bool Collision::checkBottom(vec3 const& down_pos)
{
	return this->c_utils.checkBottom(down_pos);
}

