#include "pch.h"
#include "WaterInit.h"
#include "DeferredGraphics.h"
#include "DeferredBuffer.h"
#include "VertexShader.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "RasterizerState.h"
#include "PixelShader.h"
#include "MapUtils.h"
#include "Chunk.h"
#include "ConstantBuffer.h"
#include "SamplerState.h"
#include "TestCam.h"

WaterInit::WaterInit(MapUtils* m_info) : m_info(m_info)
{
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	this->d_buff = make_shared<DeferredBuffer>(2);
	this->d_buff->setRTVsAndSRVs(
		device,
		this->m_info->width,
		this->m_info->height
	);
	this->vertex_shader = make_shared<VertexShader>(
		device,
		L"shader/World/OIT/Water/WaterInit/WaterInitVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->input_layout = make_shared<InputLayout>(
		device,
		InputLayouts::layout_pct.data(),
		InputLayouts::layout_pct.size(),
		this->vertex_shader->getBlob()
	);
	this->pixel_shader = make_shared<PixelShader>(
		device,
		L"shader/World/OIT/Water/WaterInit/WaterInitPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE
	);
	this->sampler_state = make_shared<SamplerState>(device);

	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = false;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	
	desc.StencilEnable = true;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HRESULT hr = device->CreateDepthStencilState(&desc,
		this->ds_state.GetAddressOf());
	CHECK(hr);

	this->createDSV();
}

void WaterInit::setPipe()
{
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

void WaterInit::render(
	CamType type, 
	ComPtr<ID3D11ShaderResourceView> const& depth_srv
)
{
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	d_graphic->renderBegin(this->d_buff.get(), this->dsv);
	this->setPipe();
	context->OMSetDepthStencilState(this->ds_state.Get(), 1);
	context->VSSetConstantBuffers(0, 1,
		cam->getConstantBuffer(type)->getComPtr().GetAddressOf());
	context->PSSetShaderResources(0, 1, depth_srv.GetAddressOf());
	
	//TODO : 나중에 지울 부분
	/*for (int i = 0; i < this->m_info->size_h; i++) {
		for (int j = 0; j < this->m_info->size_w; j++) {
			if (this->m_info->chunks[i][j]->w_chunk.render_flag == false)
				continue;
			this->m_info->chunks[i][j]->w_chunk.setBuffer(context);
		}
	}*/

	for (auto chunkIndex : this->m_info->GetRenderableChunkListToRead())
	{
		if (this->m_info->chunks[chunkIndex.y][chunkIndex.x]->w_chunk.render_flag == false)
			continue;
		this->m_info->chunks[chunkIndex.y][chunkIndex.x]->w_chunk.setBuffer(context);
	}
	context->OMSetDepthStencilState(nullptr, 0);
}

ComPtr<ID3D11ShaderResourceView> WaterInit::getSRV(WaterRTVType type)
{
	if (type == WaterRTVType::POSITION)
		return this->d_buff->getSRV(0);
	return this->d_buff->getSRV(1);
}

ComPtr<ID3D11DepthStencilView> WaterInit::getDSV()
{
	return this->dsv;
}

void WaterInit::createDSV()
{
	ComPtr<ID3D11Texture2D> depth_texture;
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = this->m_info->width;
	desc.Height = this->m_info->height;
	desc.ArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	HRESULT hr = d_graphic->getDevice()->CreateTexture2D(
		&desc, nullptr, depth_texture.GetAddressOf());
	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	ZeroMemory(&dsv_desc, sizeof(dsv_desc));
	dsv_desc.Format = desc.Format;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Texture2D.MipSlice = 0;
	hr = d_graphic->getDevice()->CreateDepthStencilView(
		depth_texture.Get(), &dsv_desc, this->dsv.GetAddressOf());
	CHECK(hr);
}
