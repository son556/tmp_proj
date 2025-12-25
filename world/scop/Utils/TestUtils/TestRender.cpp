#include "pch.h"
#include "TestRender.h"
#include "DeferredGraphics.h"
#include "DeferredBuffer.h"
#include "MapUtils.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "VertexShader.h"
#include "RasterizerState.h"
#include "PixelShader.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Block.h"
#include "TestCam.h"

TestRender::TestRender(MapUtils* m_info)
	: m_info(m_info)
{
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	vector<VertexDefer> vertices;
	vector<uint32> indices;
	Block::makeBox(1, vertices, indices);
	this->v_buff = make_shared<Buffer<VertexDefer>>(
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
	this->vertex_shader = make_shared<VertexShader>(
		device,
		L"shader/World/FinishVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->input_layout = make_shared<InputLayout>(
		device,
		InputLayouts::layout_pt.data(),
		InputLayouts::layout_pt.size(),
		this->vertex_shader->getBlob()
	);
	this->pixel_shader = make_shared<PixelShader>(
		device,
		L"shader/World/FinishPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_BACK
	);
	this->sampler_state = make_shared<SamplerState>(device);
}

void TestRender::setPipe()
{
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);
	uint32 offset = this->v_buff->getOffset();
	uint32 stride = this->v_buff->getStride();
	context->IASetVertexBuffers(0, 1,
		this->v_buff->getComPtr().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(
		this->i_buff->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);
	context->IASetInputLayout(this->input_layout->getComPtr().Get());
	context->VSSetShader(
		this->vertex_shader->getComPtr().Get(),
		nullptr,
		0
	);
	context->RSSetState(this->rasterizer_state->getComPtr().Get());
	context->PSSetShader(
		this->pixel_shader->getComPtr().Get(),
		nullptr,
		0
	);
	context->PSSetSamplers(0, 
		1, this->sampler_state->getComPtr().GetAddressOf());
}

void TestRender::render(ComPtr<ID3D11ShaderResourceView> srv)
{
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	d_graphic->renderBegin();
	this->setPipe();
	context->PSSetShaderResources(0, 1, srv.GetAddressOf());
	context->DrawIndexed(this->i_buff->getCount(), 0, 0);
	d_graphic->renderEnd();
}
