#include "pch.h"
#include "CubeMap.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Block.h"
#include "Buffer.h"
#include "DeferredGraphics.h"
#include "DeferredBuffer.h"
#include "RasterizerState.h"
#include "SamplerState.h"
#include "ConstantBuffer.h"
#include "SunMoon.h"
#include "Texture.h"
#include "TestCam.h"

CubeMap::CubeMap(UINT width, UINT height)
{
	this->width = width;
	this->height = height;
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	this->d_buffer = make_shared<DeferredBuffer>(1);
	this->d_buffer->setRTVsAndSRVs(device, width, height);
	this->vertex_shader = make_shared<VertexShader>(
		device,
		L"shader/World/DeferredShader/SkyBox/SkyBoxVS.hlsl",
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
	this->ccw_rasterizer_sate = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_BACK,
		true
	);
	this->pixel_shader = make_shared<PixelShader>(
		device,
		L"shader/World/DeferredShader/SkyBox/SkyBoxPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->sampler_state = make_shared<SamplerState>(device);
	this->cube_tex = make_shared<Texture>(
		device,
		L"./textures/skybox/HDRI/MyCubesEnvHDR.dds",
		true
	);
	vector<VertexDefer> vertices;
	vector<uint32> indices;
	Block::makeCubeMap(
		300.f,
		vertices,
		indices
	);
	this->vbuffer = make_shared<Buffer<VertexDefer>>(
		device,
		vertices.data(),
		vertices.size(),
		D3D11_BIND_VERTEX_BUFFER
	);
	this->ibuffer = make_shared<Buffer<uint32>>(
		device,
		indices.data(),
		indices.size(),
		D3D11_BIND_INDEX_BUFFER
	);
	MVP mvp;
	this->constant_buffer = make_shared<ConstantBuffer>(
		device,
		d_graphic->getContext(),
		mvp
	);
}

void CubeMap::render(CamType type, bool ccw_flag)
{	
	d_graphic->renderBegin(this->d_buffer.get());
	this->setPipe(ccw_flag);
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	MVP mvp = cam->getMVP(type);
	vec3 cam_pos = cam->getPos();
	Mat m = XMMatrixTranslation(cam_pos.x, cam_pos.y, cam_pos.z);
	mvp.model = m * mvp.model;
	mvp.model = mvp.model.Transpose();
	mvp.view = mvp.view.Transpose();
	mvp.proj = mvp.proj.Transpose();
	this->constant_buffer->update(mvp);
	context->VSSetConstantBuffers(0, 1,
		this->constant_buffer->getComPtr().GetAddressOf());
	context->DrawIndexed(this->ibuffer->getCount(), 0, 0);
}

ComPtr<ID3D11ShaderResourceView> CubeMap::getSRV()
{
	return this->d_buffer->getSRV(0);
}


void CubeMap::setPipe(bool ccw_flag)
{
	ComPtr<ID3D11DeviceContext> context;
	context = d_graphic->getContext();
	context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);
	context->IASetInputLayout(this->input_layout->getComPtr().Get());
	UINT offset = this->vbuffer->getOffset();
	UINT stride = this->vbuffer->getStride();
	context->IASetVertexBuffers(0, 1,
		this->vbuffer->getComPtr().GetAddressOf(),
		&stride, &offset);
	context->IASetIndexBuffer(
		this->ibuffer->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);
	context->VSSetShader(
		this->vertex_shader->getComPtr().Get(),
		nullptr,
		0
	);
	if (ccw_flag == false)
		context->RSSetState(this->rasterizer_state->getComPtr().Get());
	else
		context->RSSetState(this->ccw_rasterizer_sate->getComPtr().Get());
	context->PSSetShader(
		this->pixel_shader->getComPtr().Get(),
		nullptr,
		0
	);
	context->PSSetShaderResources(0, 1, 
		this->cube_tex->getComPtr().GetAddressOf());
	context->PSSetSamplers(0, 1,
		this->sampler_state->getComPtr().GetAddressOf());
}
