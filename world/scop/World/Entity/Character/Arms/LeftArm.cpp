#include "pch.h"
#include "LeftArm.h"
#include "ConstantBuffer.h"
#include "DeferredGraphics.h"
#include "Parts.h"
#include "Buffer.h"
#include "TestCam.h"

LeftArm::LeftArm(Mat const& o_pos, Mat const& o_rot)
{
	Parts::humanVertices(this->vertices, 0.25, 0.75, 0.25, HumanParts::LEFTARM);
	this->v_buffer = make_shared<Buffer<VertexPTN>>(
		d_graphic->getDevice(),
		this->vertices.data(),
		this->vertices.size(),
		D3D11_BIND_VERTEX_BUFFER,
		true
	);
	this->ani_vertices = this->vertices;

	this->basic_mat = Mat::CreateTranslation(vec3(-0.375, 1.125, 0));
	this->world = this->basic_mat * o_rot * o_pos;
	this->animation_flag = 0;
}

shared_ptr<Buffer<VertexPTN>>& LeftArm::getVertexBuffer()
{
	return this->v_buffer;
}

Mat LeftArm::getWorld()
{
	return this->world;
}

void LeftArm::update(Mat const& o_pos, Mat const& o_rot)
{
	if (first_view) {
		vec3&& c_dir = cam->getDir();
		float d = sqrt(c_dir.x * c_dir.x + c_dir.z * c_dir.z);
		float theta = atan2(c_dir.y, d);
		Mat rotX = Mat::CreateRotationX(theta);
		this->basic_mat = Mat::CreateTranslation(vec3(0, -0.25, 0)) *
			Mat::CreateRotationX(XMConvertToRadians(90)) * rotX * 
			Mat::CreateTranslation(vec3(-0.375, 1.375, 0));
	}
	else
		this->basic_mat = Mat::CreateTranslation(vec3(-0.375, 1.125, 0));

	if (this->animation_flag) {
		if (first_view)
			this->animationFirstView();
		else
			this->animation();
	}
	if (this->animation_flag_walk)
		this->animationWalk();
	this->world = this->basic_mat * o_rot * o_pos;
}

int LeftArm::getAnimationFlag()
{
	return this->animation_flag;
}

void LeftArm::setAnimationFlag()
{
	this->animation_flag = true;
}

void LeftArm::setAnimationFlagWalk()
{
	this->animation_flag_walk = true;
}

void LeftArm::animation()
{
	static const vec3 s = XMVector3Normalize(vec3(0, -0.325, -0.25));
	static const float speed = -40;
	static const vec3 up = vec3(0, 0, 1);
	static float w = 0;
	static float sin = 0;
	static float dt = 0;

	if (dt * -speed >= 360) {
		this->v_buffer->update(this->vertices, d_graphic->getContext());
		w = 0;
		dt = 0;
		sin = 0;
		this->animation_flag = 0;
		return;
	}
	else if (dt) {
		SimpleMath::Quaternion up_q = vec4(0, 0, 0, 0);
		SimpleMath::Quaternion d_q = vec4(0, -0.75, 0, 0);
		SimpleMath::Quaternion q = vec4(s.x * sin, s.y * sin, s.z * sin, w);
		SimpleMath::Quaternion q_conjugate = vec4(-s.x * sin, -s.y * sin, -s.z * sin, w);

		up_q = q * up_q * q_conjugate;
		d_q = q * d_q * q_conjugate;
		vec3 dir = XMVector3Normalize(vec3(d_q.x, d_q.y, d_q.z) - vec3(up_q.x, up_q.y, up_q.z));
		vec3 right = XMVector3Normalize(up.Cross(dir));
		right *= 0.125;
		vec3 z_axis = XMVector3Normalize(dir.Cross(right));
		z_axis *= 0.125;
		vec3 c_up = vec3(up_q.x, up_q.y, up_q.z);
		vec3 c_down = vec3(d_q.x, d_q.y, d_q.z);

		Parts::updateLeftArmVertices(this->ani_vertices, right, z_axis, c_up, c_down);
		this->v_buffer->update(this->ani_vertices, d_graphic->getContext());
		Mat t = Mat::CreateTranslation(vec3(0, 0.375, 0));
		this->basic_mat = this->basic_mat * t;
	}
	dt += delta_time * 30;
	w = cosf(dt * XMConvertToRadians(speed) * 0.5);
	sin = sinf(dt * XMConvertToRadians(speed) * 0.5);
}

void LeftArm::animationFirstView()
{
	static float dt = 0;
	static const float speed = 30;
	static float theta = 0;
	static int dir = 1;
	static bool flag = false;
	static Mat rotX;

	if (flag == false && dir == 1 && theta >= 60) {
		dir = -1;
	}
	else if (flag == false && dir == -1 && theta <= -60) {
		dir = 1;
		flag = true;
	}
	else if (flag && theta > 0) {
		dt = 0;
		flag = false;
		this->animation_flag = false;
		return;
	}
	static const Mat move = Mat::CreateTranslation(vec3(0, -0.25, 0));
	static const Mat r_move = Mat::CreateTranslation(vec3(0, 0.25, 0));
	this->basic_mat = move * rotX * r_move * this->basic_mat;
	dt += delta_time * dir;
	theta = dt * speed * 20;
	rotX = Mat::CreateRotationX(XMConvertToRadians(theta));
}

void LeftArm::animationWalk()
{
	static float dt = 0;
	static const float speed = 30;
	static float theta = 0;
	static int dir = 1;
	static bool flag = false;
	static Mat rotX;

	if (flag == false && dir == 1 && theta >= 60) {
		dir = -1;
	}
	else if (flag == false && dir == -1 && theta <= -60) {
		dir = 1;
		flag = true;
	}
	else if (flag && theta > 0) {
		dt = 0;
		flag = false;
		this->animation_flag_walk = false;
		return;
	}
	static const Mat move = Mat::CreateTranslation(vec3(0, -0.25, 0));
	static const Mat r_move = Mat::CreateTranslation(vec3(0, 0.25, 0));
	if (this->animation_flag == false)
		this->basic_mat = move * rotX * r_move * this->basic_mat;
	dt += delta_time * dir;
	theta = dt * speed * 20;
	rotX = Mat::CreateRotationX(XMConvertToRadians(theta));
}

