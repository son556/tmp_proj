#include "pch.h"
#include "LeftLeg.h"
#include "DeferredGraphics.h"
#include "ConstantBuffer.h"
#include "Parts.h"
#include "Buffer.h"

LeftLeg::LeftLeg(Mat const& o_pos, Mat const& o_rot)
{
	vector<VertexPTN> vertices;
	Parts::humanVertices(vertices, 0.25, 0.75, 0.25, HumanParts::LEFTLEG);
	this->v_buffer = make_shared<Buffer<VertexPTN>>(
		d_graphic->getDevice(),
		vertices.data(),
		vertices.size(),
		D3D11_BIND_VERTEX_BUFFER
	);
	this->basic_mat = Mat::CreateTranslation(vec3(-0.125, 0.375, 0));
	this->world = this->basic_mat * o_rot * o_pos;
}

shared_ptr<Buffer<VertexPTN>>& LeftLeg::getVertexBuffer()
{
	return this->v_buffer;
}

Mat LeftLeg::getWorld()
{
	return this->world;
}

void LeftLeg::update(Mat const& o_pos, Mat const& o_rot)
{
	this->basic_mat = Mat::CreateTranslation(vec3(-0.125, 0.375, 0));
	if (this->animation_flag)
		this->animationWalk();
	this->world = this->basic_mat * o_rot * o_pos;
}

void LeftLeg::setAnimationFlag()
{
	this->animation_flag = true;
}

void LeftLeg::animationWalk()
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
