#include "pch.h"
#include "Water.h"
#include "DeferredGraphics.h"
#include "DeferredBuffer.h"
#include "VertexShader.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "RasterizerState.h"
#include "PixelShader.h"
#include "MapUtils.h"
#include "SamplerState.h"
#include "Buffer.h"
#include "Block.h"
#include "Texture.h"
//#include "Utils/TestUtils/TestRender.h"
#include "ConstantBuffer.h"
#include "TestCam.h"

Water::Water(MapUtils* m_info)
	: m_info(m_info), water_init(m_info), 
	water_reflection(m_info), water_refraction(m_info)
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
		L"shader/World/OIT/Water/WaterVS.hlsl",
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
		D3D11_CULL_BACK
	);
	this->pixel_shader = make_shared<PixelShader>(
		device,
		L"shader/World/OIT/Water/WaterPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->sampler_state = make_shared<SamplerState>(device);
	Texture tex_distortion(device, 
		L"./textures/water/water_distortion_rx_gy.png");
	this->water_distortion_srv = tex_distortion.getComPtr();
	Texture tex_normal(device,
		L"./textures/water/water_normal_rx_gz_by.png");
	this->water_normal_srv = tex_normal.getComPtr();

	this->water_reflection.setWaterND(
		this->water_normal_srv, this->water_distortion_srv);
	this->water_refraction.setWaterND(
		this->water_normal_srv, this->water_distortion_srv);
	vec4 v;
	this->constant_buffer = make_shared<ConstantBuffer>(
		device,
		d_graphic->getContext(),
		v
	);

	//this->rt = make_shared<TestRender>(m_info);
}

void Water::setPipe()
{
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	uint32 stride = this->v_buff->getStride();
	uint32 offset = this->v_buff->getOffset();
	context->IASetVertexBuffers(0, 1,
		this->v_buff->getComPtr().GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(this->i_buff->getComPtr().Get(), 
		DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(this->input_layout->getComPtr().Get());
	context->VSSetShader(
		this->vertex_shader->getComPtr().Get(),
		nullptr, 0);
	context->RSSetState(this->rasterizer_state->getComPtr().Get());
	context->PSSetShader(this->pixel_shader->getComPtr().Get(),
		nullptr, 0);
	context->PSSetSamplers(0, 1, 
		this->sampler_state->getComPtr().GetAddressOf());
}

void Water::render(ComPtr<ID3D11ShaderResourceView> color)
{
	vec3 cp = cam->getPos();
	vec4 buff = vec4(cp.x, cp.y, cp.z, 1);
	if (under_water)
		buff.w = -1;
	this->constant_buffer->update(buff);
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	
	d_graphic->renderBegin(this->d_buff.get());
	this->setPipe();
	context->PSSetShaderResources(0, 1,
		this->water_reflection.getSRV().GetAddressOf());
	context->PSSetShaderResources(1, 1,
		this->water_refraction.getSRV().GetAddressOf());
	context->PSSetShaderResources(2, 1,
		color.GetAddressOf());
	context->PSSetShaderResources(3, 1,
		this->water_init.getSRV(WaterRTVType::POSITION).GetAddressOf());
	context->PSSetConstantBuffers(0, 1,
		this->constant_buffer->getComPtr().GetAddressOf());
	context->DrawIndexed(this->i_buff->getCount(), 0, 0);
}

void Water::renderWaterInit(ComPtr<ID3D11ShaderResourceView> depth_srv)
{
	// 물위치 그리기, 물 위치 스텐실에 표시
	this->water_init.render(CamType::NORMAL, depth_srv);
}

void Water::renderWaterRefraction(ComPtr<ID3D11ShaderResourceView> color)
{
	// 물의 굴절을 그림
	this->water_refraction.render(color, this->water_init.getDSV());
}

void Water::renderWaterReflection()
{
	// 물의 반사를 그림
	this->water_reflection.setDSV(this->water_init.getDSV());
	this->water_reflection.render();
}

ComPtr<ID3D11ShaderResourceView> Water::getSRV()
{
	return this->d_buff->getSRV(0);
}

ComPtr<ID3D11RenderTargetView> Water::getRTV()
{
	return this->d_buff->getRTV(0);
}
