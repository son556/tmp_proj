#pragma once

struct Box {
	vec3 top_left_up;
	vec3 velocity;
	float size_x;
	float size_y;
	float size_z;
};

class SweptAABB
{
public:
	/**
	 * @brief 충돌 시간을 반환.
	 * 
	 * \param obj 움직이는 물체
	 * \param other 정지해 있는 물체
	 * \return  충돌시간(0 ~ 1) 사이
	 */
	static float sweptAABB(Box const& obj, Box const& other);

private:
	SweptAABB() = delete;
	SweptAABB(const SweptAABB&) = delete;
	SweptAABB& operator=(const SweptAABB&) = delete;
};

