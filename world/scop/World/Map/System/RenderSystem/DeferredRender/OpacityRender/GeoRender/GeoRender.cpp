#include "pch.h"
#include "GeoRender.h"
#include "DeferredGraphics.h"
#include "DeferredBuffer.h"
#include "MapUtils.h"
#include "RasterizerState.h"
#include "SamplerState.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "ConstantBuffer.h"
#include "Chunk.h"
#include "DepthMap.h"
#include "TestCam.h"
#include "BlockTextureArray.h"

GeoRender::GeoRender(MapUtils* minfo)
{
	this->m_info = minfo;
	this->d_buffer = make_shared<DeferredBuffer>(8);
	this->d_buffer->setRTVsAndSRVs(
		d_graphic->getDevice(),
		this->m_info->width,
		this->m_info->height
	);
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	this->rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		//D3D11_FILL_WIREFRAME,
		D3D11_CULL_BACK
	);
	this->ccw_rasterizer_state = make_shared<RasterizerState>(
		device,
		D3D11_FILL_SOLID,
		D3D11_CULL_BACK,
		true
	);
	// texture manager: texture array setting

	this->linear_state = make_shared<SamplerState>(device);
	this->vertex_shader = make_shared<VertexShader>(
		device,
		L"shader/World/DeferredShader/Geometry/GeometryVS.hlsl",
		"main",
		"vs_5_0"
	);
	this->pixel_shader = make_shared<PixelShader>(
		device,
		L"shader/World/DeferredShader/Geometry/GeometryPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->input_layout = make_shared<InputLayout>(
		device,
		InputLayouts::layout_ipntt.data(),
		InputLayouts::layout_ipntt.size(),
		this->vertex_shader->getBlob()
	);
	MVP mvp;
	this->mvp_cbuffer = make_shared<ConstantBuffer>(
		device,
		context,
		mvp
	);
	CamPos cam;
	this->cam_pos_cbuffer = make_shared<ConstantBuffer>(
		device,
		context,
		cam
	);
	vec4 eye;
	this->eye_pos_cbuffer = make_shared<ConstantBuffer>(
		device,
		context,
		eye
	);
	if (this->parallax_flag) {
		this->parallax_mapping = make_shared<ParallaxMapping>(
			this->m_info->width,
			this->m_info->height
		);
	}
	this->depth_map = make_shared<DepthMap>(
		d_graphic->getDevice(),
		this->m_info->width, this->m_info->height
	);
	vec4 v;
	this->cut_constant_buff = make_shared<ConstantBuffer>(
		device,
		d_graphic->getContext(),
		v
	);
}

void GeoRender::render(
	CamType type,
	GeoRenderOption opt
)
{
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	float y = cam->getPos().y;
	vec4 c_opt;
	if (opt.cut_flag == true) {
		if (y > WATER_HEIGHT)
			c_opt.x = 1;
		else
			c_opt.x = 2;
		c_opt.y = opt.cut_height;
		this->cut_constant_buff->update(c_opt);
	}
	context->PSSetConstantBuffers(0, 1,
		this->cut_constant_buff->getComPtr().GetAddressOf());
	if (opt.ptr_dsv == nullptr)
		d_graphic->renderBegin(this->d_buffer.get(),
			this->depth_map->getDepthStencilView());
	else {
		d_graphic->renderBegin(this->d_buffer.get(), opt.ptr_dsv,
			opt.clear_rtv, opt.clear_dsv);
	}
	if (opt.ds_state)
		context->OMSetDepthStencilState(opt.ds_state, *(opt.stencil_ref_num));
	if (opt.view_port)
		context->RSSetViewports(1, opt.view_port);
	this->setPipe(opt.ccw_flag);
	this->setConstantBuffer(type);

	for (auto chunkIndex : this->m_info->GetRenderableChunkListToRead())
	{
		this->m_info->chunks[chunkIndex.y][chunkIndex.x]->setGeoRender(
			d_graphic->getContext()
		);
	}
	if (opt.ds_state)
		context->OMSetDepthStencilState(nullptr, 0);
	context->HSSetShader(nullptr, nullptr, 0);
	context->DSSetShader(nullptr, nullptr, 0);
	if (this->parallax_flag)
		this->parallaxRender(cam->getPos());
}

