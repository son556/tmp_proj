#pragma once

#include "GeoRender.h"
#include "ShadowRender.h"
#include "SsaoRender.h"
#include "SsaoBlur.h"
#include "CaveShadow.h"
#include "PBR.h"
#include "Wallpaper.h"


class MapUtils;

struct RenderOption {
	bool parallax_mapping;
	bool ssao;
	bool shadow;
	bool cave_shadow;
	bool reflection_flag;
	int ssao_blur_cnt;
	bool ccw_flag = false;
	GeoRenderOption geo_opt;
};

class OpacityRender
{
public:
	OpacityRender(MapUtils* m_info);
	~OpacityRender() = default;
	void setPipe();
	void ssaoBlur(int cnt, CamType type);
	void setPBRResources();
	void render(
		RenderOption const& render_option,
		CamType type
	);
	ComPtr<ID3D11ShaderResourceView> getSRV();
	ComPtr<ID3D11RenderTargetView> getRTV();
	ComPtr<ID3D11ShaderResourceView> getGeoDepthSRV();
	ComPtr<ID3D11ShaderResourceView> getGeoSRV(RTVIndex rtv_idx);
	ComPtr<ID3D11ShaderResourceView> getCubeSRV();

private:
	OpacityRender() = delete;
	OpacityRender(OpacityRender const&) = delete;
	OpacityRender& operator=(OpacityRender const&) = delete;

private:
	MapUtils* m_info;
	GeoRender geo_render;
	ShadowRender shadow_render;
	SsaoRender ssao_render;
	SsaoBlur ssao_blur;
	PBR pbr;
	CaveShadow cave_shadow;
	Wallpaper cube_map;
	shared_ptr<Buffer<VertexDefer>> v_buff;
	shared_ptr<Buffer<uint32>> i_buff;
	shared_ptr<DeferredBuffer> d_buff;
	shared_ptr<DeferredBuffer> d_reflection_buff;
	D3D11_VIEWPORT view_port;

private:
	shared_ptr<InputLayout> input_layout;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<SamplerState> sampler_state;
};

