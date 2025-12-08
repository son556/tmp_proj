#include "pch.h"
#include "Body.h"
#include "ConstantBuffer.h"
#include "DeferredGraphics.h"
#include "Parts.h"
#include "Buffer.h"

Body::Body(Mat const& o_pos, Mat const& o_rot) 
{
	vector<VertexPTN> vertices;
	Parts::humanVertices(vertices, 0.5, 0.75, 0.25, HumanParts::BODY);
	this->v_buffer = make_shared<Buffer<VertexPTN>>(
		d_graphic->getDevice(),
		vertices.data(),
		vertices.size(),
		D3D11_BIND_VERTEX_BUFFER
	);
	this->basic_mat = Mat::CreateTranslation(vec3(0, 1.125, 0));
	this->world = this->basic_mat * o_rot * o_pos;
}

shared_ptr<Buffer<VertexPTN>>& Body::getVertexBuffer()
{
	return this->v_buffer;
}

Mat Body::getWorld()
{
	return this->world;
}

void Body::update(Mat const& o_pos, Mat const& o_rot)
{
	this->world = this->basic_mat * o_rot * o_pos;
}

