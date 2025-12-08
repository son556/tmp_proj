#include "pch.h"
#include "WaterChunk.h"
#include "Buffer.h"
#include "MapUtils.h"

WaterChunk::WaterChunk()
{
}

WaterChunk::~WaterChunk()
{
}

void WaterChunk::update(ComPtr<ID3D11Device> const& device)
{
	if (this->vertices.size() == 0)
		return;
	this->v_buffer = make_shared<Buffer<VertexWater>>(
		device,
		this->vertices.data(),
		this->vertices.size(),
		D3D11_BIND_VERTEX_BUFFER
	);
	this->i_buffer = make_shared<Buffer<uint32>>(
		device,
		this->indices.data(),
		this->indices.size(),
		D3D11_BIND_INDEX_BUFFER
	);
	this->render_flag = true;
}

void WaterChunk::setBuffer(ComPtr<ID3D11DeviceContext> const& context)
{
	uint32 stride = this->v_buffer->getStride();
	uint32 offset = this->v_buffer->getOffset();
	context->IASetVertexBuffers(0, 1,
		this->v_buffer->getComPtr().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(
		this->i_buffer->getComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);
	context->DrawIndexed(
		this->i_buffer->getCount(),
		0, 0);
}

void WaterChunk::reset()
{
	this->vertices_idx = 0;
	this->vertices.clear();
	this->indices.clear();
	this->render_flag = false;
}
