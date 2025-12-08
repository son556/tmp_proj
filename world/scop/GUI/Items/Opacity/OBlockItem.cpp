#include "pch.h"
#include "OBlockItem.h"
#include "DeferredGraphics.h"
#include "Block.h"
#include "Buffer.h"
#include "Texture.h"

void OBlockItem::setPos(vec3 const& pos)
{
	this->world = Mat::CreateTranslation(pos);
	this->pos_xy = make_pair(pos.x, pos.y);
}

pair<float, float> const& OBlockItem::getPosXY()
{
	return this->pos_xy;
}

Mat const& OBlockItem::getWorldMatrix()
{
	return this->world;
}

shared_ptr<Buffer<VertexDefer>> const& OBlockItem::getVertexBuffer()
{
	return this->v_buffer;
}

shared_ptr<Buffer<uint32>> const& OBlockItem::getIndexBuffer()
{
	return this->i_buffer;
}

void OBlockItem::setInfo(BlockType const& item_type, bool tp_flag, 
	float w_size, float h_size)
{
	BlockItem::setInfo(tp_flag);
	this->item_type = item_type;
	ComPtr<ID3D11Device> const& device = d_graphic->getDevice();
	vector<VertexDefer> vertices;
	vector<uint32> indices;
	if (w_size == -1)
		Block::makeBox(0.07, vertices, indices);
	else
		Block::makeBox(w_size, h_size, vertices, indices);
	this->v_buffer = make_shared<Buffer<VertexDefer>>(device,
		vertices.data(), vertices.size(), D3D11_BIND_VERTEX_BUFFER);
	this->i_buffer = make_shared<Buffer<uint32>>(device,
		indices.data(), indices.size(), D3D11_BIND_INDEX_BUFFER);
}

BlockType OBlockItem::getBlockType()
{
	return this->item_type;
}

bool OBlockItem::checkFreeMove()
{
	return this->free_move_flag;
}

void OBlockItem::setFreeMove(bool free_move_flag)
{
	this->free_move_flag = free_move_flag;
}

void OBlockItem::copyFrom(BlockItem const& other)
{
	BlockItem::copyFrom(other);
	const OBlockItem* p = dynamic_cast<const OBlockItem*>(&other);
	this->item_type = p->item_type;
	this->world = p->world;
	this->i_buffer = p->i_buffer;
	this->v_buffer = p->v_buffer;
}

