#include "pch.h"
#include "WaterRefraction.h"
#include "DeferredGraphics.h"
#include "DeferredBuffer.h"
#include "VertexShader.h"
#include "RasterizerState.h"
#include "PixelShader.h"
#include "ConstantBuffer.h"
#include "Buffer.h"
#include "MapUtils.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "Block.h"
#include "SamplerState.h"
#include "TestCam.h"

WaterRefraction::WaterRefraction(MapUtils* m_info)
{
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	
	this->d_buff = make_shared<DeferredBuffer>(1);
	this->d_buff->setRTVsAndSRVs(device, m_info->width, m_info->height);
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
		L"shader/World/OIT/Water/WaterRefraction/RefractionVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->input_layout = make_shared<InputLayout>(
		device,
		InputLayouts::layout_pt.data(),
		InputLayouts::layout_pt.size(),
		this->vertex_shader->getBlob()
	);
	this->rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE
	);
	this->pixel_shader = make_shared<PixelShader>(
		device,
		L"shader/World/OIT/Water/WaterRefraction/RefractionPS.hlsl",
		"main",
		"ps_5_0"
	);
	vec4 v;
	this->constant_buffer = make_shared<ConstantBuffer>(
		device,
		d_graphic->getContext(),
		v
	);
	this->sampler_state = make_shared<SamplerState>(device);


	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = false;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	desc.StencilEnable = true;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	HRESULT hr = device->CreateDepthStencilState(&desc, 
		this->ds_state.GetAddressOf());
	CHECK(hr);
}

void WaterRefraction::render(
	ComPtr<ID3D11ShaderResourceView> color, 
	ComPtr<ID3D11DepthStencilView> dsv
)
{
	static float water_move = 0;
	water_move += WATER_SPEED * delta_time;
	water_move = fmod(water_move, 1.0f);
	vec4 v = vec4(water_move, 0, 0, 0);
	this->constant_buffer->update(v);

	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	d_graphic->renderBegin(this->d_buff.get(), dsv, true, false);
	this->setPipe();
	context->OMSetDepthStencilState(this->ds_state.Get(), 1);
	context->PSSetConstantBuffers(0, 1,
		this->constant_buffer->getComPtr().GetAddressOf());

	context->PSSetShaderResources(0, 1, color.GetAddressOf());
	context->PSSetShaderResources(1, 1, 
		this->water_distortion_srv.GetAddressOf());
	context->DrawIndexed(this->i_buff->getCount(), 0, 0);

	context->OMSetDepthStencilState(nullptr, 0);
}

ComPtr<ID3D11ShaderResourceView> WaterRefraction::getSRV()
{
	return this->d_buff->getSRV(0);
}

void WaterRefraction::setWaterND(
	ComPtr<ID3D11ShaderResourceView> normal,
	ComPtr<ID3D11ShaderResourceView> distortion
)
{
	this->water_normal_srv = normal;
	this->water_distortion_srv = distortion;
}

void WaterRefraction::setPipe()
{
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);
	uint32 stride = this->v_buff->getStride();
	uint32 offset = this->v_buff->getOffset();
	context->IASetVertexBuffers(0, 1,
		this->v_buff->getComPtr().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(this->i_buff->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT, 0);
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
