#include "pch.h"
#include "OIT.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "RasterizerState.h"
#include "DeferredBuffer.h"
#include "DeferredGraphics.h"
#include "Block.h"
#include "Buffer.h"
#include "SamplerState.h"

OIT::OIT(MapUtils* minfo) 
	: m_info(minfo), d_buff_up(nullptr), d_buff_down(nullptr),
	tp(minfo), cp(minfo)
{
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	this->d_buff_up = make_shared<DeferredBuffer>(1);
	this->d_buff_up->setRTVsAndSRVs(device,
		this->m_info->width, this->m_info->height);
	this->d_buff_down = make_shared<DeferredBuffer>(1);
	this->d_buff_down->setRTVsAndSRVs(device,
		this->m_info->width, this->m_info->height);

	this->vertex_shader = make_shared<VertexShader>(
		device,
		L"shader/World/OIT/OITResultVS.hlsl",
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
		L"shader/World/OIT/OITResultPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_BACK
	);
	this->sampler_state = make_shared<SamplerState>(device);
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
}

void OIT::setRTVandSRV(
	ComPtr<ID3D11RenderTargetView> rtv, 
	ComPtr<ID3D11ShaderResourceView> srv,
	ComPtr<ID3D11ShaderResourceView> depth_srv
)
{
	this->solid_rtv = rtv;
	this->solid_srv = srv;
	this->depth_srv = depth_srv;
}

void OIT::setPipe()
{
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
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
	context->PSSetSamplers(0, 1,
		this->sampler_state->getComPtr().GetAddressOf());
}

void OIT::render(CamType type, bool water_up_flag)
{
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();

	// transparent render
	this->tp.render(type, this->depth_srv, water_up_flag);

	// composition render
	this->cp.render(this->solid_rtv, this->tp.getAccum(),
			this->tp.getReveal());

	// result render
	this->setPipe();
	UINT stride = this->v_buff->getStride();
	UINT offset = this->v_buff->getOffset();
	context->IASetVertexBuffers(0, 1,
		this->v_buff->getComPtr().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(
		this->i_buff->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);
	if (water_up_flag)
		d_graphic->renderBegin(this->d_buff_up.get());
	else
		d_graphic->renderBegin(this->d_buff_down.get());
	context->PSSetShaderResources(0, 1, this->solid_srv.GetAddressOf());
	context->DrawIndexed(
		this->i_buff->getCount(),
		0, 0);
}

ComPtr<ID3D11ShaderResourceView> OIT::getSRV(bool up_flag)
{
	if (up_flag)
		return this->d_buff_up->getSRV(0);
	else
		return this->d_buff_down->getSRV(0);
}
