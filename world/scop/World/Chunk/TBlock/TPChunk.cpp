#include "pch.h"
#include "TPChunk.h"
#include "Buffer.h"

TPChunk::TPChunk()
{
}

TPChunk::~TPChunk()
{
}

void TPChunk::render(
	ComPtr<ID3D11DeviceContext> const& context,
	bool flag_up
)
{
	uint32 stride;
	uint32 offset;
	if (flag_up) {
		stride = this->tp_up_vbuffer->getStride();
		offset = this->tp_up_vbuffer->getOffset();
		context->IASetVertexBuffers(
			0, 1,
			this->tp_up_vbuffer->getComPtr().GetAddressOf(),
			&stride, &offset
		);
		context->IASetIndexBuffer(
			this->tp_up_ibuffer->getComPtr().Get(),
			DXGI_FORMAT_R32_UINT,
			0
		);
		context->DrawIndexed(
			this->tp_up_ibuffer->getCount(),
			0, 0);
	}
	else {
		stride = this->tp_down_vbuffer->getStride();
		offset = this->tp_down_vbuffer->getOffset();
		context->IASetVertexBuffers(
			0, 1,
			this->tp_down_vbuffer->getComPtr().GetAddressOf(),
			&stride, &offset
		);
		context->IASetIndexBuffer(
			this->tp_down_ibuffer->getComPtr().Get(),
			DXGI_FORMAT_R32_UINT,
			0
		);
		context->DrawIndexed(
			this->tp_down_ibuffer->getCount(),
			0, 0);
	}
}

void TPChunk::update(
	ComPtr<ID3D11Device> const& device,
	bool flag_up
)
{
	if (flag_up) {
		if (this->vertices_up.size() == 0)
			return;
		this->tp_up_vbuffer = make_shared<Buffer<VertexColor>>(
			device,
			this->vertices_up.data(),
			this->vertices_up.size(),
			D3D11_BIND_VERTEX_BUFFER
		);
		this->tp_up_ibuffer = make_shared<Buffer<uint32>>(
			device,
			this->indices_up.data(),
			this->indices_up.size(),
			D3D11_BIND_INDEX_BUFFER
		);
		this->render_up_flag = true;
	}
	else {
		if (this->vertices_down.size() == 0)
			return;
		this->tp_down_vbuffer = make_shared<Buffer<VertexColor>>(
			device,
			this->vertices_down.data(),
			this->vertices_down.size(),
			D3D11_BIND_VERTEX_BUFFER
		);
		this->tp_down_ibuffer = make_shared<Buffer<uint32>>(
			device,
			this->indices_down.data(),
			this->indices_down.size(),
			D3D11_BIND_INDEX_BUFFER
		);
		this->render_down_flag = true;
	}
}

void TPChunk::reset()
{
	this->vertices_idx_up = 0;
	this->render_up_flag = false;
	this->vertices_up.clear();
	this->indices_up.clear();

	this->vertices_idx_down = 0;
	this->render_down_flag = false;
	this->vertices_down.clear();
	this->indices_down.clear();
}

