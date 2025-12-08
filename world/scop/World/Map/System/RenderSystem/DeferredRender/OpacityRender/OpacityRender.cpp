#include "pch.h"
#include "OpacityRender.h"
#include "DeferredGraphics.h"
#include "DeferredBuffer.h"
#include "MapUtils.h"
#include "InputLayout.h"
#include "InputLayouts.h"
#include "VertexShader.h"
#include "RasterizerState.h"
#include "PixelShader.h"
#include "SamplerState.h"
#include "Block.h"
#include "Buffer.h"
#include "TestCam.h"
#include "Entity.h"

OpacityRender::OpacityRender(MapUtils* m_info)
	: m_info(m_info), geo_render(m_info),
	shadow_render(m_info), ssao_render(m_info->width, m_info->height),
	ssao_blur(m_info->width, m_info->height), 
	pbr(m_info->width, m_info->height), cave_shadow(m_info),
	cube_map(m_info->width, m_info->height)
{
	ComPtr<ID3D11Device> device = d_graphic->getDevice();
	
	this->d_buff = make_shared<DeferredBuffer>(1);
	this->d_buff->setRTVsAndSRVs(device,
		this->m_info->width, this->m_info->height);
	this->d_reflection_buff = make_shared<DeferredBuffer>(1);
	this->d_reflection_buff->setRTVsAndSRVs(
		device, this->m_info->width, this->m_info->height);

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
		L"shader/World/DeferredShader/ResultVS.hlsl",
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
		L"shader/World/DeferredShader/ResultPS.hlsl",
		"main",
		"ps_5_0"
	);
	this->sampler_state = make_shared<SamplerState>(device);
	this->view_port.TopLeftX = 0.0f;
	this->view_port.TopLeftY = 0.0f;
	this->view_port.Width = this->m_info->width;
	this->view_port.Height = this->m_info->height;
	this->view_port.MinDepth = 0.f;
	this->view_port.MaxDepth = 1.f;
}

