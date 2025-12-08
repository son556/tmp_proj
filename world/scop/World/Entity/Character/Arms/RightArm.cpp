#include "pch.h"
#include "RightArm.h"
#include "ConstantBuffer.h"
#include "DeferredGraphics.h"
#include "Parts.h"
#include "Buffer.h"

RightArm::RightArm(Mat const& o_pos, Mat const& o_rot)
{
	vector<VertexPTN> vertices;
	Parts::humanVertices(vertices, 0.25, 0.75, 0.25, HumanParts::RIGHTARM);
	this->v_buffer = make_shared<Buffer<VertexPTN>>(
		d_graphic->getDevice(),
		vertices.data(),
		vertices.size(),
		D3D11_BIND_VERTEX_BUFFER
	);
	this->basic_mat = Mat::CreateTranslation(vec3(0.375, 1.125, 0));
	this->world = this->basic_mat * o_rot * o_pos;
	this->animation_flag = false;
}

shared_ptr<Buffer<VertexPTN>>& RightArm::getVertexBuffer()
{
	return this->v_buffer;
}

Mat RightArm::getWorld()
{
	return this->world;
}

void RightArm::update(Mat const& o_pos, Mat const& o_rot)
{
	this->basic_mat = Mat::CreateTranslation(vec3(0.375, 1.125, 0));
	if (this->animation_flag)
		this->animationWalk();
	this->world = this->basic_mat * o_rot * o_pos;
}

void RightArm::setAnimationFlag()
{
	this->animation_flag = true;
}

void RightArm::animationWalk()
{
	static float dt = 0;
	static const float speed = 30;
	static float theta = 0;
	static int dir = -1;
	static bool flag = false;
	static Mat rotX;

	if (flag == false && dir == -1 && theta <= -60) {
		dir = 1;
	}
	else if (flag == false && dir == 1 && theta >= 60) {
		dir = -1;
		flag = true;
	}
	else if (flag && theta <= 0) {
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

