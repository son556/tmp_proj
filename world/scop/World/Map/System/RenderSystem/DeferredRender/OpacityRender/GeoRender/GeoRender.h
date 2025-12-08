#pragma once

#include "ParallaxMapping.h"

class MapUtils;
class HullShader;
class DomainShader;
class DepthMap;

struct GeoRenderOption {
	ID3D11DepthStencilState* ds_state = nullptr;
	ComPtr<ID3D11DepthStencilView> ptr_dsv = nullptr;
	int* stencil_ref_num = nullptr;
	D3D11_VIEWPORT* view_port = nullptr;
	bool clear_rtv = true;
	bool clear_dsv = true;
	bool ccw_flag = false;
	bool cut_flag = false;
	int cut_height;
};

class GeoRender
{
public:
	GeoRender(MapUtils* minfo);
	~GeoRender() = default;
	void render(
		CamType type,
		GeoRenderOption opt = 
		{nullptr, nullptr, nullptr, nullptr, true, true}
	);
	void setParallaxFlag(bool flag);
	ComPtr<ID3D11ShaderResourceView> getSRV(RTVIndex idx);
	ComPtr<ID3D11ShaderResourceView> getNPSRV(RTVIndex idx);
	ComPtr<ID3D11ShaderResourceView> getDepthSRV();
	ComPtr<ID3D11DepthStencilView> getDSV();
	ComPtr<ID3D11RenderTargetView> getRTV(RTVIndex idx);

private:
	GeoRender() = delete;
	GeoRender(GeoRender const&) = delete;
	GeoRender& operator=(GeoRender const&) = delete;

private:
	void setPipe(bool ccw_flag);
	void setConstantBuffer(CamType type);
	void parallaxRender(vec3 const& cam_pos);

private:
	MapUtils* m_info;
	shared_ptr<ParallaxMapping> parallax_mapping;
	shared_ptr<DeferredBuffer> d_buffer;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<RasterizerState> ccw_rasterizer_state;
	shared_ptr<SamplerState> linear_state;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<InputLayout> input_layout;
	shared_ptr<HullShader> hull_shader;
	shared_ptr<DomainShader> domain_shader;
	shared_ptr<ConstantBuffer> cut_constant_buff;

private:
	bool parallax_flag = true;
	shared_ptr<ConstantBuffer> mvp_cbuffer;
	shared_ptr<ConstantBuffer> cam_pos_cbuffer;
	shared_ptr<ConstantBuffer> eye_pos_cbuffer;
	shared_ptr<DepthMap> depth_map;
};

