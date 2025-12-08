#include "pch.h"
#include "SweptAABB.h"


float fmin(float a, float b, float c) {
	float res = min(a, b);
	res = min(res, c);
	return res;
}

float fmax(float a, float b, float c) {
	float res = max(a, b);
	res = max(res, c);
	return res;
}

float SweptAABB::sweptAABB(Box const& obj, Box const& other)
{
	float dx_entry, dy_entry, dz_entry;
	float dx_exit, dy_exit, dz_exit;

	if (obj.velocity.x > 0) {
		dx_entry = other.top_left_up.x - (obj.top_left_up.x + obj.size_x);
		dx_exit = (other.top_left_up.x + other.size_x) - obj.top_left_up.x;
	}
	else {
		dx_entry = (other.top_left_up.x + other.size_x) - obj.top_left_up.x;
		dx_exit = other.top_left_up.x - (obj.top_left_up.x + obj.size_x);
	}
	if (obj.velocity.y > 0) {
		dy_entry = (other.top_left_up.y - other.size_y) - obj.top_left_up.y;
		dy_exit = other.top_left_up.y - (obj.top_left_up.y - obj.size_y);
	}
	else {
		dy_entry = other.top_left_up.y - (obj.top_left_up.y - obj.size_y);
		dy_exit = (other.top_left_up.y - other.size_y) - obj.top_left_up.y;
	}
	if (obj.velocity.z > 0) {
		dz_entry = (other.top_left_up.z - other.size_z) - obj.top_left_up.z;
		dz_exit = other.top_left_up.z - (obj.top_left_up.z - obj.size_z);
	}
	else {
		dz_entry = other.top_left_up.z - (obj.top_left_up.z - obj.size_z);
		dz_exit =(other.top_left_up.z - other.size_z) - obj.top_left_up.z;
	}

	float tx_entry, ty_entry, tz_entry;
	float tx_exit, ty_exit, tz_exit;

	if (obj.velocity.x == 0) {
		tx_entry = -numeric_limits<float>::infinity();
		tx_exit = numeric_limits<float>::infinity();
	}
	else {
		tx_entry = dx_entry / obj.velocity.x;
		tx_exit = dx_exit / obj.velocity.x;
	}
	if (obj.velocity.y == 0) {
		ty_entry = -numeric_limits<float>::infinity();
		ty_exit = numeric_limits<float>::infinity();
	}
	else {
		ty_entry = dy_entry / obj.velocity.y;
		ty_exit = dy_exit / obj.velocity.y;
	}
	if (obj.velocity.z == 0) {
		tz_entry = -numeric_limits<float>::infinity();
		tz_exit = numeric_limits<float>::infinity();
	}
	else {
		tz_entry = dz_entry / obj.velocity.z;
		tz_exit = dz_exit / obj.velocity.z;
	}

	float entry_time = fmax(tx_entry, ty_entry, tz_entry);
	float exit_time = fmin(tx_exit, ty_exit, tz_exit);
	if (entry_time > exit_time ||
		tx_entry < 0.0f && ty_entry < 0.0f && tz_entry < 0.0f ||
		tx_entry > 1.0f || ty_entry > 1.0f || tz_entry > 1.0f) {
		return 1.0f;
	}

	return entry_time;
}