void OpacityRender::setPipe()
{
	ComPtr<ID3D11DeviceContext> context;
	context = d_graphic->getContext();
	context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(this->input_layout->getComPtr().Get());
	uint32 stride = this->v_buff->getStride();
	uint32 offset = this->v_buff->getOffset();
	context->IASetVertexBuffers(0, 1,
		this->v_buff->getComPtr().GetAddressOf(),
		&stride, &offset);
	context->IASetIndexBuffer(
		this->i_buff->getComPtr().Get(),
		DXGI_FORMAT_R32_UINT,
		0
	);
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

void OpacityRender::ssaoBlur(int cnt, CamType type)
{
	float texel_dist = 3.0f;
	ComPtr<ID3D11DeviceContext> context;
	context = d_graphic->getContext();
	//// ssao blur width start
	d_graphic->renderBegin(
		this->ssao_blur.getWidthDBuffer().get());
	context->PSSetShaderResources(0, 1,
		this->geo_render.getSRV(RTVIndex::w_normal).GetAddressOf());
	context->PSSetShaderResources(1, 1,
		this->geo_render.getDepthSRV().GetAddressOf());
	context->PSSetShaderResources(2, 1,
		this->ssao_render.getSRV().GetAddressOf());
	this->ssao_blur.render(0, type, texel_dist);

	//// ssao blur height start
	d_graphic->renderBegin(
		this->ssao_blur.getHeightDBuffer().get());
	context->PSSetShaderResources(0, 1,
		this->geo_render.getSRV(RTVIndex::w_normal).GetAddressOf());
	context->PSSetShaderResources(1, 1,
		this->geo_render.getDepthSRV().GetAddressOf());
	context->PSSetShaderResources(2, 1,
		this->ssao_blur.getWidthSRV().GetAddressOf());
	this->ssao_blur.render(1, type, texel_dist);

	// 세세하게 blur하여 노이즈를 줄여줌
	for (int i = 1; i < cnt; i++) {
		// ssao blur width start
		d_graphic->renderBegin(
			this->ssao_blur.getWidthDBuffer().get());
		context->PSSetShaderResources(0, 1,
			this->geo_render.getSRV(RTVIndex::w_normal).GetAddressOf());
		context->PSSetShaderResources(1, 1,
			this->geo_render.getDepthSRV().GetAddressOf());
		context->PSSetShaderResources(2, 1,
			this->ssao_blur.getHeightSRV().GetAddressOf());
		this->ssao_blur.render(0, type, 1.0f);

		// ssao blur height start
		d_graphic->renderBegin(
			this->ssao_blur.getHeightDBuffer().get());
		context->PSSetShaderResources(0, 1,
			this->geo_render.getSRV(RTVIndex::w_normal).GetAddressOf());
		context->PSSetShaderResources(1, 1,
			this->geo_render.getDepthSRV().GetAddressOf());
		context->PSSetShaderResources(2, 1,
			this->ssao_blur.getWidthSRV().GetAddressOf());
		this->ssao_blur.render(1, type, 1.0f);
	}
}

void OpacityRender::setPBRResources()
{
	ComPtr<ID3D11DeviceContext> context = d_graphic->getContext();
	context->PSSetShaderResources(1, 1,
		this->geo_render.getSRV(RTVIndex::w_normal).GetAddressOf());
	context->PSSetShaderResources(2, 1,
		this->geo_render.getSRV(RTVIndex::w_position).GetAddressOf());
	context->PSSetShaderResources(3, 1,
		this->geo_render.getSRV(RTVIndex::rma).GetAddressOf());
}

void OpacityRender::render(
	RenderOption const& render_option,
	CamType type
)
{
	ComPtr<ID3D11DeviceContext> context;
	context = d_graphic->getContext();
	vec3 cam_pos = cam->getPos();

	// sky box
	this->cube_map.render(type, render_option.ccw_flag);
	this->m_info->directional_light_pos =
		this->cube_map.getDirectionalLightPos(); // 태양 위치
	this->m_info->light_dir = vec3(cosf(sun_radian), sinf(sun_radian), 0);

	// geo render
	this->geo_render.setParallaxFlag(render_option.parallax_mapping);
	this->geo_render.render(type, render_option.geo_opt);

	// pbr render
	this->pbr.setRTV();
	this->setPBRResources();
	vec3 lp = this->m_info->directional_light_pos;
	this->pbr.render(lp, cam_pos, 
		this->geo_render.getSRV(RTVIndex::color));

	if (render_option.shadow) {
		// shadow map render
		this->shadow_render.renderCSM(type);

		context->PSSetShaderResources(9, 1,
			this->geo_render.getSRV(RTVIndex::w_position).GetAddressOf());
		context->PSSetShaderResources(10, 1,
			this->geo_render.getSRV(RTVIndex::ssao_normal).GetAddressOf());
		context->PSSetShaderResources(11, 1,
			entity->getSRVPos().GetAddressOf());
		context->PSSetShaderResources(12, 1,
			entity->getSRVNormal().GetAddressOf());
		this->shadow_render.render(type);
	}

	if (render_option.cave_shadow)
		this->cave_shadow.render(type);
	else {
		float ca[4] = { 15.f, 15.f, 15.f, 1.f };
		context->ClearRenderTargetView(
			this->cave_shadow.getRTV().Get(), ca);
	}

	if (render_option.ssao) {
		// ssao start
		d_graphic->renderBegin(
			this->ssao_render.getDBuffer().get());
		context->PSSetShaderResources(0, 1,
			this->geo_render.getSRV(RTVIndex::ssao_normal).GetAddressOf()
		);
		context->PSSetShaderResources(1, 1,
			this->geo_render.getSRV(RTVIndex::w_position).GetAddressOf()
		);
		context->PSSetShaderResources(
			2,
			1,
			this->geo_render.getDepthSRV().GetAddressOf()
		);
		this->ssao_render.render(type);

		// ssao blur start
		this->ssaoBlur(render_option.ssao_blur_cnt, type);
	}
	else {
		float ca[4] = { 1.f, 1.f, 1.f, 1.f };
		context->ClearRenderTargetView(
			this->ssao_render.getDBuffer()->getRTV(0).Get(), ca);
	}
	d_graphic->renderBegin(this->d_buff.get());
	d_graphic->setViewPort(this->view_port);
	this->setPipe();
	context->PSSetShaderResources(0, 1,
		this->pbr.getAmbientLight().GetAddressOf());
	context->PSSetShaderResources(1, 1,
		this->pbr.getDirectLight().GetAddressOf());
	context->PSSetShaderResources(2, 1,
		this->shadow_render.getSRV().GetAddressOf());
	if (render_option.ssao) {
		context->PSSetShaderResources(3, 1,
			this->ssao_blur.getHeightSRV().GetAddressOf());
	}
	else {
		context->PSSetShaderResources(3, 1,
			this->ssao_render.getSRV().GetAddressOf());
	}
	context->PSSetShaderResources(4, 1,
		this->cube_map.getSRV().GetAddressOf());
	context->PSSetShaderResources(5, 1,
		this->cave_shadow.getSRV().GetAddressOf());
	context->PSSetShaderResources(6, 1, entity->getSRV().GetAddressOf());
	context->DrawIndexed(
		this->i_buff->getCount(),
		0, 0);
}

ComPtr<ID3D11ShaderResourceView> OpacityRender::getSRV()
{
	return this->d_buff->getSRV(0);
}

ComPtr<ID3D11RenderTargetView> OpacityRender::getRTV()
{
	return this->d_buff->getRTV(0);
}

ComPtr<ID3D11ShaderResourceView> OpacityRender::getGeoDepthSRV()
{
	return this->geo_render.getDepthSRV();
}

ComPtr<ID3D11ShaderResourceView> OpacityRender::getGeoSRV(
	RTVIndex rtv_idx
)
{
	return this->geo_render.getSRV(rtv_idx);
}

ComPtr<ID3D11ShaderResourceView> OpacityRender::getCubeSRV()
{
	return this->cube_map.getSRV();
}
