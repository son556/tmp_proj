#pragma once

#include "CascadeShadow.h"
#include "WorldUtils.h"

class MapUtils;
class DeferredBuffer;
class RasterizerState;
class VertexShader;
class PixelShader;
class InputLayout;
class TextureArray;
class StructuredBuffer;
template<typename T> class Buffer;


struct FrusumSplit {
	vec4 light_dir; // world space
	vec4 vz_arr[8]; // view space
	Mat view;
};

class ShadowRender
{
public:
	ShadowRender(MapUtils* minfo);
	~ShadowRender() = default;
	void renderCSM(CamType type);
	void render(CamType type);
	ComPtr<ID3D11ShaderResourceView> getSRV();

private:
	ShadowRender() = delete;
	ShadowRender(ShadowRender const&) = delete;
	ShadowRender& operator=(ShadowRender) = delete;

private:
	void setPipe();
	void setCSMPipe();
	void devideFrustum();

private:
	MapUtils* m_info;
	shared_ptr<DeferredBuffer> d_buffer;
	shared_ptr<RasterizerState> rasterizer_state;
	shared_ptr<VertexShader> vertex_shader;
	shared_ptr<PixelShader> pixel_shader;
	shared_ptr<InputLayout> input_layout;
	shared_ptr<ConstantBuffer> ps_cbuffer;

private:
	int split_cnt = 5;
	FrusumSplit frustum_split;
	shared_ptr<Buffer<VertexDefer>> vbuffer;
	shared_ptr<Buffer<uint32>> ibuffer;

private:
	vector<MVP> mvps;
	vector<shared_ptr<CascadeShadow>> csms;
	shared_ptr<StructuredBuffer> structured_buffer;
	shared_ptr<TextureArray> tex2d_arr;
	shared_ptr<PixelShader> s_pixel_shader;
	shared_ptr<VertexShader> s_vertex_shader;
	shared_ptr<InputLayout> s_input_layout;
	shared_ptr<RasterizerState> s_rasterizer_state;
};