void GeoRender::setPipe(bool ccw_flag)
{
	ComPtr<ID3D11DeviceContext> context = 
		d_graphic->getContext();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(this->input_layout->getComPtr().Get());
	context->VSSetShader(
		this->vertex_shader->getComPtr().Get(),
		nullptr,
		0
	);
	if (ccw_flag == false)
	{
		context->RSSetState(this->rasterizer_state->getComPtr().Get());
	}
	else
	{
		context->RSSetState(this->ccw_rasterizer_state->getComPtr().Get());
	}

	context->PSSetShader(
		this->pixel_shader->getComPtr().Get(),
		nullptr,
		0
	);
	context->PSSetSamplers(
		0,
		1,
		this->linear_state->getComPtr().GetAddressOf()
	);
	context->PSSetShaderResources(
		0,
		1,
		BlockTextureArray::getBlocksColor()->getComPtr().GetAddressOf()
	);
	context->PSSetShaderResources(
		1,
		1,
		BlockTextureArray::getBlocksS()->getComPtr().GetAddressOf()
	);
	context->PSSetShaderResources(
		2,
		1,
		BlockTextureArray::getBlocksNormal()->getComPtr().GetAddressOf()
	);

	//context->HSSetShader(
	//	this->hull_shader->getComPtr().Get(),
	//	nullptr,
	//	0
	//);
	//context->DSSetShader(
	//	this->domain_shader->getComPtr().Get(),
	//	nullptr,
	//	0
	//);
	//// 실제 vertex위치 옮기는 경우
	//context->DSSetShaderResources(0, 1,
	//	BlockTextureArray::getBlocksNormal()->getComPtr().GetAddressOf());
}

void GeoRender::setConstantBuffer(CamType type)
{
	ComPtr<ID3D11DeviceContext> context = 
		d_graphic->getContext();

	// set hs constant
	vec3 cam_pos = cam->getPos();
	vec4 eye;
	eye.x = cam_pos.x;
	eye.y = cam_pos.y;
	eye.z = cam_pos.z;
	eye.w = 1;
	/*this->eye_pos_cbuffer->update(eye);
	context->HSSetConstantBuffers(0, 1,
		this->eye_pos_cbuffer->getComPtr().GetAddressOf());*/
	
	// set ds constant
	/*context->DSSetConstantBuffers(0, 1,
		cam->getConstantBuffer(type)->getComPtr().GetAddressOf());
	context->DSSetConstantBuffers(1, 1,
		this->eye_pos_cbuffer->getComPtr().GetAddressOf());*/

	context->VSSetConstantBuffers(0, 1, cam->getConstantBuffer(type)->getComPtr().GetAddressOf());
}

void GeoRender::parallaxRender(vec3 const& cam_pos)
{
	this->parallax_mapping->setRTV();
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	context->PSSetShaderResources(0, 1,
		BlockTextureArray::getBlocksColor()->getComPtr().GetAddressOf());
	context->PSSetShaderResources(1, 1,
		BlockTextureArray::getBlocksNormal()->getComPtr().GetAddressOf());
	context->PSSetShaderResources(2, 1,
		BlockTextureArray::getBlocksS()->getComPtr().GetAddressOf());
	context->PSSetShaderResources(3, 1,
		this->d_buffer->getSRV(5).GetAddressOf());
	context->PSSetShaderResources(4, 1,
		this->d_buffer->getSRV(1).GetAddressOf());
	context->PSSetShaderResources(5, 1,
		this->d_buffer->getSRV(6).GetAddressOf());
	context->PSSetShaderResources(6, 1,
		this->d_buffer->getSRV(7).GetAddressOf());
	this->parallax_mapping->render();
}

void GeoRender::setParallaxFlag(bool flag)
{
	this->parallax_flag = flag;
}

ComPtr<ID3D11ShaderResourceView> GeoRender::getSRV(RTVIndex idx)
{
	int index = static_cast<int>(idx);
	if (this->parallax_flag)
		return this->parallax_mapping->getSRV(idx);
	return this->d_buffer->getSRV(index);
}

ComPtr<ID3D11ShaderResourceView> GeoRender::getNPSRV(RTVIndex idx)
{
	int index = static_cast<int>(idx);
	return this->d_buffer->getSRV(index);
}

ComPtr<ID3D11ShaderResourceView> GeoRender::getDepthSRV()
{
	return this->depth_map->getShaderResourceView();
}

ComPtr<ID3D11DepthStencilView> GeoRender::getDSV()
{
	return this->depth_map->getDepthStencilView();
}

ComPtr<ID3D11RenderTargetView> GeoRender::getRTV(RTVIndex idx)
{
	int index = static_cast<int>(idx);
	if (this->parallax_flag)
		return this->parallax_mapping->getRTV(idx);
	return this->d_buffer->getRTV(index);
}

