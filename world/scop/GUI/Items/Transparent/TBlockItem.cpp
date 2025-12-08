#include "pch.h"
#include "TBlockItem.h"
#include "Buffer.h"
#include "DeferredGraphics.h"
#include "WorldUtils.h"

const vec4 TBlockItem::red = vec4(1, 0, 0, 0.3);
const vec4 TBlockItem::blue = vec4(0, 0, 1, 0.3);
const vec4 TBlockItem::green = vec4(0, 1, 0, 0.3);
const vec4 TBlockItem::glass = vec4(0.06, 0.07, 0.1, 0.6);
const vec4 TBlockItem::err_color = vec4(0, 0, 0, 0);

void TBlockItem::setPos(vec3 const& pos)
{
	this->world = Mat::CreateTranslation(pos);
	this->pos_xy = make_pair(pos.x, pos.y);
}

pair<float, float> const& TBlockItem::getPosXY()
{
	return this->pos_xy;
}

Mat const& TBlockItem::getWorldMatrix()
{
	return this->world;
}

shared_ptr<Buffer<VertexColor>> const& TBlockItem::getVertexBuffer()
{
	return this->v_buff;
}

shared_ptr<Buffer<uint32>> const& TBlockItem::getIndexBuffer()
{
	return this->i_buff;
}

void TBlockItem::setInfo(BlockType item_type, 
	vec4 const& color, bool tp_flag, float w_size, float h_size)
{
	BlockItem::setInfo(tp_flag);
	this->item_type = item_type;
	ComPtr<ID3D11Device> const& device = d_graphic->getDevice();
	float scale = 0.14;
	vec3 s = vec3(scale, scale, 0);
	if (w_size != -1)
		s = vec3(w_size, h_size, 0);
	vector<VertexColor> vertices;
	vertices.resize(4);
	vertices[0].pos = vec3(-0.5, 0.5, 0) * s;
	vertices[0].col = color;
	vertices[1].pos = vec3(0.5, 0.5, 0) * s;
	vertices[1].col = color;
	vertices[2].pos = vec3(0.5, -0.5, 0) * s;
	vertices[2].col = color;
	vertices[3].pos = vec3(-0.5, -0.5, 0) * s;
	vertices[3].col = color;

	static vector<uint32> indices = { 0, 1, 3, 1, 2, 3 };

	this->v_buff = make_shared<Buffer<VertexColor>>(
		device,
		vertices.data(),
		vertices.size(),
		D3D11_BIND_VERTEX_BUFFER
	);
	this->i_buff = make_shared<Buffer<uint32>>(
		device,
		indices.data(),
		indices.size(),
		D3D11_BIND_INDEX_BUFFER
	);
}

BlockType TBlockItem::getBlockType()
{
	return this->item_type;
}

bool TBlockItem::checkFreeMove()
{
	return this->free_move_flag;
}

void TBlockItem::setFreeMove(bool free_move_flag)
{
	this->free_move_flag = free_move_flag;
}

vec4 const& TBlockItem::getColor(BlockType block_type)
{
	if (block_type == BlockType::TRANSPARENCY_BLUE)
		return blue;
	if (block_type == BlockType::TRANSPARENCY_RED)
		return red;
	if (block_type == BlockType::TRANSPARENCY_GREEN)
		return green;
	if (block_type == BlockType::GLASS)
		return glass;
	return err_color;
}

void TBlockItem::copyFrom(BlockItem const& other)
{
	BlockItem::copyFrom(other);
	TBlockItem const* oth = dynamic_cast<TBlockItem const*>(&other);
	this->i_buff = oth->i_buff;
	this->v_buff = oth->v_buff;
	this->world = oth->world;
	this->item_type = oth->item_type;
}


