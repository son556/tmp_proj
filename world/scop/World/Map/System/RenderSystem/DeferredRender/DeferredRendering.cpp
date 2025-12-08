#include "pch.h"
#include "DeferredRendering.h"
#include "MapUtils.h"
#include "DeferredGraphics.h"
#include "RasterizerState.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "Buffer.h"
#include "Block.h"
#include "DeferredBuffer.h"
#include "TestCam.h"

#include "Entity.h"

DeferredRendering::DeferredRendering(MapUtils* minfo)
	: m_info(minfo), opacity_render(minfo), oit(minfo), water(minfo)
{
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	
	this->rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_BACK
	);
	vector<VertexDefer> vertices;
	vector<uint32> indices;
	Block::makeBox(1, vertices, indices);
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

	// 결과 출력용
	this->fin_vs = make_shared<VertexShader>(
		device,
		L"shader/World/FinishVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->fin_ia = make_shared<InputLayout>(
		device,
		InputLayouts::layout_pt.data(),
		InputLayouts::layout_pt.size(),
		this->fin_vs->getBlob()
	);
	this->fin_ps = make_shared<PixelShader>(
		device,
		L"shader/World/FinishPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->d_buffer = make_shared<DeferredBuffer>(1);
	this->d_buffer->setRTVsAndSRVs(device, minfo->width, minfo->height);
}

void DeferredRendering::Render()
{
	ComPtr<ID3D11DeviceContext> context;
	context = d_graphic->getContext();
	RenderOption r_opt;

	// opacity render
	r_opt.parallax_mapping = true;
	r_opt.ssao = true;
	r_opt.shadow = true;
	r_opt.cave_shadow = true;
	r_opt.reflection_flag = false;
	r_opt.ssao_blur_cnt = 3;
	this->opacity_render.render(r_opt, CamType::NORMAL);

	if (under_water)
		this->renderUnderWater(context);
	else
		this->renderAboveWater(context);

	context->DrawIndexed(this->ibuffer->getCount(), 0, 0);
}

ComPtr<ID3D11ShaderResourceView>& DeferredRendering::getSRV()
{
	return this->d_buffer->getSRV(0);
}

void DeferredRendering::renderUnderWater(ComPtr<ID3D11DeviceContext>& context)
{
	// oit up render
	this->oit.setRTVandSRV(this->opacity_render.getRTV(),
		this->opacity_render.getSRV(), this->opacity_render.getGeoDepthSRV());
	this->oit.render(CamType::NORMAL, true);

	// water init;
	this->water.renderWaterInit(this->opacity_render.getGeoDepthSRV());

	// water refraction render
	this->water.renderWaterRefraction(this->oit.getSRV(true));

	// water reflection render
	this->water.renderWaterReflection();

	// water render
	this->water.render(this->opacity_render.getSRV());

	// oit down render
	this->oit.setRTVandSRV(this->water.getRTV(), this->water.getSRV(),
		this->opacity_render.getGeoDepthSRV());
	this->oit.render(CamType::NORMAL, false);

	// result
	d_graphic->renderBegin(this->d_buffer.get());
	this->setFinPipe();
	context->PSSetShaderResources(0, 1,
		this->oit.getSRV(false).GetAddressOf());
}

void DeferredRendering::renderAboveWater(ComPtr<ID3D11DeviceContext>& context)
{
	// oit down render
	this->oit.setRTVandSRV(this->opacity_render.getRTV(),
		this->opacity_render.getSRV(), this->opacity_render.getGeoDepthSRV());
	this->oit.render(CamType::NORMAL, false);

	// water init;
	this->water.renderWaterInit(this->opacity_render.getGeoDepthSRV());

	// water refraction render
	this->water.renderWaterRefraction(this->oit.getSRV(false));

	// water reflection render
	this->water.renderWaterReflection();

	// water render
	this->water.render(this->opacity_render.getSRV());

	// oit up render
	this->oit.setRTVandSRV(this->water.getRTV(), this->water.getSRV(),
		this->opacity_render.getGeoDepthSRV());
	this->oit.render(CamType::NORMAL, true);

	// result
	d_graphic->renderBegin(this->d_buffer.get());
	this->setFinPipe();
	context->PSSetShaderResources(0, 1,
		this->oit.getSRV(true).GetAddressOf());
}

void DeferredRendering::setFinPipe()
{
	ComPtr<ID3D11DeviceContext> context;
	context = d_graphic->getContext();
	context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(this->fin_ia->getComPtr().Get());
	uint32 stride = this->vbuffer->getStride();
	uint32 offset = this->vbuffer->getOffset();
	context->IASetVertexBuffers(0, 1,
		this->vbuffer->getComPtr().GetAddressOf(),
		&stride, &offset);
	context->IASetIndexBuffer(
		this->ibuffer->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);
	context->VSSetShader(
		this->fin_vs->getComPtr().Get(),
		nullptr,
		0
	);
	context->RSSetState(this->rasterizer_state->getComPtr().Get());
	context->PSSetShader(
		this->fin_ps->getComPtr().Get(),
		nullptr,
		0
	);
}
